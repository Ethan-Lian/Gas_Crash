#pragma once
#include "CoreMinimal.h"
#include "MyBaseCharacter.h"
#include "GameplayTagContainer.h"
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
protected:
	virtual void BeginPlay() override;
	void Handle_DeathAbiltyAndDeathEffect();

	virtual void HandleDeath() override;
private:
	
	UPROPERTY(EditDefaultsOnly,Category="GC|AbilitySystem")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributesSet;
	
	//prevent multiple handling death when receive multiple damage at the same time
	UPROPERTY()
	bool bDeathHandled = false;
};
