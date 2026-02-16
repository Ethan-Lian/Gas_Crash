#pragma once
#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GC_GameplayAbility.h"
#include "GC_Respawn.generated.h"

UCLASS()
class GAS_CRASH_API UGC_Respawn : public UGC_GameplayAbility
{
	GENERATED_BODY()
	
public:
	UGC_Respawn();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
	
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
