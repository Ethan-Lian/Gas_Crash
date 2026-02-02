#include "AI/GC_EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Character/GC_EnemyCharacter.h"

AGC_EnemyAIController::AGC_EnemyAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGC_EnemyAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AGC_EnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// make sure we are possessing an Enemy
	if (!Cast<AGC_EnemyCharacter>(InPawn)) return;
	
	// RunBehaviorTree automatically handles:
	// 1. Creating BehaviorTreeComponent
	// 2. Initializing BlackBoard
	// 3. Starting the Behavior Tree
	if (BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);
	}
}
