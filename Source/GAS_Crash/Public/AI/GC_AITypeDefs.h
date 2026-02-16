#pragma once
#include "CoreMinimal.h"

/*
 *Blackboard Keys constants
 * 
 *Best Practice: Use namespaces to avoid magic strings
 *Advantages: Compile-time checks, refactoring safety
 */ 

namespace BBKeys
{
	// ==================== TargetActor ====================
	const FName TargetActor = TEXT("TargetActor");
	
	// ==================== Combats and Status ====================
	const FName bCanAttack = TEXT("bCanAttack");
	const FName bPrimaryAbilityReady = TEXT("bPrimaryAbilityReady");
}
