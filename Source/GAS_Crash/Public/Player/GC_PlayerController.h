#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GC_PlayerController.generated.h"

struct FGameplayTag;
struct FInputActionValue;
class UEnhancedInputComponent;
class UGC_AbilitySystemComponent;
class UGC_InputConfig;
class UInputMappingContext;

UCLASS()
class GAS_CRASH_API AGC_PlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	virtual void SetupInputComponent() override;
	
	virtual void PlayerTick(float DeltaTime) override;

	bool IsDied() const;
	
	UGC_AbilitySystemComponent* GetGCAbilitySystemComponent() const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "GC|Input")
	TObjectPtr<UGC_InputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "GC|Input")
	TArray<TObjectPtr<UInputMappingContext>> InputMappingContexts;

	void BindNativeInputActions(UEnhancedInputComponent* EnhancedInputComponent);
	
	void BindAbilityInputActions(UEnhancedInputComponent* EnhancedInputComponent);
	
	void Input_JumpPressed();
	
	void Input_JumpReleased();
	
	void Input_Look(const FInputActionValue& Value);
	
	void Input_Move(const FInputActionValue& Value);
	
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
};
