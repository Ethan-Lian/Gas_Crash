#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GC_GameplayAbility.generated.h"

class AGC_EnemyCharacter;
class AMyBaseCharacter;
class UAbilitySystemComponent;

UCLASS()
class GAS_CRASH_API UGC_GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="GC|Debug")
	bool bDrawDebugs = false;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//get BaseCharacter from AvatarActor
	UFUNCTION(BlueprintCallable, Category="GC|Ability")
	AMyBaseCharacter* GetBaseCharacter() const;

	//Get GC_EnemyCharacter from AvatarActor
	UFUNCTION(BlueprintCallable, Category="GC|Ability")
	AGC_EnemyCharacter* GetGC_EnemyCharacter() const;
};
