#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "GC_AbilitySystemGlobals.generated.h"

/**
 * UGC_AbilitySystemGlobals
 *
 * UAbilitySystemGlobals is a singleton that acts as the factory for core GAS objects.
 * AllocGameplayEffectContext() is called every time ANY GameplayEffect is applied.
 * By overriding it here, every GE in the entire game automatically gets our custom
 * FGC_GameplayEffectContext instead of the default one — zero per-ability setup needed.
 *
 * Registered in DefaultGame.ini under [/Script/GameplayAbilities.AbilitySystemGlobals]
 * AbilitySystemGlobalsClassName=/Script/GAS_Crash.GC_AbilitySystemGlobals
 */
UCLASS(Config = Game)
class GAS_CRASH_API UGC_AbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	UGC_AbilitySystemGlobals(const FObjectInitializer& ObjectInitializer);

	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
