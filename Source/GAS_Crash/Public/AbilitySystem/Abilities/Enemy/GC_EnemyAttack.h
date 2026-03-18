#pragma once
#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GC_GameplayAbility.h"
#include "Engine/EngineTypes.h"
#include "GC_EnemyAttack.generated.h"

class AActor;
class AGC_Projectile;

UCLASS()
class GAS_CRASH_API UGC_EnemyAttack : public UGC_GameplayAbility
{
	GENERATED_BODY()

public:
	UGC_EnemyAttack();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Attack")
	TSubclassOf<AGC_Projectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Attack")
	FName MuzzleSocketName = FName("Muzzle_Front");

	// ⭐ Positive value — ExecCalc handles the subtraction internally.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Attack")
	float ProjectileDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Attack")
	FVector SpawnLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Attack")
	FVector TargetAimOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Attack")
	ESpawnActorCollisionHandlingMethod CollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

private:
	//Find Target Actor in front of enemy
	AActor* ResolveTargetActor() const;
	
	//Calculate Projectile Spawn Transform based on Target Actor
	bool ResolveSpawnTransform(const AActor* TargetActor, FTransform& OutSpawnTransform) const;

	//Calculate Target Aim Location based on Target Actor
	FVector ResolveTargetAimLocation(const AActor* TargetActor) const;
};
