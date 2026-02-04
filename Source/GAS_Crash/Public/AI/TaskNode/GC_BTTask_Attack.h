#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GC_BTTask_Attack.generated.h"


UCLASS()
class GAS_CRASH_API UGC_BTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UGC_BTTask_Attack();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
protected:
	UPROPERTY(EditAnywhere,Category="GC|Blackboard")
	FBlackboardKeySelector TargetActorKey;
};
