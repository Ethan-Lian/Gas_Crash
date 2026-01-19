#include "Player/GC_PlayerState.h"
#include "AbilitySystem/GC_AbilitySystemComponent.h"

AGC_PlayerState::AGC_PlayerState()
{
	//improve update frequency
	SetNetUpdateFrequency(100.f);
	//create AbilitySystemComponent
	AbilitySystemComponent = CreateDefaultSubobject<UGC_AbilitySystemComponent>(FName("AbilitySystemComponent"));
	// make ASC can be replicated,if false, server's change will not transform to client.  
	AbilitySystemComponent->SetIsReplicated(true);
	// set ReplicationMode
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
}

UAbilitySystemComponent* AGC_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
