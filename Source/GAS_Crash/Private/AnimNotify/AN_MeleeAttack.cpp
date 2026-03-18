#include "AnimNotify/AN_MeleeAttack.h"
#include "AbilitySystem/GC_GameplayEffectContext.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/MyBaseCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameplayEffect.h"
#include "Character/GC_PlayerCharacter.h"
#include "GameplayTags/GCTags.h"

UAN_MeleeAttack::UAN_MeleeAttack()
{
	#if WITH_EDITORONLY_DATA
		NotifyColor = FColor::Red;
	#endif
}

FString UAN_MeleeAttack::GetNotifyName_Implementation() const
{
	return TEXT("GC_MeleeAttackNotify");
}

void UAN_MeleeAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (!IsValid(MeshComp)) return;
	
	// Get existing runtime data for this mesh, or create a default one if not found.
	FMeleeRuntimeData& RuntimeData = RuntimeDataByMesh.FindOrAdd(MeshComp);
	RuntimeData.HitActorsInCurrentWindow.Reset();
	
	if(!DamageEffect)
	{
		UE_LOG(LogTemp,Warning,TEXT("UAN_MeleeAttack::NotifyBegin - No DamageEffect set!"));
	}
}

void UAN_MeleeAttack::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if(!IsValid(MeshComp)) return;

	AActor* OwnerActor = nullptr;
	UAbilitySystemComponent* SourceASC = nullptr;
	
	//Get OwnerActor and SourceASC,if failed just return.
	if(!ResolveAttackContext(MeshComp,OwnerActor,SourceASC)) return;

	//Get existing runtime data for this mesh, or create a default one if not found.
	FMeleeRuntimeData& RuntimeData = RuntimeDataByMesh.FindOrAdd(MeshComp);

	//Process trace and apply damage.
	ProcessTraceAndApplyDamage(MeshComp,OwnerActor,SourceASC,RuntimeData);
}

void UAN_MeleeAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (!IsValid(MeshComp)) return;
	
	//Clear runtime data for this mesh to free memory,because the attack window ended.
	RuntimeDataByMesh.Remove(MeshComp);
}

bool UAN_MeleeAttack::ResolveAttackContext(USkeletalMeshComponent* MeshComp, AActor*& OutOwnerActor,
	UAbilitySystemComponent*& OutASC) const
{
	OutOwnerActor = nullptr;
	OutASC = nullptr;

	if(!IsValid(MeshComp)) return false;

	AActor* OwnerActor = MeshComp->GetOwner();
	if(!IsValid(OwnerActor)) return false;

	// only allow server to apply damage.
	if(!OwnerActor->HasAuthority()) return false;

	// Only allow alive character to apply damage, prevent applying damage after death.
	AMyBaseCharacter* CharacterOwner = Cast<AMyBaseCharacter>(OwnerActor);
	if(!IsValid(CharacterOwner) || !CharacterOwner->IsAlive()) return false;

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	if(!IsValid(SourceASC)) return false;

	OutOwnerActor = OwnerActor;
	OutASC = SourceASC;
	return true;
}

void UAN_MeleeAttack::ProcessTraceAndApplyDamage(USkeletalMeshComponent* MeshComp, AActor* OwnerActor,
	UAbilitySystemComponent* SourceASC, FMeleeRuntimeData& RuntimeData)
{
	if(!IsValid(MeshComp) || !IsValid(OwnerActor) || !DamageEffect || !IsValid(SourceASC) ) return;
	
	const FVector FallbackLocation = MeshComp->GetComponentLocation();
	const FVector CurrentSocketStart = GetSocketLocationSafe(MeshComp,TraceStartSocketName,FallbackLocation);
	const FVector CurrentSocketEnd = GetSocketLocationSafe(MeshComp,TraceEndSocketName,FallbackLocation);
	
	TArray<FHitResult> HitResultsThisTick;
	
	//Perform a single sweep segment and collect HitResults.
	SweepSegment(MeshComp,OwnerActor,CurrentSocketStart,CurrentSocketEnd,HitResultsThisTick);

	//Iterate over hit results, filter and apply damage.
	for (const FHitResult& HitResult : HitResultsThisTick)
	{
		AActor*	TargetActor = HitResult.GetActor();
		if (!IsEligibleDamageTarget(OwnerActor,TargetActor))
		{
			continue;
		}

		if(RuntimeData.HitActorsInCurrentWindow.Contains(TargetActor))
		{
			continue;
		}
		
		ApplyDamageToTarget(OwnerActor,SourceASC,TargetActor,HitResult);
		RuntimeData.HitActorsInCurrentWindow.Add(TargetActor);
	}
}

