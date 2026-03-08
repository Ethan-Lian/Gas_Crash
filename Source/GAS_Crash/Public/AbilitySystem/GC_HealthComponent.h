#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GC_HealthComponent.generated.h"

struct FOnAttributeChangeData;
class UGC_AttributeSet;
class UAbilitySystemComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_CRASH_API UGC_HealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGC_HealthComponent();
	
	void InitializeWithAbilitySystem(UAbilitySystemComponent* InASC,UGC_AttributeSet* InAttributeSet);
	
	// UnBind Function, if the component does not unbind, it is easy to leave dirty delegates when the character respawns, Pawn switches, or objects are destroyed. 
	void UninitializeFromAbilitySystem();
protected:
	// when Component Destroyed,make sure unbind delegate and reset ASC and AttributeSet to void crash.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	void HandleHealthAttributeChanged(const FOnAttributeChangeData& ChangeData);
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UGC_AttributeSet> AttributeSet;
};
