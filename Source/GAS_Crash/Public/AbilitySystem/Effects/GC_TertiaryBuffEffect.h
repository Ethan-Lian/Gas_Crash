#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GC_TertiaryBuffEffect.generated.h"

UCLASS()
class GAS_CRASH_API UGC_TertiaryBuffEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	// Configure the temporary super-armor buff that grants Armor and a state tag.
	UGC_TertiaryBuffEffect(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
