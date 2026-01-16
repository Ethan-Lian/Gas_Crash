#include "Player/GC_PlayerState.h"

#include "AbilitySystemComponent.h"

AGC_PlayerState::AGC_PlayerState()
{
	//improve update frequency ?
	SetNetUpdateFrequency(100.f);
	
	//create AbilitySystemComponent
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(FName("AbilitySystemComponent"));
	// let this ASC be replicated  ?
	AbilitySystemComponent->SetIsReplicated(true);
	//?
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	
}

UAbilitySystemComponent* AGC_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
