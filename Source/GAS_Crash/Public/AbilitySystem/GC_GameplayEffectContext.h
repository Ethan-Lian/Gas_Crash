#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "GC_GameplayEffectContext.generated.h"

/**
 * FGC_GameplayEffectContext
 *
 * The base FGameplayEffectContext only carries Instigator, EffectCauser, and HitResult.
 * To pass richer combat metadata (critical hits, damage type) from an Ability into the
 * ExecutionCalculation and downstream systems (AttributeSet, GameplayCue), we MUST
 * subclass this and override NetSerialize() so the extra fields survive network travel.
 *
 * Without this, ExecCalc is blind to "why" the damage happened — it only sees raw numbers.
 */
USTRUCT()
struct GAS_CRASH_API FGC_GameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	FGC_GameplayEffectContext() : FGameplayEffectContext() {}
	FGC_GameplayEffectContext(AActor* InInstigator, AActor* InEffectCauser)
		: FGameplayEffectContext(InInstigator, InEffectCauser) {}

	// --- Static helpers ---

	/**
	 * Safely downcasts a FGameplayEffectContextHandle to our custom type.
	 * Always use this instead of a raw C-cast so we don't crash on mismatched types.
	 */
	static FGC_GameplayEffectContext* ExtractEffectContext(struct FGameplayEffectContextHandle Handle);

	// --- Accessors ---

	bool IsCriticalHit() const { return bIsCriticalHit; }
	
	void SetIsCriticalHit(bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; }

	FGameplayTag GetDamageTypeTag() const { return DamageTypeTag; }
	
	void SetDamageTypeTag(const FGameplayTag& InTag) { DamageTypeTag = InTag; }

	// --- Overrides (required for custom context to work correctly) ---

	/** Must override — returns the concrete struct type so GAS can identify and upcast safely. */
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGC_GameplayEffectContext::StaticStruct();
	}

	/** Must override — deep-copies all fields including our custom ones. */
	virtual FGameplayEffectContext* Duplicate() const override
	{
		FGC_GameplayEffectContext* NewContext = new FGC_GameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	/**
	 * Must override — serializes our custom fields so they survive the network trip.
	 * Without this, bIsCriticalHit and DamageTypeTag are lost on clients / simulated proxies.
	 */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

protected:
	/** Whether this hit was a critical strike. Serialized and sent to clients. */
	UPROPERTY()
	bool bIsCriticalHit = false;

	/**
	 * Semantic damage type (e.g. DamageType.Melee, DamageType.Projectile).
	 * Stored here instead of as a SetByCaller tag so the ExecCalc and AttributeSet
	 * can read the type without caring about which data channel sent the magnitude.
	 */
	UPROPERTY()
	FGameplayTag DamageTypeTag;
};

// TStructOpsTypeTraits tells UE's reflection and network systems that this struct
// provides custom NetSerialze (WithNetSerializer) and can be safely copied (WithCopy).
// Without WithNetSerializer = true, the engine falls back to a generic blob serializer
// that doesn't know about our extra fields.
template<>
struct TStructOpsTypeTraits<FGC_GameplayEffectContext>
	: public TStructOpsTypeTraitsBase2<FGC_GameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy          = true,
	};
};
