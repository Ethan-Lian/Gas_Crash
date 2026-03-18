#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GC_GameplayAbility.generated.h"

class AGC_EnemyCharacter;
class AMyBaseCharacter;

//Lyra-style minimal activation policy
//OnInputTriggered: activate once when button is pressed,WhileInputActive: keep trying to activate while key is held
UENUM(BlueprintType)
enum class EGC_AbilityActivationPolicy : uint8
{
	OnInputTriggered,
	WhileInputActive
};

UCLASS()
class GAS_CRASH_API UGC_GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="GC|Debug")
	bool bDrawDebugs = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GC|Input")
	EGC_AbilityActivationPolicy ActivationPolicy = EGC_AbilityActivationPolicy::OnInputTriggered;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//get BaseCharacter from AvatarActor
	UFUNCTION(BlueprintCallable, Category="GC|Ability")
	AMyBaseCharacter* GetBaseCharacter() const;

	//Get GC_EnemyCharacter from AvatarActor
	UFUNCTION(BlueprintCallable, Category="GC|Ability")
	AGC_EnemyCharacter* GetGC_EnemyCharacter() const;
	
	EGC_AbilityActivationPolicy GetActivationPolicy() const
	{
		return ActivationPolicy;
	}
};
