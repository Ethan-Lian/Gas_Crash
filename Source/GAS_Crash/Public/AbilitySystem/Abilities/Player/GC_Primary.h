#pragma once
#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GC_GameplayAbility.h"
#include "GC_Primary.generated.h"


UCLASS()
class GAS_CRASH_API UGC_Primary : public UGC_GameplayAbility
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable,Category="GC|Abilities")
	TArray<AActor*> HitBoxOverlapTest();
	
	// Send GameEvent which included Tag Payload messages to Enemy.
	UFUNCTION(BlueprintCallable,Category="GC|Abilities")
	void SendEventToEnemy(const TArray<AActor*>& OverlapActors);
private:
	UPROPERTY(EditDefaultsOnly,Category="GC|Abilities")
	float HitBoxRadius = 135.f;
	
	UPROPERTY(EditDefaultsOnly,Category="GC|Abilities")
	float HitBoxForwardOffset = 150.f;
	
	UPROPERTY(EditDefaultsOnly,Category="GC|Abilities")
	float HitBoxElevationOffset = 20.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "GC|Abilities")
	float BaseDamage = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "GC|Abilities")
	TSubclassOf<UGameplayEffect> DamageEffect;
	
	void DrawHitBoxOverlap(const FVector& Location,const TArray<FOverlapResult>& OverlapResults) const;
	
	//Apply DamageEffect to HitResult  
	UFUNCTION(BlueprintCallable)  
	void ApplyDamageEffectToHitResult(const TArray<AActor*>& Actors);
};
