#include "AbilitySystem/Abilities/Player/GC_Tertiary.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Effects/GC_TertiaryBuffEffect.h"
#include "Character/MyBaseCharacter.h"
#include "GameFramework/Character.h"
#include "GameplayEffect.h"
#include "GameplayTags/GCTags.h"

UGC_Tertiary::UGC_Tertiary()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	bRetriggerInstancedAbility = false;

	FGameplayTagContainer AbilityAssetTags;
	AbilityAssetTags.AddTag(GCTags::GCAbilities::player::Tertiary);
	SetAssetTags(AbilityAssetTags);
	TertiaryBuffEffectClass = UGC_TertiaryBuffEffect::StaticClass();
}

bool UGC_Tertiary::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// Reject activation if generic GAS requirements already fail.
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AMyBaseCharacter* BaseCharacter = Cast<AMyBaseCharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
	if (!IsValid(BaseCharacter) || !BaseCharacter->IsAlive())
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!IsValid(ASC))
	{
		return false;
	}

	if (!IsValid(TertiaryBuffEffectClass))
	{
		return false;
	}

	return !ASC->HasMatchingGameplayTag(GCTags::State::Buff::SuperArmor);
}

void UGC_Tertiary::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Commit cost/cooldown first, then continue with the buff application.
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!IsActive())
	{
		return;
	}

	AMyBaseCharacter* BaseCharacter = Cast<AMyBaseCharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(BaseCharacter) || !BaseCharacter->IsAlive() || !IsValid(ASC) || !IsValid(TertiaryBuffEffectClass))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Prevent duplicate activations from stacking while the temporary state tag is already present.
	if (ASC->HasMatchingGameplayTag(GCTags::State::Buff::SuperArmor))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	EffectContext.AddInstigator(BaseCharacter, BaseCharacter);

	FGameplayEffectSpecHandle EffectSpecHandle = ASC->MakeOutgoingSpec(TertiaryBuffEffectClass, GetAbilityLevel(), EffectContext);
	if (!EffectSpecHandle.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const float BuffDuration = EffectSpecHandle.Data->GetDuration();

	ActiveTertiaryBuffHandle = ASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	if (!ActiveTertiaryBuffHandle.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AddTertiaryBuffCue();
	BP_OnTertiaryStarted();

	ActiveDurationTask = UAbilityTask_WaitDelay::WaitDelay(this, FMath::Max(BuffDuration, KINDA_SMALL_NUMBER));
	if (!IsValid(ActiveDurationTask))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveDurationTask->OnFinish.AddDynamic(this, &UGC_Tertiary::HandleTertiaryDurationFinished);
	ActiveDurationTask->ReadyForActivation();
}

void UGC_Tertiary::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// Stop any outstanding wait task before removing the replicated buff state.
	if (IsValid(ActiveDurationTask))
	{
		ActiveDurationTask->EndTask();
		ActiveDurationTask = nullptr;
	}

	RemoveTertiaryBuffCue();
	RemoveTertiaryBuffEffect();
	BP_OnTertiaryFinished(bWasCancelled);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGC_Tertiary::HandleTertiaryDurationFinished()
{
	// End the ability normally when the temporary buff duration elapses.
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGC_Tertiary::AddTertiaryBuffCue() const
{
	// Add a persistent cue so the buff particles stay attached for the whole super-armor window.
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(ASC) || !IsValid(AvatarActor))
	{
		return;
	}

	if (!AvatarActor->HasAuthority())
	{
		return;
	}

	FGameplayCueParameters CueParameters;
	CueParameters.Location = AvatarActor->GetActorLocation();
	CueParameters.RawMagnitude = TertiaryBuffCueMagnitude;
	CueParameters.Instigator = AvatarActor;
	CueParameters.EffectCauser = AvatarActor;

	// Always target the character mesh explicitly so the first cue activation does not rely on fallback attachment resolution.
	if (const ACharacter* Character = Cast<ACharacter>(AvatarActor))
	{
		CueParameters.TargetAttachComponent = Character->GetMesh();
	}

	ASC->AddGameplayCue(GCTags::GameplayCue::Tertiary_Buff, CueParameters);
}

void UGC_Tertiary::RemoveTertiaryBuffCue() const
{
	// Remove the persistent cue when the buff ends or gets cancelled.
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(ASC) || !IsValid(AvatarActor))
	{
		return;
	}

	if (!AvatarActor->HasAuthority())
	{
		return;
	}

	ASC->RemoveGameplayCue(GCTags::GameplayCue::Tertiary_Buff);
}

void UGC_Tertiary::RemoveTertiaryBuffEffect()
{
	// Clear the active gameplay effect so death/cancel never leaves stale armor or tags behind.
	if (!ActiveTertiaryBuffHandle.IsValid())
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (const AActor* AvatarActor = GetAvatarActorFromActorInfo(); IsValid(AvatarActor) && AvatarActor->HasAuthority())
		{
			ASC->RemoveActiveGameplayEffect(ActiveTertiaryBuffHandle);
		}
	}

	ActiveTertiaryBuffHandle.Invalidate();
}
