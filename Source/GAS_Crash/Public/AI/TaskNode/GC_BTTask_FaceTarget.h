#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GC_BTTask_FaceTarget.generated.h"

UCLASS()
class GAS_CRASH_API UGC_BTTask_FaceTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UGC_BTTask_FaceTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category="GC|Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category="GC|Rotate", meta=(ClampMin="0.1"))
	float AcceptableAngleDegrees = 10.0f;

	UPROPERTY(EditAnywhere, Category="GC|Rotate", meta=(ClampMin="1.0"))
	float RotationSpeedDegPerSec = 540.0f;

private:
	bool RotateTowardsTarget(class AGC_EnemyCharacter* EnemyCharacter, const class AActor* TargetActor, float DeltaSeconds) const;
};
