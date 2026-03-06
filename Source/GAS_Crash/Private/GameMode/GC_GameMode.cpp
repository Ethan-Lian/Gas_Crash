#include "GameMode/GC_GameMode.h"

#include "AbilitySystemComponent.h"
#include "Character/GC_PlayerCharacter.h"
#include "GameplayTags/GCTags.h"

AGC_GameMode::AGC_GameMode()
{
	// Set default pawn class if needed
	// DefaultPawnClass = AGC_PlayerCharacter::StaticClass();
}

void AGC_GameMode::RequestRespawnForPlayer(AGC_PlayerCharacter* DeadPlayer)
{
	if (!HasAuthority() || !bEnableAutoRespawn || !IsValid(DeadPlayer)) return;

	if (DeadPlayer->IsAlive()) return;
	
	if (RespawnDelay > 0.f)
	{
		FTimerHandle& TimerHandle = RespawnTimers.FindOrAdd(DeadPlayer);
		if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle)) return;

		FTimerDelegate RespawnDelegate;
		RespawnDelegate.BindUObject(this, &ThisClass::RespawnPlayer, DeadPlayer);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, RespawnDelegate, RespawnDelay, false);

		UE_LOG(LogTemp, Log, TEXT("Player %s died, respawning in %.1f seconds"),*DeadPlayer->GetName(), RespawnDelay);
	}
	else
	{
		RespawnPlayer(DeadPlayer);
	}
	
}

void AGC_GameMode::RespawnPlayer(AGC_PlayerCharacter* PlayerCharacter)
{
	if (!IsValid(PlayerCharacter)) 
	{
		// Player might have disconnected or been destroyed, clean up timer handle
		RespawnTimers.Remove(PlayerCharacter);
		return;
	}

	UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();
	if (!ASC) 
	{
		RespawnTimers.Remove(PlayerCharacter);
		return;
	}

	// Activate respawn ability by tag
	// The ability handles all respawn logic (remove death effect, teleport, etc.)
	bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(GCTags::GCAbilities::player::Respawn));

	//Clean up timer handle
	RespawnTimers.Remove(PlayerCharacter);
	
	if (bActivated)
	{
		UE_LOG(LogTemp, Log, TEXT("Player %s respawned"), *PlayerCharacter->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to activate respawn ability for %s"),*PlayerCharacter->GetName());
	}
}
