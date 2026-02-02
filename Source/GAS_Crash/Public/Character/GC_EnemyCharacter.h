#pragma once
#include "CoreMinimal.h"
#include "MyBaseCharacter.h"
#include "GC_EnemyCharacter.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;
class UPawnSensingComponent;

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
	float MinAttackDelay = .1f;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="GC|AI")
	float MaxAttackDelay = .5f;

	//获取随机攻击延迟 [Min, Max]
	UFUNCTION(BlueprintCallable, Category="GC|AI", meta=(CompactNodeTitle="Random Delay"))
	float GetRandomAttackDelay() const;

protected:
	virtual void BeginPlay() override;

private:
	
	UPROPERTY(EditDefaultsOnly,Category="GC|AbilitySystem")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributesSet;

	//感知组件（用于检测玩家）
	UPROPERTY(VisibleAnywhere, Category="GC|AI")
	TObjectPtr<UPawnSensingComponent> PawnSensingComponent;
	
	//Callback Function when PawnSensing sees a pawn
	UFUNCTION()
	void OnSeePawn(APawn* SeenPawn);
};
