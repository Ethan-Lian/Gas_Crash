#include "AbilitySystem/Abilities/Player/GC_PlayerRespawn.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/GC_PlayerCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

UGC_PlayerRespawn::UGC_PlayerRespawn()
{
	// ServerOnly prevents client prediction issues
	// Respawn involves authoritative state changes (teleport, attribute reset)
	// that must not be predicted by clients
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	// InstancedPerActor maintains per-player respawn state
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGC_PlayerRespawn::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Perform respawn immediately; GameMode handles respawn timing.
	PerformRespawn();
}

void UGC_PlayerRespawn::PerformRespawn()
{
	AGC_PlayerCharacter* PlayerChar = Cast<AGC_PlayerCharacter>(GetAvatarActorFromActorInfo());

	if (!PlayerChar)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Character's HandleRespawn does:
	// 1. Remove death effect (removes Dead tag)
	// 2. Reset bDeathHandled and bAlive flags
	// 3. Re-enable movement and collision
	PlayerChar->HandleRespawn();

	// Find spawn point using UE's PlayerStart system
	// In production, you'd use GameMode's ChoosePlayerStart for team-based spawning
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		APlayerStart::StaticClass(),
		PlayerStarts);

	if (PlayerStarts.Num() > 0)
	{
		// Simple random selection - production code would use GameMode logic
		AActor* SpawnPoint = PlayerStarts[FMath::RandRange(0, PlayerStarts.Num() - 1)];
		PlayerChar->SetActorLocationAndRotation(
			SpawnPoint->GetActorLocation(),
			SpawnPoint->GetActorRotation());
	}
	else
	{
		// Fallback: respawn at world origin if no PlayerStart found
		UE_LOG(LogTemp, Warning, TEXT("No PlayerStart found, respawning at origin"));
		PlayerChar->SetActorLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
	}
}
