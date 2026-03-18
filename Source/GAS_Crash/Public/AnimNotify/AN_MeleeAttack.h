#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "AN_MeleeAttack.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
class UAnimSequenceBase;
class USkeletalMeshComponent;
class UWorld;
class AActor;
struct FAnimNotifyEventReference;
struct FHitResult;

UCLASS()
class GAS_CRASH_API UAN_MeleeAttack : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAN_MeleeAttack();
	
	//Start AnimNotifyState Attack Window
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	
	//AnimNotifyState window tick, called every frame during the window, used to perform trace and apply damage.	
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	
	//End AnimNotifyState Attack Window
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	//Get the name of the notify for display in the editor
	virtual FString GetNotifyName_Implementation() const override;
private:
	struct FMeleeRuntimeData
	{
		TSet<TWeakObjectPtr<AActor>> HitActorsInCurrentWindow;
	};

	//Runtime state cache: one runtime data per MeshComp
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FMeleeRuntimeData> RuntimeDataByMesh;

	//===============Melee trace parameters======================
	UPROPERTY(EditAnywhere, Category = "GC|Trace")
	FName TraceStartSocketName = FName("Weapon_End");

	UPROPERTY(EditAnywhere, Category = "GC|Trace")
	FName TraceEndSocketName = FName("Weapon_Front");

	UPROPERTY(EditAnywhere, Category = "GC|Trace")
	float TraceRadius = 12.f;

	UPROPERTY(EditAnywhere, Category = "GC|Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECollisionChannel::ECC_Pawn;

	//==============Damage Parameters======================
	UPROPERTY(EditAnywhere, Category = "GC|Damage")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere, Category = "GC|Damage")
	float DamageMagnitude = -20.f;

	//==============Debug parameter======================
	UPROPERTY(EditAnywhere, Category = "GC|Debug")
	bool bDrawDebug = false;

	UPROPERTY(EditAnywhere, Category = "GC|Debug", meta = (ClampMin = 0.f))
	float DebugDrawTime = 1.f;

	//================Private helper functions==================== 
	//Get OwnerActor and SourceASC from MeshComp
	bool ResolveAttackContext(USkeletalMeshComponent* MeshComp,AActor*& OutOwnerActor,UAbilitySystemComponent*& OutASC) const;

	//Check if CandidateActor is a valid damage target for OwnerActor.
	bool IsEligibleDamageTarget(AActor* OwnerActor,AActor* CandidateActor) const;
	
	//Perform the trace and apply damage to candidate targets.
	void ProcessTraceAndApplyDamage(USkeletalMeshComponent* MeshComp, AActor* OwnerActor, UAbilitySystemComponent* SourceASC, FMeleeRuntimeData& RuntimeData);
	
	//Perform a single sweep segment and collect hit results into OutHitResults.
	void SweepSegment(USkeletalMeshComponent* MeshComp, AActor* OwnerActor, const FVector& Start, const FVector& End, TArray<FHitResult>& OutHitResults) const;
	
	//Apply damage effect to TargetActor
	void ApplyDamageToTarget(AActor* SourceActor, UAbilitySystemComponent* SourceASC, AActor* TargetActor,const FHitResult& HitResult) const;
	
	FVector GetSocketLocationSafe(const USkeletalMeshComponent* MeshComp, const FName& SocketName, const FVector& FallbackLocation) const;
	
	void DrawDebugSweep(const UWorld* World, const FVector& Start, const FVector& End, const FColor& Color) const;
};
