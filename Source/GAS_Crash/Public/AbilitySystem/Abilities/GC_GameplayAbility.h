#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GC_GameplayAbility.generated.h"

 
UCLASS()
class GAS_CRASH_API UGC_GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="GC|Debug")
	bool bDrawDebugs = false;
};
