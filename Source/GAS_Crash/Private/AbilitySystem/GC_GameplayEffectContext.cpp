#include "AbilitySystem/GC_GameplayEffectContext.h"

FGC_GameplayEffectContext* FGC_GameplayEffectContext::ExtractEffectContext(FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* BaseContext = Handle.Get();

	// MUST use GetScriptStruct()->IsChildOf() rather than a raw C-cast or dynamic_cast.
	// GAS stores the context as a base pointer; the struct type info lets us safely verify
	// the concrete type before downcasting, preventing UB on mismatched context types.
	if (BaseContext && BaseContext->GetScriptStruct()->IsChildOf(FGC_GameplayEffectContext::StaticStruct()))
	{
		return static_cast<FGC_GameplayEffectContext*>(BaseContext);
	}

	return nullptr;
}

bool FGC_GameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	// Serialize everything the base context knows about (Instigator, EffectCauser, HitResult, Origin…)
	bool bBaseSuccess = true;
	FGameplayEffectContext::NetSerialize(Ar, Map, bBaseSuccess);

	// Serialize our custom fields on top.
	// SerializeBits(ptr, N) packs N bits — using 1 bit for a bool is optimal bandwidth.
	Ar.SerializeBits(&bIsCriticalHit, 1);

	// FGameplayTag::NetSerialize uses the tag name string under the hood,
	// which is cheap because tags are registered globally and resolved by name on both ends.
	bool bTagSuccess = true;
	DamageTypeTag.NetSerialize(Ar, Map, bTagSuccess);

	// Preserve failure from either layer instead of masking it.
	bOutSuccess = bBaseSuccess && bTagSuccess && !Ar.IsError();
	return bOutSuccess;
}
