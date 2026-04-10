#include "ActorManage/GC_EnemySpawner.h"
#include "AbilitySystemComponent.h"
#include "Character/GC_EnemyCharacter.h"
#include "Engine/World.h"
#include "GameplayTags/GCTags.h"


AGC_EnemySpawner::AGC_EnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGC_EnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	
	// only manage respawns in Server.
	if (!HasAuthority()) return;
	
	/*
	 Cache the Spawn Transform, we will use it to spawn enemy and respawn enemy.
	 Transform is a struct that contains location,rotation and scale.
	 We can use GetActorTransform to get the transform of the spawner actor.
	 So we can spawn enemy at the same location and rotation as the spawner.
	*/
	SpawnTransform = GetActorTransform();

	SpawnEnemyIfNeeded();
}

void AGC_EnemySpawner::SpawnEnemyIfNeeded()
{
	if (!HasAuthority()) return;
	if (!EnemyClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemySpawner] EnemyClass is null on %s"), *GetName());
		return;
	}
	if (IsValid(SpawnedEnemy)) return;

	UWorld* World = GetWorld();
	if (!IsValid(World)) return;

	//FActorSpawnParameters is a struct that contains parameters for spawning an actor,such as the collision handling method.
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = SpawnCollisionHandling;

	//Spawn the New enemy at the SpawnTransform,with the specified SpawnParams.
	AGC_EnemyCharacter* NewEnemy = World->SpawnActor<AGC_EnemyCharacter>(EnemyClass, SpawnTransform, SpawnParams);
	if (!IsValid(NewEnemy))
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemySpawner] Spawn failed on %s, retry in %.2fs"), *GetName(), RetryDelay);
		ScheduleSpawnRetry();
		return;
	}

	SpawnedEnemy = NewEnemy;
	//Subscribe the Enemy Died Delegate,Bind the HandleEnemyDied function to the OnEnemyDied event of the spawned enemy.
	SpawnedEnemy->OnEnemyDied.AddUniqueDynamic(this, &ThisClass::HandleEnemyDied);
	//Set the Respawn Transform to the spawned enemy,so the enemy can reset to this transform when respawned.
	SpawnedEnemy->SetRespawnTransform(SpawnTransform);
}

void AGC_EnemySpawner::HandleEnemyDied(AGC_EnemyCharacter* DeadEnemy)
{
	if (!HasAuthority()) return;
	if (!IsValid(DeadEnemy)) return;
	//Make sure the enemy that died is the one spawned by this spawner,avoid handling other enemy's death.
	if (SpawnedEnemy != DeadEnemy) return;

	GetWorldTimerManager().SetTimer(
		RespawnTimerHandle,
		this,
		&AGC_EnemySpawner::TriggerRespawn,
		RespawnDelay,
		false
		);
}

void AGC_EnemySpawner::TriggerRespawn()
{
	//server only
	if (!HasAuthority()) return;

	//if the enemy is destroyed when the respawn timer expires, we need to spawn a new enemy instead of trying to trigger respawn ability.
	if (!IsValid(SpawnedEnemy))
	{
	SpawnEnemyIfNeeded(); 
	return;
	}

	if (UAbilitySystemComponent* ASC = SpawnedEnemy->GetAbilitySystemComponent())
	{
		const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(GCTags::GCAbilities::Enemy::Respawn));
		if (!bActivated)
		{
			UE_LOG(LogTemp, Warning, TEXT("[EnemySpawner] Respawn ability activation failed on %s, retry in %.2fs"), *GetName(), RetryDelay);
			GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AGC_EnemySpawner::TriggerRespawn, RetryDelay, false);
		}
	}
}

void AGC_EnemySpawner::ScheduleSpawnRetry()
{
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AGC_EnemySpawner::SpawnEnemyIfNeeded, RetryDelay, false);
}
