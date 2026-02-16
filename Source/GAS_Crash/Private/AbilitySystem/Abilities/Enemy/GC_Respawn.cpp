#include "AbilitySystem/Abilities/Enemy/GC_Respawn.h"

#include "Character/GC_EnemyCharacter.h"

UGC_Respawn::UGC_Respawn()
{
	//Respawn ability should only be excuted on the server, and each enemy should have its own instance of this ability to manage its respawn state.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGC_Respawn::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                  const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (AGC_EnemyCharacter* EnemyCharacter = GetGC_EnemyCharacter())
	{
		EnemyCharacter->HandleRespawn();
	}
	
	//End the ability immediately after triggering the respawn logic.
	EndAbility(Handle,ActorInfo,ActivationInfo,false,false);
}
