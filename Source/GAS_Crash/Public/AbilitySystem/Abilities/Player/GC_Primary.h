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
	
	UFUNCTION(BlueprintCallable,Category="GC|Abilities")
	void SendEventToEnemy(const TArray<AActor*>& OverlapActors);
private:
	UPROPERTY(EditDefaultsOnly,Category="GC|Abilities")
	float HitBoxRadius = 100.f;
	
	UPROPERTY(EditDefaultsOnly,Category="GC|Abilities")
	float HitBoxForwardOffset = 200.f;
	
	UPROPERTY(EditDefaultsOnly,Category="GC|Abilities")
	float HitBoxElevationOffset = 20.f;
	
	void DrawHitBoxOverlap(const FVector& Location,const TArray<FOverlapResult>& OverlapResults) const;
	
	
};
