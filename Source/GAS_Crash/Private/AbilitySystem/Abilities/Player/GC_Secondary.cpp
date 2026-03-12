#include "AbilitySystem/Abilities/Player/GC_Secondary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/MyBaseCharacter.h"
#include "Curves/CurveFloat.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameplayEffect.h"
#include "Engine/OverlapResult.h"
#include "GameplayTags/GCTags.h"
#include "Character/GC_EnemyCharacter.h"

void UGC_Secondary::ExecuteSecondaryImpact()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor) || !AvatarActor->HasAuthority()) return;

	const AMyBaseCharacter* BaseCharacter = Cast<AMyBaseCharacter>(AvatarActor);
	if (!IsValid(BaseCharacter) || !BaseCharacter->IsAlive()) return;

	if (!DamageEffect) return;

	const FVector ImpactCenter = ResolveImpactCenter();
	const TArray<AActor*> Targets = QueryAOETargets(ImpactCenter);

	ApplyAOEToTargets(Targets, ImpactCenter);
	ExecuteExplosionCue(ImpactCenter);

	if (bDrawDebugs)
	{
		DrawDebugAOE(ImpactCenter, Targets);
	}
}

FVector UGC_Secondary::ResolveImpactCenter() const
{
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	return IsValid(AvatarActor) ? AvatarActor->GetActorLocation() : FVector::ZeroVector;
}

TArray<AActor*> UGC_Secondary::QueryAOETargets(const FVector& Center) const
{
	TArray<AActor*> Targets;

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor) || !GetWorld()) return Targets;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GC_SecondaryAOE), false, AvatarActor);
	QueryParams.AddIgnoredActor(AvatarActor);

	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECR_Ignore);
	ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Block);

	TArray<FOverlapResult> OverlapResults;
	const bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		Center,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(AOERadius),
		QueryParams,
		ResponseParams);

	if (!bHasOverlap) return Targets;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* CandidateActor = Result.GetActor();
		if (!IsEligibleAOETarget(CandidateActor)) continue;

		Targets.AddUnique(CandidateActor);
	}

	return Targets;
}

bool UGC_Secondary::IsEligibleAOETarget(AActor* CandidateActor) const
{
	if (!IsValid(CandidateActor)) return false;

	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor) || CandidateActor == AvatarActor) return false;

	const AMyBaseCharacter* BaseCharacter = Cast<AMyBaseCharacter>(CandidateActor);
	if (!IsValid(BaseCharacter) || !BaseCharacter->IsAlive()) return false;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CandidateActor);
	if (!IsValid(TargetASC)) return false;

	return TargetASC->HasMatchingGameplayTag(GCTags::GCIdentity::Enemy);
}

void UGC_Secondary::ApplyAOEToTargets(const TArray<AActor*>& Targets, const FVector& Center)
{
	for (AActor* TargetActor : Targets)
	{
		if (!IsValid(TargetActor)) continue;

		const float Distance = FVector::Dist(TargetActor->GetActorLocation(), Center);
		const float Alpha = FMath::Clamp(Distance / AOERadius, 0.f, 1.f);

		const float DamageScale = DamageFalloffCurve ? DamageFalloffCurve->GetFloatValue(Alpha) : 1.f;
		const float KnockbackScale = KnockbackFalloffCurve ? KnockbackFalloffCurve->GetFloatValue(Alpha) : 1.f;

		ApplyDamageToTarget(TargetActor, DamageScale);
		ApplyKnockbackToTarget(TargetActor, Center, KnockbackScale);
	}
}

void UGC_Secondary::ApplyDamageToTarget(AActor* TargetActor, float DamageScale) const
{
	if (!IsValid(TargetActor) || !DamageEffect) return;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(SourceASC)) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!IsValid(TargetASC)) return;

	AActor* SourceActor = GetAvatarActorFromActorInfo();
	if (!IsValid(SourceActor)) return;

	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddInstigator(SourceActor, SourceActor);
	ContextHandle.AddSourceObject(SourceActor);
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, GetAbilityLevel(), ContextHandle);
	if (!SpecHandle.IsValid()) return;

	const float FinalDamage = BaseDamage * DamageScale;

	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
		SpecHandle,
		GCTags::SetByCaller::SecondaryAOEAbility,
		FinalDamage);

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

void UGC_Secondary::ApplyKnockbackToTarget(AActor* TargetActor, const FVector& Center, float KnockbackScale) const
{
	ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
	if (!IsValid(TargetCharacter)) return;

	//Calculate horizontal push direction from explosion center to target, ignoring vertical difference.
	FVector PushDirection = TargetActor->GetActorLocation() - Center;
	PushDirection.Z = 0.f;

	// Close-range overlaps can collapse the radial vector toward zero.
	// Fall back to the caster forward so the knockback still feels deterministic.
	if (!PushDirection.Normalize())
	{
		if (const AActor* AvatarActor = GetAvatarActorFromActorInfo())
		{
			PushDirection = AvatarActor->GetActorForwardVector();
			PushDirection.Z = 0.f;
			PushDirection.Normalize();
		}
		else
		{
			PushDirection = FVector::ForwardVector;
		}
	}

	FVector LaunchVelocity = PushDirection * (BaseHorizontalKnockback * KnockbackScale);
	LaunchVelocity.Z = BaseVerticalKnockback * KnockbackScale;

	if(AGC_EnemyCharacter* EnemyCharacter = Cast<AGC_EnemyCharacter>(TargetCharacter))
	{
		EnemyCharacter->EnterKnockbackState(LaunchVelocity, KnockbackRecoveryDelay);
	}
}

void UGC_Secondary::ExecuteExplosionCue(const FVector& Center) const
{
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(SourceASC)) return;

	FGameplayCueParameters CueParameters;
	CueParameters.Location = Center;
	// In this project RawMagnitude is treated as effect strength, not spatial radius.
	// Use absolute base damage so cue-side scaling stays semantically aligned with damage feedback cues.
	CueParameters.RawMagnitude = FMath::Abs(BaseDamage);
	CueParameters.Instigator = GetAvatarActorFromActorInfo();
	CueParameters.EffectCauser = GetAvatarActorFromActorInfo();

	// Source explosion cue: earthquake particle, world camera shake, sound.
	SourceASC->ExecuteGameplayCue(GCTags::GameplayCue::Secondary_Explosion, CueParameters);
}

void UGC_Secondary::DrawDebugAOE(const FVector& Center, const TArray<AActor*>& Targets) const
{
	if (!GetWorld()) return;

	DrawDebugSphere(GetWorld(), Center, AOERadius, 32, FColor::Yellow, false, 2.f);

	for (AActor* TargetActor : Targets)
	{
		if (!IsValid(TargetActor)) continue;
		DrawDebugSphere(GetWorld(), TargetActor->GetActorLocation() + FVector(0.f, 0.f, 100.f), 16.f, 12, FColor::Red, false, 2.f);
	}
}

