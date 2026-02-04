#include "AI/GC_BTService_AIDataCollector.h"

#include "AIController.h"
#include "AI/GC_AITypeDefs.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/GC_EnemyCharacter.h"

UGC_BTService_AIDataCollector::UGC_BTService_AIDataCollector()
{
	NodeName = "AI Data Collector";
	
	//default update span per 0.2s
	Interval = 0.2f;
	//random change update time
	RandomDeviation = 0.05f;
	
	//Set default BBKey name 
	TargetActorKey.SelectedKeyName = BBKeys::TargetActor;
	DistanceToTargetKey.SelectedKeyName = BBKeys::DistanceToTarget;
	bCanAttackKey.SelectedKeyName = BBKeys::bCanAttack;

}


void UGC_BTService_AIDataCollector::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return;
	
	AGC_EnemyCharacter* EnemyCharacter = Cast<AGC_EnemyCharacter>(Controller->GetPawn());
	if (!EnemyCharacter) return;
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;
	
	//Get TargetActor
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	

	//Update Distance Data
	if (bEnableDistanceCheck && Target)
	{
		UpdateDistanceData(BB,EnemyCharacter,Target);
	}
	
}

void UGC_BTService_AIDataCollector::UpdateDistanceData(class UBlackboardComponent* BB,
	class AGC_EnemyCharacter* EnemyCharacter, AActor* Target)
{
	//Get Distance from Enemy to Target,Write Distance in BB
	const float Distance = EnemyCharacter->GetDistanceTo(Target);
	BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName,Distance);
	
	//Check if Enemy can attack
	BB->SetValueAsBool(bCanAttackKey.SelectedKeyName,Distance<=EnemyCharacter->AttackRadius);
}
