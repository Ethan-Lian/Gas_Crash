#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GC_DamageExecution.generated.h"

/**
 * UGC_DamageExecution
 *
 * ⭐⭐⭐ Interview Point:
 * ExecutionCalculation is the "final arbitrator" of damage in a GAS project.
 * It runs server-side only, captures Attributes from both Source and Target,
 * reads the CustomEffectContext for metadata, and outputs to Meta Attributes.
 *
 * Why not just modify Health directly in the Ability?
 * Because then every ability reinvents damage rules. ExecCalc centralizes:
 *   - Armor / resistance formulas
 *   - Critical hit multipliers
 *   - Future passive modifiers (e.g. equipment bonuses)
 * without touching any existing Ability code.
 *
 * Assigned to a GE via: GE → Executions → GC_DamageExecution
 */
UCLASS()
class GAS_CRASH_API UGC_DamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UGC_DamageExecution();

protected:
	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
