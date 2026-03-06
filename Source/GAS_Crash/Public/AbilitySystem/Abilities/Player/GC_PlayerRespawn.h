#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GC_GameplayAbility.h"
#include "GC_PlayerRespawn.generated.h"

class UAnimMontage;

/**
 * Player Respawn Ability
 * Handles respawn logic: removes death effect, restores movement, teleports to spawn point
 * Triggered by external systems (GameMode, PlayerController, UI)
 */
UCLASS()
class GAS_CRASH_API UGC_PlayerRespawn : public UGC_GameplayAbility
{
	GENERATED_BODY()

public:
	UGC_PlayerRespawn();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
		
private:
	UFUNCTION()
	void PerformRespawn();
};
