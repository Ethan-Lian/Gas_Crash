#pragma once
#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"


// This struct is used to send damage feedback data from AttributeSet to HealthComponent, 
// because AttributeSet is the only one who can confirm the damage type after GE executed, 
// and HealthComponent is responsible for sending GameplayCue and GameplayEvent,
// so we need this struct to send the damage feedback data to HealthComponent.

struct FGC_DamageFeedbackData
{
	//final damage value
	float DamageMagnitude = 0.f;
	
	//check if Fatal(致死伤害)
	bool bIsFatal = false;
	
	//the source of Damage
	FGameplayTag DamageTypeTag;
	
	//the instigator of this damage
	TWeakObjectPtr<AActor> Instigator;
	
	//the real make this damage happened object such as projectile
	TWeakObjectPtr<AActor> EffectCauser;
	
	//the EffectContext of this damage contain more information such as hitresult.
	FGameplayEffectContextHandle EffectContext;
};
