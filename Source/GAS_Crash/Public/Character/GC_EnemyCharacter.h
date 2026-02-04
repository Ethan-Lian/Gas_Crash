#pragma once
#include "CoreMinimal.h"
#include "MyBaseCharacter.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "GC_EnemyCharacter.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;
class UPawnSensingComponent;

UCLASS()
class GAS_CRASH_API AGC_EnemyCharacter : public AMyBaseCharacter
{
	GENERATED_BODY()

public:
	AGC_EnemyCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual UAttributeSet* GetAttributeSet() const override;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="GC|AI")
	float AcceptanceRadius = 500.f;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="GC|AI")
	float MinAttackDelay = .1f;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="GC|AI")
	float MaxAttackDelay = .5f;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="GC|AI")
	float AttackRadius = 150;

	//GET Random Attack Delay [Min, Max]
	UFUNCTION(BlueprintCallable, Category="GC|AI", meta=(CompactNodeTitle="Random Delay"))
	float GetRandomAttackDelay() const;
	
	//====================Ability Query API====================
	
	//Check Ability is Ready by Tag
	UFUNCTION(BlueprintPure, Category="GC|Ability")
	bool IsAbilityReady(const FGameplayTag& AbilityTag) const;
    
	//Try to Activate Ability by Tag
	UFUNCTION(BlueprintCallable, Category="GC|Ability")
	bool TryActivateAbilityByTag(const FGameplayTag& AbilityTag);
    
	//Check any ability is ready
	UFUNCTION(BlueprintPure, Category="GC|Ability")
	bool HasAnyAbilityReady(const FGameplayTagContainer& AbilityTags);

protected:
	virtual void BeginPlay() override;

private:
	
	UPROPERTY(EditDefaultsOnly,Category="GC|AbilitySystem")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributesSet;

	//SensingComponent used to sensing player
	UPROPERTY(VisibleAnywhere, Category="GC|AI")
	TObjectPtr<UPawnSensingComponent> PawnSensingComponent;
	
	// ================Cache Ability============
	
	// AbilityHandle Cache(Tag -> SpecHandle Mapping) used to search quickly 
	UPROPERTY()
	TMap<FGameplayTag,FGameplayAbilitySpecHandle>  AbilityHandleCache;
	
	//Build Ability Cache , called after Given ability
	void BuildAbilityCache();
	
	//Callback Function when PawnSensing sees a pawn
	UFUNCTION()
	void OnSeePawn(APawn* SeenPawn);
};
