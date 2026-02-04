#include "AI/TaskNode/GC_BTTask_Attack.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "AI/GC_AITypeDefs.h"
#include "Character/GC_EnemyCharacter.h"
#include "GameplayTags/GCTags.h"

UGC_BTTask_Attack::UGC_BTTask_Attack()
{
	NodeName = "EnemyAttackNode";
	TargetActorKey.SelectedKeyName = BBKeys::TargetActor;
}

EBTNodeResult::Type UGC_BTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//Get necessary Components
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;
	
	AGC_EnemyCharacter* EnemyCharacter = Cast<AGC_EnemyCharacter>(AIController->GetPawn());
	if (!EnemyCharacter) return EBTNodeResult::Failed;
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;
	
	//Activate Attack Ability
	bool bSuccess = EnemyCharacter->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(FGameplayTagContainer(GCTags::GCAbilities::Enemy::Attack));
	
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("[BTTask_Attack] Ability activated successfully"));
		return EBTNodeResult::Succeeded;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[BTTask_Attack] Failed to activate ability"));
	return EBTNodeResult::Failed;
	
}
