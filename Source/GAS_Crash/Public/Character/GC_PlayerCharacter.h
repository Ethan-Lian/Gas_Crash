#pragma once
#include "CoreMinimal.h"
#include "MyBaseCharacter.h"
#include "GC_PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class GAS_CRASH_API AGC_PlayerCharacter : public AMyBaseCharacter
{
	GENERATED_BODY()

public:
	AGC_PlayerCharacter();
	
private:
	UPROPERTY(VisibleAnywhere,Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent;
	
	UPROPERTY(VisibleAnywhere,Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent;	
	
};