void UAN_MeleeAttack::SweepSegment(USkeletalMeshComponent* MeshComp, AActor* OwnerActor, const FVector& Start,
	const FVector& End, TArray<FHitResult>& OutHitResults) const
{
	if (!IsValid(OwnerActor) || !IsValid(MeshComp)) return;
	
	UWorld* World = MeshComp->GetWorld();
	if (!IsValid(World)) return;
	
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AN_MeleeAttack),false,OwnerActor);
	QueryParams.AddIgnoredActor(OwnerActor);
	
	const bool bHit = World->SweepMultiByChannel(
		OutHitResults,
		Start,
		End,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(TraceRadius),
		QueryParams);
	
	if (bDrawDebug)
	{
		DrawDebugSweep(World,Start,End,bHit ? FColor::Red : FColor::Green);
	}
}

bool UAN_MeleeAttack::IsEligibleDamageTarget(AActor* OwnerActor, AActor* CandidateActor) const
{
	if (!IsValid(OwnerActor) || !IsValid(CandidateActor)) return false;
	
	if (CandidateActor==OwnerActor) return false;
	
	// MeleeEnemy can only attack player and must be alive.
	const AGC_PlayerCharacter* PlayerCharacter = Cast<AGC_PlayerCharacter>(CandidateActor);
	if (!IsValid(PlayerCharacter) || !PlayerCharacter->IsAlive()) return false;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CandidateActor);
	if (!IsValid(TargetASC)) return false;

	//MeleeEnemy only can attack targets that the GAS identity system marks as players.
	return TargetASC->HasMatchingGameplayTag(GCTags::GCIdentity::Player);
}

void UAN_MeleeAttack::ApplyDamageToTarget(AActor* SourceActor, UAbilitySystemComponent* SourceASC,
	AActor* TargetActor ,const FHitResult& HitResult) const
{
	if (!IsValid(SourceActor) || !IsValid(SourceASC) || !IsValid(TargetActor) || !DamageEffect) return;
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;
	
	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddInstigator(SourceActor,SourceActor);
	ContextHandle.AddSourceObject(SourceActor);
	ContextHandle.AddHitResult(HitResult,true);

	// Semantic damage type drives downstream cue dispatch and hit react routing.
	if (FGC_GameplayEffectContext* GCContext = FGC_GameplayEffectContext::ExtractEffectContext(ContextHandle))
	{
		GCContext->SetDamageTypeTag(GCTags::DamageType::Melee);
	}
	
	FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect,1,ContextHandle);
	if (!EffectSpecHandle.IsValid()) return;

	// Unified damage pipeline: all damage magnitudes go through SetByCaller::Damage.
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
		EffectSpecHandle,
		GCTags::SetByCaller::Damage,
		DamageMagnitude);
	
	SourceASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(),TargetASC);
}

FVector UAN_MeleeAttack::GetSocketLocationSafe(const USkeletalMeshComponent* MeshComp, const FName& SocketName,
	const FVector& FallbackLocation) const
{
	if (!IsValid(MeshComp) || SocketName.IsNone() || !MeshComp->DoesSocketExist(SocketName)) return FallbackLocation;
	
	return MeshComp->GetSocketLocation(SocketName);
}

void UAN_MeleeAttack::DrawDebugSweep(const UWorld* World, const FVector& Start, const FVector& End,
	const FColor& Color) const
{
	if (!bDrawDebug || !IsValid(World)) return;
	
	DrawDebugLine(World,Start,End,Color,false,DebugDrawTime,0,1.f);
	DrawDebugSphere(World, Start, TraceRadius, 12, Color, false, DebugDrawTime);
	DrawDebugSphere(World, End, TraceRadius, 12, Color, false, DebugDrawTime);
}
