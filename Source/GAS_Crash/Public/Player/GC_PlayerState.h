#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GC_PlayerState.generated.h"

class UAbilitySystemComponent;

UCLASS()
class GAS_CRASH_API AGC_PlayerState : public APlayerState,public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AGC_PlayerState();
	
	//achieve ASInterface , its function is to return ASC
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
private:
	UPROPERTY(VisibleAnywhere,Category="GC|Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};
