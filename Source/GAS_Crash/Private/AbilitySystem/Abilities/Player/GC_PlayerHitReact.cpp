#include "AbilitySystem/Abilities/Player/GC_PlayerHitReact.h"
#include "AbilitySystemComponent.h"
#include "Character/MyBaseCharacter.h"
#include "GameplayTags/GCTags.h"
#include "Utils/GC_BluePrintLibrary.h"

UGC_PlayerHitReact::UGC_PlayerHitReact()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	bRetriggerInstancedAbility = false;
	
	// Check if this object is the class default object (CDO) for the ability.
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Register the gameplay event trigger once on the CDO to avoid blueprint-side misconfiguration.
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = GCTags::GCEvents::player::HitReact;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

bool UGC_PlayerHitReact::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                            const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;
	
	const AMyBaseCharacter* BaseCharacter = Cast<AMyBaseCharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
	if (!IsValid(BaseCharacter)) return false;

	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!IsValid(ASC)) return false;

	// Super armor explicitly blocks hit react activation even if an event slips through.
	if (ASC->HasMatchingGameplayTag(GCTags::State::Buff::SuperArmor)) return false;
	
	return BaseCharacter->IsAlive();
}

void UGC_PlayerHitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AMyBaseCharacter* BaseCharacter = Cast<AMyBaseCharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	
	if (!IsValid(BaseCharacter) || !IsValid(ASC))
	{
		FinishHitReactAbility(true);
		return;
	} 
	
	// Skip hit react if the character is already dead.
	if (!BaseCharacter->IsAlive() || ASC->HasMatchingGameplayTag(GCTags::GCEvents::player::Dead))
	{
		FinishHitReactAbility(true);
		return;
	}

	// Execute a dedicated cue only after the ability has actually entered hit react.
	ExecuteHitReactCue(TriggerEventData);
	
	const AActor* DamageInstigator = TriggerEventData ? TriggerEventData->Instigator.Get() : nullptr;
	
	TargetForward = BaseCharacter->GetActorForwardVector().GetSafeNormal();
	ToInstigator = IsValid(DamageInstigator) ? (DamageInstigator->GetActorLocation() - BaseCharacter->GetActorLocation()).GetSafeNormal() : TargetForward;
	
	const EHitDirection HitDirection = UGC_BluePrintLibrary::GetHitDirection(TargetForward, ToInstigator);
	HitReactSectionName = UGC_BluePrintLibrary::GetHitReactionName(HitDirection);
	
	// Forward the resolved montage section to the blueprint presentation layer.
	BP_StartHitReact(HitReactSectionName, TargetForward, ToInstigator);
}

void UGC_PlayerHitReact::FinishHitReactAbility(bool bWasCancelled)
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
	}
}

void UGC_PlayerHitReact::ExecuteHitReactCue(const FGameplayEventData* TriggerEventData) const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	
	if (!IsValid(ASC) || !IsValid(AvatarActor)) return;
	if (!AvatarActor->HasAuthority()) return;

	FGameplayCueParameters CueParameters;
	CueParameters.Location = AvatarActor->GetActorLocation();
	CueParameters.RawMagnitude = HitReactCueMagnitude;
	CueParameters.EffectContext = TriggerEventData ? TriggerEventData->ContextHandle : FGameplayEffectContextHandle();

	if (TriggerEventData && IsValid(TriggerEventData->Instigator))
	{
		// FGameplayEventData stores a const instigator pointer, while cue parameters expect an actor weak pointer.
		CueParameters.Instigator = const_cast<AActor*>(TriggerEventData->Instigator.Get());
	}

	if (TriggerEventData)
	{
		if (AActor* EffectCauser = TriggerEventData->ContextHandle.GetEffectCauser())
		{
			CueParameters.EffectCauser = EffectCauser;
		}

		/* Prefer the hit result stored in the gameplay effect context so the cue
		 * can spawn at the actual impact point instead of the actor origin. */
		if (const FHitResult* HitResult = TriggerEventData->ContextHandle.GetHitResult())
		{
			CueParameters.Location = HitResult->ImpactPoint.IsNearlyZero() ? HitResult->Location : HitResult->ImpactPoint;
			CueParameters.Normal = HitResult->ImpactNormal.IsNearlyZero() ? HitResult->Normal : HitResult->ImpactNormal;
		}
		else if (IsValid(TriggerEventData->Instigator))
		{
			CueParameters.Normal = (AvatarActor->GetActorLocation() - TriggerEventData->Instigator->GetActorLocation()).GetSafeNormal();
		}
	}

	ASC->ExecuteGameplayCue(GCTags::GameplayCue::Player_HitReact, CueParameters);
}
