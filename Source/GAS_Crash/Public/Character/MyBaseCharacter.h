#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "MyBaseCharacter.generated.h"

class UAbilitySystemComponent;

UCLASS(Abstract)//抽象标记,无法被实例化,专门用来被继承
class GAS_CRASH_API AMyBaseCharacter : public ACharacter,public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMyBaseCharacter();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

};
