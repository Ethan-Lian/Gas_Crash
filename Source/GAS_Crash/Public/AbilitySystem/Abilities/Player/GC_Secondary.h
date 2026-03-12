#pragma once
#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GC_GameplayAbility.h"
#include "GC_Secondary.generated.h"

class UGameplayEffect;
class UCurveFloat;

UCLASS()
class GAS_CRASH_API UGC_Secondary : public UGC_GameplayAbility
{
	GENERATED_BODY()

public:
	// Called by GA_Secondary blueprint when the montage hit frame event arrives.
	UFUNCTION(BlueprintCallable, Category = "GC|Secondary")
	void ExecuteSecondaryImpact();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Secondary|Damage")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Secondary|Damage")
	float BaseDamage = -40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Secondary|AOE", meta = (ClampMin = "0.0"))
	float AOERadius = 400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Secondary|Knockback")
	float BaseHorizontalKnockback = 1200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Secondary|Knockback")
	float BaseVerticalKnockback = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Secondary|Curves")
	TObjectPtr<UCurveFloat> DamageFalloffCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Secondary|Curves")
	TObjectPtr<UCurveFloat> KnockbackFalloffCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Secondary|Knockback", meta = (ClampMin = "0.05"))
	float KnockbackRecoveryDelay = 0.65f;

private:
	//Helper functions for executing the secondary impact logic

	//Resolve the center of the secondary impact.
	FVector ResolveImpactCenter() const;

	//Query the world for valid AOE targets within the specified radius
	TArray<AActor*> QueryAOETargets(const FVector& Center) const;
	
	//check if the candidate actor is a valid target for the AOE effect
	bool IsEligibleAOETarget(AActor* CandidateActor) const;
	
	//Apply damage effect and knockback
	void ApplyAOEToTargets(const TArray<AActor*>& Targets, const FVector& Center);
	
	void ApplyDamageToTarget(AActor* TargetActor, float DamageScale) const;
	
	void ApplyKnockbackToTarget(AActor* TargetActor, const FVector& Center, float KnockbackScale) const;
	
	//Trigger the explosion gameplay cue at the impact center
	void ExecuteExplosionCue(const FVector& Center) const;
	
	//Debug function to visualize the AOE radius and affected targets.
	void DrawDebugAOE(const FVector& Center, const TArray<AActor*>& Targets) const;

};
