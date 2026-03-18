#pragma once
#include "CoreMinimal.h"
#include "AbilitySystem/GC_AbilitySet.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "MyBaseCharacter.generated.h"

struct FOnAttributeChangeData;
class UAttributeSet;
class UGameplayEffect;
class UAbilitySystemComponent;
class UGC_HealthComponent;

// Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FASCInitalized,UAbilitySystemComponent*,ASC,UAttributeSet*,AS);

//Abstract Mark : cannot be instantiated; intended for inheritance only;
UCLASS(Abstract)
class GAS_CRASH_API AMyBaseCharacter : public ACharacter,public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMyBaseCharacter();
	
	UGC_HealthComponent* GetHealthComponent() const {return HealthComponent;}
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	virtual UAttributeSet* GetAttributeSet() const{ return nullptr;}
	
	bool IsAlive() const {return bAlive;}
	
	void SetAlive(bool AliveStatus) {bAlive = AliveStatus;}
	
	//Handle Death
	virtual void HandleDeath();
	
	//Handle Respawn
	UFUNCTION(BlueprintCallable,Category="GC|Death")
	virtual void HandleRespawn();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	//Define a delegate for ASC initialization, used for asynchronous callback initialization
	UPROPERTY(BlueprintAssignable)
	FASCInitalized OnAscInitialized;
protected:
	//Give Startup Abilities and Startup(Passive) Effects by AbilitySet, only called once in BeginPlay/OnPossessed.
	void GiveStartupAbilitySets();

	//Clear Startup Abilities and Startup(Passive) Effects by AbilitySet, only called once in EndPlay/OnUnPossessed.
	void ClearStartupAbilitySets();

	// Apply respawn reset effect to restore runtime attributes after death.
	UFUNCTION(BlueprintCallable,Category="GC|Attributes")
	void ResetAttributes();

	//Death state protect, only handle once per death, reset in respawn
	UPROPERTY(EditDefaultsOnly,Category="GC|Death")
	bool bDeathHandled = false;
	
	//Initialize HealthComponent with ASC and AttributeSet.
	void InitializeHealthComponent() const;
private:
	//Startup AbilitySets to grant(授予) when character spawn or possessed, can be set in BP child class.
	UPROPERTY(EditDefaultsOnly, Category = "GC|AbilitySystem")
	TArray<TObjectPtr<UGC_AbilitySet>> StartupAbilitySets;

	// One GrantedHandles entry per AbilitySet (parallel array).
	// This allows selective revocation of a single set's abilities without touching others —
	// critical for equipment swapping (equip A grants ability X; swap to B revokes X, grants Y).
	UPROPERTY()
	TArray<FGC_AbilitySet_GrantedHandles> GrantedAbilitySetHandlesArray;

	UPROPERTY()
	bool bStartupAbilitySetsGranted = false;

	UPROPERTY(EditDefaultsOnly,Category="GC|Effects")
	TSubclassOf<UGameplayEffect> ResetAttributeEffects;
	
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
	
	UPROPERTY(BlueprintReadOnly,meta=(AllowPrivateAccess = "true"))
	bool bAlive = true;
	
	UPROPERTY(EditDefaultsOnly,Category="GC|Component")
	UGC_HealthComponent* HealthComponent;
};
