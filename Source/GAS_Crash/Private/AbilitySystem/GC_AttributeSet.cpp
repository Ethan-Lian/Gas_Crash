#include "AbilitySystem/GC_AttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "GameplayTags/GCTags.h"
#include "Net/UnrealNetwork.h"
#include "Math/UnrealMathUtility.h"

void UGC_AttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue) const
{
	//时机: 仅在客户端触发,当服务器复制到客户端的值,也就是下面参数Health,与本地预测的值OldValue不一样时触发
	//作用: 将新值注入ASC,对齐基准值,处理预测(回滚),触发所有订阅了Health变化的委托.
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass,Health,OldValue);
}

void UGC_AttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass,MaxHealth,OldValue);
}

void UGC_AttributeSet::OnRep_Mana(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass,Mana,OldValue);
}

void UGC_AttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass,MaxMana,OldValue);
}



void UGC_AttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass,Health,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass,MaxHealth,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass,Mana,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass,MaxMana,COND_None,REPNOTIFY_Always);
	
	DOREPLIFETIME(ThisClass,bAttributeInitialized);
}

void UGC_AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	//only care health attribute change
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		//HealthDelta is negative when damage health, positive when add health or initialize health
		const float HealthDelta  =Data.EvaluatedData.Magnitude;
		
		AActor* TargetActor = Data.Target.GetAvatarActor();
		AActor* InstigatorActor = Cast<AActor>(Data.EffectSpec.GetEffectContext().GetInstigator());
		
		//Confirm the damage type
		if (HealthDelta < 0.f && IsValid(TargetActor) && IsValid(InstigatorActor))
		{
			FGC_DamageFeedbackData Payload;
			Payload.DamageMagnitude = FMath::Abs(HealthDelta);
			Payload.bIsFatal = GetHealth() <= 0.f;
			Payload.Instigator = InstigatorActor;
			Payload.EffectCauser = Data.EffectSpec.GetEffectContext().GetEffectCauser();
			Payload.EffectContext = Data.EffectSpec.GetEffectContext();
			
			const float MeleeSetByCaller = Data.EffectSpec.GetSetByCallerMagnitude(GCTags::SetByCaller::Melee);
			const float ProjectileSetByCaller = Data.EffectSpec.GetSetByCallerMagnitude(GCTags::SetByCaller::Projectile);
			
			if (!FMath::IsNearlyZero(MeleeSetByCaller))
			{
				Payload.DamageTypeTag = GCTags::SetByCaller::Melee;
			}
			else if (!FMath::IsNearlyZero(ProjectileSetByCaller))
			{
				Payload.DamageTypeTag = GCTags::SetByCaller::Projectile;
			}
			
			OnDamageConfirmed.Broadcast(Payload);
		}
	}
	
	// When the GE initializes the attribute for the first time, notify the UI or WidgetComponent to start binding real values.
	if (!bAttributeInitialized)
	{
		bAttributeInitialized = true;
		OnAttributeSetInitialized.Broadcast();
	}
}


void UGC_AttributeSet::OnRep_AttributeInitialized()
{
	// When the server initializes the attribute for the first time, 
	// notify the client subscribers to start binding real values.
	if (bAttributeInitialized)
	{
		OnAttributeSetInitialized.Broadcast();
	}
}
