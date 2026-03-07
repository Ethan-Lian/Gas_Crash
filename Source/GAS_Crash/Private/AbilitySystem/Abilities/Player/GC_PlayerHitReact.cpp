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
	
	//Check if this object is the class default object (CDO) for the ability,
	//only the CDO will enter this branch to avoid blueprint misconfiguration.
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		//Event triggers are defined in the CDO (class default object) to avoid blueprint misconfiguration.
		//Create a trigger data,set the trigger tag and trigger source and add it to the AbilityTriggers array.
		FAbilityTriggerData TriggerData;
		//only trigger when the tag is HitReact.
		TriggerData.TriggerTag = GCTags::GCEvents::player::HitReact;
		//only trigger from gameplay event, this means only when we call ASC->HandleGameplayEvent with this tag, can trigger this ability.
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		//register this trigger data to the ability, so when the ASC receive the gameplay event with the tag we specified, it will trigger this ability.
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
	
	return BaseCharacter->IsAlive();
}

void UGC_PlayerHitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AMyBaseCharacter* BaseCharacter = Cast<AMyBaseCharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	
	if (!IsValid(BaseCharacter) || !ASC)
	{
		FinishHitReactAbility(true);
		return;
	} 
	
	// if player is dead or has dead tag, just end the ability and don't play hit react.
	if (!BaseCharacter->IsAlive() || ASC->HasMatchingGameplayTag(GCTags::GCEvents::player::Dead))
	{
		FinishHitReactAbility(true);
		return;
	}
	
	const AActor* DamageInstigator = TriggerEventData ? TriggerEventData->Instigator.Get() : nullptr;
	
	TargetForward = BaseCharacter->GetActorForwardVector().GetSafeNormal();
	ToInstigator = IsValid(DamageInstigator) ? (DamageInstigator->GetActorLocation()-BaseCharacter->GetActorLocation()).GetSafeNormal() : TargetForward;
	
	const EHitDirection HitDirection = UGC_BluePrintLibrary::GetHitDirection(TargetForward,ToInstigator);
	HitReactSectionName = UGC_BluePrintLibrary::GetHitReactionName(HitDirection);
	
	//Perform right Section montage in GA_PlayerHitReact blueprint.
	BP_StartHitReact(HitReactSectionName,TargetForward,ToInstigator);
}

void UGC_PlayerHitReact::FinishHitReactAbility(bool bWasCancelled)
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,bWasCancelled);
	}
}
