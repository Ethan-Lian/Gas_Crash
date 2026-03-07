#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "MyBaseCharacter.generated.h"

struct FOnAttributeChangeData;
class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class UAbilitySystemComponent;

// Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FASCInitalized,UAbilitySystemComponent*,ASC,UAttributeSet*,AS);

//Abstract Mark : cannot be instantiated; intended for inheritance only;
UCLASS(Abstract)
class GAS_CRASH_API AMyBaseCharacter : public ACharacter,public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMyBaseCharacter();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	virtual UAttributeSet* GetAttributeSet() const{ return nullptr;}
	
	bool IsAlive() const {return bAlive;}
	
	void SetAlive(bool AliveStatus) {bAlive = AliveStatus;}
	
	//Listen Health Changed
	void OnHealthChanged(const FOnAttributeChangeData& AttributeChangeData);
	
	//Handle Death
	virtual void HandleDeath();
	
	//Handle Respawn
	UFUNCTION(BlueprintCallable,Category="GC|Death")
	virtual void HandleRespawn();
	
	//Define a delegate for ASC initialization, used for asynchronous callback initialization
	UPROPERTY(BlueprintAssignable)
	FASCInitalized OnAscInitialized;
protected:
	void GiveStartupAbilities();
	
	//Apply Effects to CharacterAttribute
	void InitializeAttribute() const;
	
	//Apply ResetAttributeEffect to CharacterAttribute
	UFUNCTION(BlueprintCallable,Category="GC|Attributes")
	void ResetAttributes();

	//Death state protect, only handle once per death, reset in respawn
	UPROPERTY(EditDefaultsOnly,Category="GC|Death")
	bool bDeathHandled = false;
private:
	UPROPERTY(EditDefaultsOnly,Category="GC|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupGameplayAbilities;
	
	UPROPERTY(EditDefaultsOnly,Category="GC|Effects")
	TSubclassOf<UGameplayEffect> InitializeAttributesEffects;
	
	UPROPERTY(EditDefaultsOnly,Category="GC|Effects")
	TSubclassOf<UGameplayEffect> ResetAttributeEffects;
	
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
	
	UPROPERTY(BlueprintReadOnly,meta=(AllowPrivateAccess = "true"))
	bool bAlive = true;
};
