#pragma once
#include "CoreMinimal.h"
#include "MyBaseCharacter.h"
#include "GC_EnemyCharacter.generated.h"

class UAbilitySystemComponent;

UCLASS()
class GAS_CRASH_API AGC_EnemyCharacter : public AMyBaseCharacter
{
	GENERATED_BODY()

public:
	AGC_EnemyCharacter();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly,Category="GC|AbilitySystem")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	 
};
