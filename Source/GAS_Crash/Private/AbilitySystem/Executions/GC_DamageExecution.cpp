#include "AbilitySystem/Executions/GC_DamageExecution.h"
#include "AbilitySystem/GC_AttributeSet.h"
#include "AbilitySystem/GC_GameplayEffectContext.h"
#include "GameplayTags/GCTags.h"

// FGC_DamageStatics:
// This local struct caches FGameplayEffectAttributeCaptureDefinitions at startup.
// Capture definitions are expensive to construct (involves UProperty reflection lookups),
// so we build them ONCE as a static singleton and reuse on every execution.
// The pattern mirrors Lyra's FDamageStatics exactly.
//
// DECLARE_ATTRIBUTE_CAPTUREDEF creates two members:
//   - FProperty*                                ArmorProperty   (the reflected UProperty pointer)
//   - FGameplayEffectAttributeCaptureDefinition ArmorDef        (capture config: which attribute, source/target, snapshot?)
// DEFINE_ATTRIBUTE_CAPTUREDEF fills them in via the AttributeSet's static getter.
struct FGC_DamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);

	FGC_DamageStatics()
	{
		// Source vs Target capture:
		// The third parameter (EGameplayEffectAttributeCaptureSource::Target) is critical:
		// it tells the ExecCalc to read Armor from the TARGET of the GE, not the Source.
		// The fourth parameter (false) means "don't snapshot at GE creation time" — we want
		// the live value at execution time so armor buffs/debuffs applied mid-flight are respected.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGC_AttributeSet, Armor, Target, false);
	}
};

static const FGC_DamageStatics& DamageStatics()
{
	static FGC_DamageStatics Statics;
	return Statics;
}

UGC_DamageExecution::UGC_DamageExecution()
{
	// Register all attribute captures so the GAS Aggregator knows to prepare them
	// before Execute_Implementation is called. Without this line, AttemptCalculateCapturedAttributeMagnitude
	// would fail silently and return 0.
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
}

void UGC_DamageExecution::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// ExtractEffectContext safely downcasts the base context to our custom type.
	// We gain access to bIsCriticalHit and DamageTypeTag that the Ability set before apply.
	const FGC_GameplayEffectContext* GCContext =
		FGC_GameplayEffectContext::ExtractEffectContext(Spec.GetContext());

	// --- Step 1: Read base damage magnitude ---
	// All damage sources now use the unified SetByCaller::Damage channel.
	float BaseDamage = Spec.GetSetByCallerMagnitude(GCTags::SetByCaller::Damage, false, 0.f);

	if (FMath::IsNearlyZero(BaseDamage))
	{
		// Nothing to output — GE was applied without a damage SetByCaller.
		return;
	}

	// --- Step 2: Apply modifiers from Context and captured Attributes ---

	// Critical hit: 1.5× multiplier. In the future you can capture the crit multiplier
	// from an Attribute (e.g. CritDamageBonus) so equipment/passives can modify it.
	const bool bIsCrit = GCContext && GCContext->IsCriticalHit();
	const float CritMultiplier = bIsCrit ? 1.5f : 1.0f;

	float DamageAfterCrit = BaseDamage * CritMultiplier;

	// Build tag-aware evaluate parameters so captured attributes respect modifier tag requirements.
	// Example: a "Fortified" GE may add Armor only when Target has State.Buff.Fortified.
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// Attribute Capture — the core value of ExecCalc
	// This is why ExecCalc exists instead of calculating damage in the Ability:
	// we can read the TARGET's Armor attribute at execution time, which the Ability
	// has no access to (Abilities only know their own ASC, not the target's attributes).
	//
	// Damage reduction formula: Reduction = Armor / (Armor + K)
	// This gives diminishing returns — same formula used in League of Legends, WoW, etc.
	//   0   Armor → 0% reduction
	//   50  Armor → 33% reduction
	//   100 Armor → 50% reduction
	//   200 Armor → 67% reduction
	// K controls the "softness" of the curve. Higher K = armor is less effective.
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		DamageStatics().ArmorDef,
		EvaluationParameters,
		TargetArmor);
	TargetArmor = FMath::Max(TargetArmor, 0.f);

	constexpr float ArmorScalingConstant = 100.f;
	const float ArmorReduction = TargetArmor / (TargetArmor + ArmorScalingConstant);
	DamageAfterCrit *= (1.f - ArmorReduction);

	const float FinalDamage = FMath::Max(DamageAfterCrit, 0.f);

	// --- Step 3: Output to IncomingDamage Meta Attribute ---
	
	// We output to IncomingDamage (not Health directly) so PostGameplayEffectExecute
	// can apply additional logic: shield absorption, death check, clamping, and
	// broadcasting the damage feedback delegate — all in one place.
	if (FinalDamage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				UGC_AttributeSet::GetIncomingDamageAttribute(),
				EGameplayModOp::Additive,
				FinalDamage));
	}
#endif // WITH_SERVER_CODE
}
