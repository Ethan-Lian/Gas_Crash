#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GC_Projectile.generated.h"

class UGameplayEffect;
class UProjectileMovementComponent;

UCLASS()
class GAS_CRASH_API AGC_Projectile : public AActor
{
	GENERATED_BODY()

public:
	AGC_Projectile();
	
	//when overlap actor auto trigger this function.
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="GC|Damage",meta=(ExposeOnSpawn,ClampMin="0.0"))
	float Damage = -20.f;
	
	UFUNCTION(BlueprintImplementableEvent,Category="GC|Projectile")
	void SpawnImpactEffects();
private:
	UPROPERTY(VisibleAnywhere,Category="GC|Projectile")
	TObjectPtr<UProjectileMovementComponent> MovementComponent;
	
	UPROPERTY(EditAnywhere,Category="GC|Damage")
	TSubclassOf<UGameplayEffect> DamageEffect;
};
