#include "AI/TaskNode/GC_BTTask_FaceTarget.h"
#include "AIController.h"
#include "AI/GC_AITypeDefs.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/GC_EnemyCharacter.h"

UGC_BTTask_FaceTarget::UGC_BTTask_FaceTarget()
{
	NodeName = "Face Target";
	bNotifyTick = true;
	TargetActorKey.SelectedKeyName = BBKeys::TargetActor;
}

EBTNodeResult::Type UGC_BTTask_FaceTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	AGC_EnemyCharacter* EnemyCharacter = Cast<AGC_EnemyCharacter>(AIController->GetPawn());
	if (!EnemyCharacter || !EnemyCharacter->IsAlive()) return EBTNodeResult::Failed;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return EBTNodeResult::Failed;

	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor) return EBTNodeResult::Failed;

	return RotateTowardsTarget(EnemyCharacter, TargetActor, 0.0f) ? EBTNodeResult::Succeeded : EBTNodeResult::InProgress;
}

void UGC_BTTask_FaceTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AGC_EnemyCharacter* EnemyCharacter = Cast<AGC_EnemyCharacter>(AIController->GetPawn());
	if (!EnemyCharacter || !EnemyCharacter->IsAlive())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (RotateTowardsTarget(EnemyCharacter, TargetActor, DeltaSeconds))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

bool UGC_BTTask_FaceTarget::RotateTowardsTarget(AGC_EnemyCharacter* EnemyCharacter, const AActor* TargetActor, float DeltaSeconds) const
{
	if (!EnemyCharacter || !TargetActor) return false;

	FVector ToTarget = TargetActor->GetActorLocation() - EnemyCharacter->GetActorLocation();
	ToTarget.Z = 0.0f;

	if (ToTarget.IsNearlyZero())
	{
		return true;
	}

	const float DesiredYaw = ToTarget.Rotation().Yaw;
	const float CurrentYaw = EnemyCharacter->GetActorRotation().Yaw;
	const float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentYaw, DesiredYaw);

	if (FMath::Abs(DeltaYaw) <= AcceptableAngleDegrees)
	{
		return true;
	}

	if (DeltaSeconds <= 0.0f)
	{
		return false;
	}

	const float NewYaw = FMath::FixedTurn(CurrentYaw, DesiredYaw, RotationSpeedDegPerSec * DeltaSeconds);
	EnemyCharacter->SetActorRotation(FRotator(0.0f, NewYaw, 0.0f));

	const float RemainingDeltaYaw = FMath::FindDeltaAngleDegrees(EnemyCharacter->GetActorRotation().Yaw, DesiredYaw);
	return FMath::Abs(RemainingDeltaYaw) <= AcceptableAngleDegrees;
}