#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GC_GameplayAbility.h"
#include "GC_PlayerDeath.generated.h"

class UAnimMontage;

/**
 * Player Death Ability
 * Handles death animation and disables movement/collision
 * Triggered by HandleDeath() when health reaches 0
 */
UCLASS()
class GAS_CRASH_API UGC_PlayerDeath : public UGC_GameplayAbility
{
	GENERATED_BODY()

public:
	UGC_PlayerDeath();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// Whether to disable movement on death
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Movement")
	bool bDisableMovementOnDeath = true;

	// Whether to disable collision on death (prevents ragdoll from being pushed)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Collision")
	bool bDisableCollisionOnDeath = true;
};
