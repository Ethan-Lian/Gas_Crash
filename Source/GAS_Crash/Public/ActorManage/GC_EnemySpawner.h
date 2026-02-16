#pragma once
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "GC_EnemySpawner.generated.h"

class AGC_EnemyCharacter;

UCLASS()
class GAS_CRASH_API AGC_EnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AGC_EnemySpawner();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere,Category="GC|Spawn")
	TSubclassOf<AGC_EnemyCharacter> EnemyClass;
	
	UPROPERTY(EditAnywhere,Category="GC|Spawn")
	float RespawnDelay = 2.f;

	UPROPERTY(EditAnywhere,Category="GC|Spawn", meta=(ClampMin="0.1"))
	float RetryDelay = 1.f;

	UPROPERTY(EditAnywhere,Category="GC|Spawn")
	ESpawnActorCollisionHandlingMethod SpawnCollisionHandling = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
private:
	UPROPERTY()
	TObjectPtr<AGC_EnemyCharacter> SpawnedEnemy;
	
	FTimerHandle RespawnTimerHandle;
	
	FTransform SpawnTransform;
	
	UFUNCTION()
	void HandleEnemyDied(AGC_EnemyCharacter* DeadEnemy);
	
	void SpawnEnemyIfNeeded();
	
	// Handle Enemy Respawn by call enemy's RespawnAbility.
	void TriggerRespawn();

	// Schedule a retry to spawn enmey if the initial spawn failed.
	void ScheduleSpawnRetry();
};
