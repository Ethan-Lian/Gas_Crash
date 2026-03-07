#pragma once
#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GC_GameplayAbility.h"
#include "GC_PlayerHitReact.generated.h"

UCLASS()
class GAS_CRASH_API UGC_PlayerHitReact : public UGC_GameplayAbility
{
	GENERATED_BODY()
	
public:
	UGC_PlayerHitReact();
	
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "GC|HitReact")
	FVector TargetForward = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "GC|HitReact")
	FVector ToInstigator = FVector::ForwardVector;

	UPROPERTY(BlueprintReadOnly, Category = "GC|HitReact")
	FName HitReactSectionName = NAME_None;

	UFUNCTION(BlueprintImplementableEvent, Category = "GC|HitReact")
	void BP_StartHitReact(FName InSectionName, FVector InTargetForward, FVector InToInstigator);

	UFUNCTION(BlueprintCallable, Category = "GC|HitReact")
	void FinishHitReactAbility(bool bWasCancelled = false);
};
