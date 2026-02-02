#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "GC_EnemyAIController.generated.h"

class UBehaviorTree;

UCLASS()
class GAS_CRASH_API AGC_EnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AGC_EnemyAIController();

	//Behavior Tree Asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GC|AI")
	UBehaviorTree* BehaviorTree;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
};
