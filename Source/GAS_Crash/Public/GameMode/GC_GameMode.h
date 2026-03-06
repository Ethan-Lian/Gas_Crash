#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GC_GameMode.generated.h"

class AGC_PlayerCharacter;

/**
 * Game Mode with auto-respawn functionality
 * Listens to player death events and triggers respawn after a delay
 */
UCLASS()
class GAS_CRASH_API AGC_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGC_GameMode();

	UFUNCTION(BlueprintCallable, Category = "GC|Respawn")
	void RequestRespawnForPlayer(AGC_PlayerCharacter* DeadPlayer);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GC|Respawn")
	float RespawnDelay = 1.0f;

	// Whether to enable auto-respawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GC|Respawn")
	bool bEnableAutoRespawn = true;

private:
	// Performs the respawn
	void RespawnPlayer(AGC_PlayerCharacter* PlayerCharacter);

	// Stores timer handles for each player's respawn
	TMap<AGC_PlayerCharacter*, FTimerHandle> RespawnTimers;
};
