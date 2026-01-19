#include "AbilitySystem/Abilities/Player/GC_Primary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/OverlapResult.h"
#include "GameplayTags/GCTags.h"

TArray<AActor*> UGC_Primary::HitBoxOverlapTest()
{
	 //1.define ignore item,add actor that is executing this ability
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
	
	//2.config QueryParams
	//FCollisionQueryParams: 这是一个通用的结构体，用来存放“这次检测要忽略谁”、“是否开启复杂碰撞（多边形检测）”等设置
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(ActorsToIgnore);
	
	//3.config ResponseParams
	//这是一个“过滤器,把除了pawn之外的所有类都忽略掉
	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECR_Ignore);
	ResponseParams.CollisionResponse.SetResponse(ECC_Pawn,ECR_Block);
	
	//4.cal location and make a collision shape
	FCollisionShape Shape = FCollisionShape::MakeSphere(HitBoxRadius);
	FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector()*HitBoxForwardOffset;
	FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation() + Forward + FVector(0,0,HitBoxElevationOffset);
	
	//5. apply check
	TArray<FOverlapResult> OverlapResults; 
	
	GetWorld()->OverlapMultiByChannel(
		OverlapResults,  //restore result in this array 
		Location,
		FQuat::Identity, //球的旋转 (球怎么转都一样，所以用 Identity 也就是不转)
		ECC_Visibility,  //Trace Channel
		Shape,
		QueryParams, //Ignore list
		ResponseParams // 过滤规则, only block pawn
		);
	
	//collect overlap actor
	TArray<AActor*> OverlapActors;
	for (const FOverlapResult& result : OverlapResults)
	{
		if (!IsValid(result.GetActor())) continue;
		OverlapActors.AddUnique(result.GetActor());
	}
	
	//.draw debug
	if (bDrawDebugs)
	{
		DrawHitBoxOverlap(Location,OverlapResults);
	}
	
	return OverlapActors;
}


void UGC_Primary::SendEventToEnemy(const TArray<AActor*>& OverlapActors)
{
	for (AActor* Actor : OverlapActors)
	{
		if (!IsValid(Actor))continue;
		
		FGameplayEventData payload;
		payload.Instigator = GetAvatarActorFromActorInfo();
		
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Actor,GCTags::GCEvents::Enemy::HitReact,payload);
	}
}

void UGC_Primary::DrawHitBoxOverlap(const FVector& Location, const TArray<FOverlapResult>& OverlapResults) const
{
	DrawDebugSphere(GetWorld(),Location,100.f,32,FColor::Blue,false,3.f);
		
	//loop result and draw 
	for (const FOverlapResult& Result : OverlapResults )
	{
		if (IsValid(Result.GetActor())){
			FVector location = Result.GetActor()->GetActorLocation();
			DrawDebugSphere(GetWorld(),location+FVector(0,0,100),100.f,32,FColor::Green,false,3.f);
		}
	}
}


