#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GC_BluePrintLibrary.generated.h"

UENUM(BlueprintType)
enum class EHitDirection : uint8
{
	Front,
	Back,
	Left,
	Right
};

USTRUCT(BlueprintType)
struct FClosestActorWithTagTResult
{
	GENERATED_BODY()

	//为啥这个结构体都blueprintType了,还需要给这2个变量加UPROPERTY(),使用的时机到底是什么?
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> ClosestActor;
	
	UPROPERTY(BlueprintReadWrite)
	float ClosestDistance = TNumericLimits<float>::Max();
};

UCLASS()
class GAS_CRASH_API UGC_BluePrintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure)
	static EHitDirection GetHitDirection(const FVector& TargetForward,const FVector& ToInstigator);
	
	UFUNCTION(BlueprintPure)
	static FName GetHitReactionName(const EHitDirection& HitDirection);
	
	UFUNCTION(BlueprintCallable)
	static FClosestActorWithTagTResult FindClosestActorWithTag(const UObject* WorldContextObject,const FVector& Origin,const FName& Tag = FName("Player"));
};
