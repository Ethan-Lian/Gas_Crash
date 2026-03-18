#pragma once
#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "AbilitySystem/Abilities/GC_GameplayAbility.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "GC_AbilitySet.generated.h"

class UGC_AbilitySystemComponent;
class UGC_GameplayAbility;
class UGameplayEffect;
class UObject;

USTRUCT(BlueprintType)
struct FGC_AbilitySet_GameplayAbility
{
	GENERATED_BODY()
	
public:
	// The GameplayAbility to grant.
	UPROPERTY(EditDefaultsOnly,Category="Ability")
	TSubclassOf<UGC_GameplayAbility> Ability;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	int32 AbilityLevel = 1;

	//This tag is written into AbilitySpec dynamic tags.Later ASC uses this tag to route input to the right ability spec.
	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

USTRUCT(BlueprintType)
struct FGC_AbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:
	//The GameplayEffect to grant.
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GameplayEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float EffectLevel = 1.f;
};

//This struct holds the grantedHandles(已授予的句柄,record the all has granted ability and effect) for a given AbilitySet,
//allowing for selective revocation(选择性撤销) later(such as when the AbilitySet is removed).
USTRUCT(BlueprintType)
struct FGC_AbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:
	//Add granted AbilitySpecHandle to this Struct
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	//Add granted GameplayEffectHandle to this Struct
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);

	//Clear granted Abilities and Effects from ASC by the cached granted Handles.
	void TakeFromAbilitySystem(UGC_AbilitySystemComponent* GCASC);

private:
	// Store granted AbilitySpecHandles and GameplayEffectHandles, so they can be removed later when the set is removed.
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;
};


UCLASS()
class GAS_CRASH_API UGC_AbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	void GiveToAbilitySystem(UGC_AbilitySystemComponent* GCASC, FGC_AbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta = (TitleProperty = "Ability"))
	TArray<FGC_AbilitySet_GameplayAbility> GrantedGameplayAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta = (TitleProperty = "GameplayEffect"))
	TArray<FGC_AbilitySet_GameplayEffect> GrantedGameplayEffects;
};
