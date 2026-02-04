#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "GameplayTagContainer.h"
#include "GC_BTService_AIDataCollector.generated.h"



/**
 * AI数据采集服务（重构版）
 * 
 * 架构原则：
 * ❌ 不应该：做决策、改变AI状态、执行行为
 * ✅ 应该：查询数据、更新Blackboard、提供决策依据
 * 
 * 职责边界：
 * - 查询战斗数据（距离、血量、技能CD）
 * - 更新Blackboard键值
 * - 不参与状态决策（由BehaviorTree的Selector和Decorator负责）
 * 
 * 性能优化：
 * - 使用EnemyCharacter的缓存接口，避免遍历
 * - 可配置更新频率（Interval）
 * - 只更新必要的键值
 */

/*
 * AI Data Collect Service
 * 
 * 
 */

UCLASS()
class GAS_CRASH_API UGC_BTService_AIDataCollector : public UBTService
{
	GENERATED_BODY()
public:
	
	UGC_BTService_AIDataCollector();
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
protected:
	//===========================Black Board Key Set======================
	
	//TargetActor
	UPROPERTY(EditAnywhere,Category="GC|Blackboard")
	FBlackboardKeySelector TargetActorKey;
	
	//Distance to Target
	UPROPERTY(EditAnywhere,Category="GC|Blackboard")
	FBlackboardKeySelector DistanceToTargetKey;
	
	//Can Attack
	UPROPERTY(EditAnywhere, Category="GC|Blackboard")
	FBlackboardKeySelector bCanAttackKey;
	
	
	//===========================Config Options===========================
	
	//Enable Distance Check
	UPROPERTY(EditAnywhere,Category="GC|Config")
	bool bEnableDistanceCheck;
	
private:
	void UpdateDistanceData(class UBlackboardComponent* BB,class AGC_EnemyCharacter* EnemyCharacter,AActor* Target);
	
};
