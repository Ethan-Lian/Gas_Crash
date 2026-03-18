#include "AbilitySystem/GC_AbilitySystemGlobals.h"
#include "AbilitySystem/GC_GameplayEffectContext.h"

UGC_AbilitySystemGlobals::UGC_AbilitySystemGlobals(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FGameplayEffectContext* UGC_AbilitySystemGlobals::AllocGameplayEffectContext() const
{
	// Every GE context in the game is now our custom type.
	// Any system that calls ASC->MakeEffectContext() or GE->GetEffectContext()
	// will get back a FGC_GameplayEffectContext that can carry crit/damage-type metadata.
	return new FGC_GameplayEffectContext();
}
