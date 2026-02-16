#pragma once
#include "CoreMinimal.h"
#include "MyBaseCharacter.h"
#include "GameplayEffectTypes.h"
#include "Math/TransformNonVectorized.h"
#include "GC_EnemyCharacter.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;

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
	float AcceptanceRadius = 500.f;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="GC|AI")
	float AttackRadius = 150;
	
	//====================Death Event====================
	
	//Event triggered when enemy died
	UPROPERTY(BlueprintAssignable, Category="GC|Event")
	FOnEnemyDied OnEnemyDied;
	
	//GE_Death
	UPROPERTY(EditAnywhere,Category="GC|Abilties")
	TSubclassOf<UGameplayEffect> GE_Death;
	
	//====================Respawn Event===================
	
	//Handle Respawn called by Respawn GameplayAbility,reset attributes and location.
	virtual void HandleRespawn() override;
	
	//Set Respawn Transform , called by Spawner when spawn nemy
	void SetRespawnTransform(const FTransform& InTransform);
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
private:
	
	UPROPERTY(EditDefaultsOnly,Category="GC|AbilitySystem")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributesSet;
	
	//prevent multiple handling death when receive multiple damage at the same time
	UPROPERTY()
	bool bDeathHandled = false;
	
	//Cache Respawn Transform, set by spawner when spawn enemy, used to reset enemy location when respawn.
	FTransform RespawnTransform;
	
	//Cache the handle of death effect, used to remove death effect when respawn.
	FActiveGameplayEffectHandle ActivateDeathEffectHandle;
};
