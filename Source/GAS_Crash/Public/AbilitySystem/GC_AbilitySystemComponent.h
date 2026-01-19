#pragma once
#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GC_AbilitySystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_CRASH_API UGC_AbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRep_ActivateAbilities() override;
	
private:
	void HandleAutoActivatedAbilities(const FGameplayAbilitySpec& AbilitySpec);
};
