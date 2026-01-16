#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GC_PlayerController.generated.h"

struct FGameplayTag;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;


UCLASS()
class GAS_CRASH_API AGC_PlayerController : public APlayerController
{
	GENERATED_BODY()
protected:
	virtual void SetupInputComponent() override;
	
private:
	//InputMappingContext
	UPROPERTY(EditDefaultsOnly,Category="GC|InputMappingContext")
	TArray<TObjectPtr<UInputMappingContext>> InputMappingContexts;
	
	//InputAction
	UPROPERTY(EditDefaultsOnly,Category="GC|InputAction")
	TObjectPtr<UInputAction> JumpAction;
	
	UPROPERTY(EditDefaultsOnly,Category="GC|InputAction")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditDefaultsOnly,Category="GC|InputAction")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditDefaultsOnly,Category="GC|AbilitiesAction")
	TObjectPtr<UInputAction> PrimaryAction;
	
	UPROPERTY(EditDefaultsOnly,Category="GC|AbilitiesAction")
	TObjectPtr<UInputAction> SecondaryAction;
	
	UPROPERTY(EditDefaultsOnly,Category="GC|AbilitiesAction")
	TObjectPtr<UInputAction> TertiaryAction;
	
	//Callback function
	void Jump();
	void JumpStop();
	void Look(const FInputActionValue& value);
	void Move(const FInputActionValue& value);
	void Primary();
	void Secondary();
	void Tertiary();
	
	//Activate Ability Function by Tag
	void ActivateAbility(const FGameplayTag& AbilityTag) const;
};
