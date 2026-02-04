#pragma once

#include "CoreMinimal.h"
#include "GC_AITypeDefs.generated.h"

/**
 * AI行为状态枚举
 * 
 * 这个枚举仅用于可视化调试，不应该驱动BehaviorTree决策
 * BehaviorTree应通过Selector优先级和Decorator条件自然决定行为
 */
UENUM(BlueprintType)
enum class EAIState : uint8
{
	Idle        UMETA(DisplayName = "待机"),
	Patrolling  UMETA(DisplayName = "巡逻"),
	Chasing     UMETA(DisplayName = "追击"),
	Attacking   UMETA(DisplayName = "攻击"),
	Fleeing     UMETA(DisplayName = "逃跑"),
	Stunned     UMETA(DisplayName = "晕眩")
};

/**
 * Blackboard键名常量
 * 
 * 工程实践:使用命名空间避免魔法字符串
 * 优势:编译期检查、重构安全
 */
namespace BBKeys
{
	// ==================== 目标相关 ====================
	const FName TargetActor = TEXT("TargetActor");
	const FName LastKnownTargetLocation = TEXT("LastKnownTargetLocation");
	
	// ==================== 距离和位置 ====================
	const FName DistanceToTarget = TEXT("DistanceToTarget");
	const FName PatrolLocation = TEXT("PatrolLocation");
	const FName FleeDestination = TEXT("FleeDestination");
	
	// ==================== 战斗状态 ====================
	const FName bCanAttack = TEXT("bCanAttack");
	const FName bPrimaryAbilityReady = TEXT("bPrimaryAbilityReady");
	const FName bHasSpecialAbilityReady = TEXT("bHasSpecialAbilityReady");
	const FName SelfHealthPercent = TEXT("SelfHealthPercent");
	
	// ==================== AI配置参数 ====================
	const FName AttackRange = TEXT("AttackRange");
	const FName DetectionRadius = TEXT("DetectionRadius");
	const FName ChaseDistance = TEXT("ChaseDistance");
	const FName RetreatHealthThreshold = TEXT("RetreatHealthThreshold");
	const FName Aggression = TEXT("Aggression");
	
	// ==================== 调试信息 ====================
	const FName CurrentAIState = TEXT("CurrentAIState"); // 仅用于调试可视化
}
