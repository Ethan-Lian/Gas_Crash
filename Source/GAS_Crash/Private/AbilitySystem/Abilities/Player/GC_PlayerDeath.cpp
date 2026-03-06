#include "AbilitySystem/Abilities/Player/GC_PlayerDeath.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTags/GCTags.h"

UGC_PlayerDeath::UGC_PlayerDeath()
{
	// InstancedPerActor ensures each player has their own ability instance
	// This is important for tracking per-player death state (montage task, etc.)
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// ServerInitiated ensures death animation plays on all clients
	// Server activates, then replicates to clients for synchronized death presentation
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UGC_PlayerDeath::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Disable movement and collision to prevent:
	// 1. Player input affecting dead character
	// 2. Physics interactions pushing the corpse around
	// 3. Overlap events triggering on dead body
	if (bDisableMovementOnDeath)
	{
		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			MovementComponent->DisableMovement();
			MovementComponent->StopMovementImmediately();
		}
	}

	if (bDisableCollisionOnDeath)
	{
		Character->SetActorEnableCollision(false);
	}

	// Cancel only attack abilities; keep this death ability running for montage playback.
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		FGameplayTagContainer AttackAbilityTags;
		AttackAbilityTags.AddTag(GCTags::GCAbilities::player::Primary);
		AttackAbilityTags.AddTag(GCTags::GCAbilities::player::Secondary);
		AttackAbilityTags.AddTag(GCTags::GCAbilities::player::Tertiary);

		//CancelAbilities can stop(not delete ability) abilities by tag
		ASC->CancelAbilities(&AttackAbilityTags, nullptr, this);
	}
}
