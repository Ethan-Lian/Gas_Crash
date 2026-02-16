#include "AI/GC_EnemyAIController.h"
#include "AI/GC_AITypeDefs.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/GC_EnemyCharacter.h"
#include "Character/GC_PlayerCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

AGC_EnemyAIController::AGC_EnemyAIController()
{
	PrimaryActorTick.bCanEverTick = false;
	
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(FName("AIPerceptionComponent"));
	SetPerceptionComponent(*PerceptionComponent);
	
	//Configure Sight Sense
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(FName("SightConfig"));
	SightConfig->SightRadius = 1000.0f;
	SightConfig->LoseSightRadius = 1200.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	SightConfig->SetMaxAge(2.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	
	//Add and Configure Sense
	PerceptionComponent->ConfigureSense(*SightConfig);
	//Set Dominant Sense, usually for sight-based AI
	PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	//Bind Delegate to update Target
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this,&ThisClass::HandleTargetPerceptionUpdated);
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

void AGC_EnemyAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	//if percept actor is self or null return
	if (!Actor || Actor == GetPawn()) return;

	AGC_PlayerCharacter* PlayerCharacter = Cast<AGC_PlayerCharacter>(Actor);
	if (!PlayerCharacter) return;

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB) return;

	// If target is dead, clear current target immediately to stop chasing.
	if (!PlayerCharacter->IsAlive())
	{
		const UObject* CurrentTarget = BB->GetValueAsObject(BBKeys::TargetActor);
		if (CurrentTarget == Actor)
		{
			BB->ClearValue(BBKeys::TargetActor);
			BB->SetValueAsBool(BBKeys::bCanAttack, false);
		}
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		BB->SetValueAsObject(BBKeys::TargetActor, PlayerCharacter);
	}
	else
	{
		// Only clear if the lost target is the current target
		const UObject* CurrentTarget = BB->GetValueAsObject(BBKeys::TargetActor);
		if (CurrentTarget == Actor)
		{
			BB->ClearValue(BBKeys::TargetActor);
		}
	}
}

void AGC_EnemyAIController::RestartAfterRespawn()
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->ClearValue(BBKeys::TargetActor);
		BB->SetValueAsBool(BBKeys::bCanAttack,false);
	}

	// StopLogic() sets bIsRunning=false, bIsPaused=false.
	// RestartLogic() only works when tree is running or paused — after StopLogic it does nothing.
	// Must use RunBehaviorTree() to force a fresh start.
	if (BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);
	}

	// Force perception refresh so the enemy can re-detect nearby players immediately
	if (PerceptionComponent)
	{
		PerceptionComponent->ForgetAll();
		PerceptionComponent->RequestStimuliListenerUpdate();
	}
}
