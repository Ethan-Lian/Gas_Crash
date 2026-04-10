#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "AbilitySystem/Abilities/GC_GameplayAbility.h"
#include "GC_Tertiary.generated.h"

class UAbilityTask_WaitDelay;
class UGameplayEffect;

UCLASS()
class GAS_CRASH_API UGC_Tertiary : public UGC_GameplayAbility
{
	GENERATED_BODY()

public:
	// Initialize the tertiary ability defaults for super-armor gameplay.
	UGC_Tertiary();

	// Reject activation when the owner is dead, invalid, or already in super armor.
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	// Apply the temporary buff effect, then keep the ability alive until the buff duration expires.
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	// Remove the active buff effect handle when the ability ends or gets cancelled.
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	// Optional presentation hook for blueprint child classes.
	UFUNCTION(BlueprintImplementableEvent, Category = "GC|Tertiary")
	void BP_OnTertiaryStarted();

	// Optional presentation cleanup hook for blueprint child classes.
	UFUNCTION(BlueprintImplementableEvent, Category = "GC|Tertiary")
	void BP_OnTertiaryFinished(bool bWasCancelled);

	// Buff effect class that grants temporary armor and super-armor state.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Tertiary")
	TSubclassOf<UGameplayEffect> TertiaryBuffEffectClass;

	// GameplayCue strength value passed to the tertiary release cue.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Tertiary|Cue")
	float TertiaryBuffCueMagnitude = 50.0f;

public:
	// Start the persistent tertiary buff gameplay cue on the owner.
	UFUNCTION(BlueprintCallable, Category = "GC|Tertiary|Cue")
	void AddTertiaryBuffCue() const;

	// Stop the persistent tertiary buff gameplay cue on the owner.
	UFUNCTION(BlueprintCallable, Category = "GC|Tertiary|Cue")
	void RemoveTertiaryBuffCue() const;

private:
	// Finish the tertiary ability after the buff duration elapses.
	UFUNCTION()
	void HandleTertiaryDurationFinished();

	// Remove the cached buff effect handle from the owning ASC.
	void RemoveTertiaryBuffEffect();

private:
	UPROPERTY()
	FActiveGameplayEffectHandle ActiveTertiaryBuffHandle;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitDelay> ActiveDurationTask;
};
