#pragma once
#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "GC_AbilitySystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_CRASH_API UGC_AbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;

	virtual void OnRep_ActivateAbilities() override;
	
	UFUNCTION(BlueprintCallable, Category = "GC|Input")
	void AbilityInputTagPressed(const FGameplayTag& InputTag);

	UFUNCTION(BlueprintCallable, Category = "GC|Input")
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	UFUNCTION(BlueprintCallable, Category = "GC|Input")
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

	UFUNCTION(BlueprintCallable, Category = "GC|Input")
	void ClearAbilityInput();
	
	//Set the GameAbility Level
	UFUNCTION(BlueprintCallable,Category="GC|Abilities")
	void SetAbilityLevel(TSubclassOf<UGameplayAbility> AbilityClass,int32 Level);
	
	//Increase Ability Level,default increase 1 level.
	UFUNCTION(BlueprintCallable,Category="GC|Abilities")
	void IncreaseAbilityLevel(TSubclassOf<UGameplayAbility> AbilityClass,int32 IncreaseLevel = 1);
protected:
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	
private:
	// Returns true if the ability was successfully auto-activated or has no ActivateOnGiven tag.
	// Returns false only when TryActivateAbility fails, signaling a retry is needed.
	bool HandleAutoActivatedAbilities(const FGameplayAbilitySpec& AbilitySpec);
private:
	// Tracks specs that have already had auto-activation attempted,
	// so OnRep_ActivateAbilities only processes each newly-added spec once.
	TSet<FGameplayAbilitySpecHandle> AutoActivatedSpecHandles;

	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
};
