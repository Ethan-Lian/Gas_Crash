#pragma once
#include "CoreMinimal.h"
#include "MyBaseCharacter.h"
#include "GameplayEffectTypes.h"
#include "Math/TransformNonVectorized.h"
#include "GC_EnemyCharacter.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;
struct FHitResult;

// Forward declare delegate for enemy death event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDied,AGC_EnemyCharacter*, EnemyCharacter);

UCLASS()
class GAS_CRASH_API AGC_EnemyCharacter : public AMyBaseCharacter
{
	GENERATED_BODY()

public:
	AGC_EnemyCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual UAttributeSet* GetAttributeSet() const override;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="GC|AI")
	float AttackRadius = 800;
	
	//====================Death Event====================
	
	//Event triggered when enemy died
	UPROPERTY(BlueprintAssignable, Category="GC|Event")
	FOnEnemyDied OnEnemyDied;
	
	//GE_Death
	UPROPERTY(EditAnywhere,Category="GC|Effects")
	TSubclassOf<UGameplayEffect> GE_Death;
	
	//====================Respawn Event===================
	
	//Handle Respawn called by Respawn GameplayAbility,reset attributes and location.
	virtual void HandleRespawn() override;
	
	//Set Respawn Transform , called by Spawner when spawn nemy
	void SetRespawnTransform(const FTransform& InTransform);

	//====================Knockback State====================

	//Enter Knockback State, called by GC_Secondary when apply knockback to enemy
	void EnterKnockbackState(const FVector& LaunchVelocity, float RecoveryDelay);
protected:
	virtual void BeginPlay() override;
	
	//Handle Death,called when receive damage and health is 0,handle death effect and death ability and stop ai logic.
	virtual void HandleDeath() override;
	
	//Stop AI Logic and Clear Blackboard Value
	void StopAILogicAndClearBlackboradValue();
	
	//Handle Death Effect and Death Ability when Enemy died.
	void Handle_DeathAbilityAndDeathEffect();

	//Handle Dead Tag Changed,called when Dead Tag count changed
	void OnDeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	
	UFUNCTION()
	void OnKnockbackLanded(const FHitResult& HitResult);
private:
	
	UPROPERTY(EditDefaultsOnly,Category="GC|AbilitySystem")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributesSet;
	
	//Cache Respawn Transform, set by spawner when spawn enemy, used to reset enemy location when respawn.
	FTransform RespawnTransform;
	
	//Cache the handle of death effect, used to remove death effect when respawn.
	FActiveGameplayEffectHandle ActivateDeathEffectHandle;

	//Exit Knockback State, called when knockback landed or knockback recovery timeout, resume ai logic and movement.
	void ExitKnockback();
	
	//Handle Knockback Recovery Timeout, called when knockback recovery timer expired, resume ai logic and movement.
	void HandleKnockbackRecoveryTimeout();
	
	//Pause AI logic and movement when enemy is in knockback state, this is used to prevent weird movement like keep chasing player when enemy is knocked back.
	void PauseAILogicForKnockback();
	
	//Resume AI logic and movement after knockback recovery, this is used to resume normal behavior after knockback ends.
	void ResumeAILogicAfterKnockback();

	FTimerHandle KnockbackRecoveryTimerHandle;

	// Prevent duplicate AddDynamic / ResumeLogic mismatches.
	bool bKnockbackLandedBound = false;
	bool bKnockbackPausedBrainLogic = false;
};
