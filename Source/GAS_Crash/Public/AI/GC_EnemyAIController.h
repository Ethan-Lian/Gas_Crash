#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "GC_EnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UBehaviorTree;

UCLASS()
class GAS_CRASH_API AGC_EnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AGC_EnemyAIController();

	//Behavior Tree Asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GC|AI")
	UBehaviorTree* BehaviorTree;

	UFUNCTION(BlueprintCallable,Category="GC|AI")
	void RestartAfterRespawn();
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	
private:
	//Sight Config, used to configure sight sense
	UPROPERTY(EditAnywhere, Category="GC|AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
	
	//Handle Target Perception Updated, called when the perception of a target is updated
	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
