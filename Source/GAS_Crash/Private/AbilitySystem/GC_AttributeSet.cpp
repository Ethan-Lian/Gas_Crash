#include "AbilitySystem/GC_AttributeSet.h"
#include "AbilitySystem/GC_GameplayEffectContext.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "GameplayTags/GCTags.h"
#include "Net/UnrealNetwork.h"
#include "Math/UnrealMathUtility.h"

void UGC_AttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue) const
{
	// timerline: only in client, when server replicate new Health value to client,
	// if the new value is different from old value, this function will be triggered, then it will inject new value to ASC, 
	// align with base value, handle prediction(rollback), trigger all delegate that subscribe Health change.
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

void UGC_AttributeSet::OnRep_Armor(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass,Armor,OldValue);
}



void UGC_AttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass,Health,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass,MaxHealth,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass,Mana,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass,MaxMana,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass,Armor,COND_None,REPNOTIFY_Always);

	DOREPLIFETIME(ThisClass,bAttributeInitialized);
}

void UGC_AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
	else if (Attribute == GetArmorAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

void UGC_AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// =========================================================
	// PIPELINE: ExecCalc → IncomingDamage → Health
	// PostGameplayEffectExecute fires AFTER the GE has modified the attribute value.
	// For Meta Attributes (IncomingDamage), the value is already set on the AttributeSet
	// by the time we reach here. We read it, apply it to real attributes, then zero it.
	// The separation of concerns:
	//   ExecCalc   = "how much damage after all modifiers?"
	//   This block = "what happens to the character as a result?"
	// =========================================================
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float DamageToApply = GetIncomingDamage();
		SetIncomingDamage(0.f); //Consume and clear immediately — Meta Attributes must not persist

		if (DamageToApply > 0.f)
		{
			const float NewHealth = FMath::Clamp(GetHealth() - DamageToApply, 0.f, GetMaxHealth());
			SetHealth(NewHealth);

			AActor* TargetActor    = Data.Target.GetAvatarActor();
			AActor* InstigatorActor = Cast<AActor>(Data.EffectSpec.GetEffectContext().GetInstigator());

			if (IsValid(TargetActor) && IsValid(InstigatorActor))
			{

				// We extract the Custom GE Context to read the DamageTypeTag.
				// This replaces the old SetByCaller-sniffing hack: instead of inferring
				// damage type by checking which SetByCaller tag has a non-zero value,
				// the Ability explicitly sets the type on the Context before applying the GE.
				const FGC_GameplayEffectContext* GCContext =
					FGC_GameplayEffectContext::ExtractEffectContext(Data.EffectSpec.GetContext());

				FGC_DamageFeedbackData Payload;
				Payload.DamageMagnitude = DamageToApply;
				Payload.bIsFatal        = NewHealth <= 0.f;
				Payload.Instigator      = InstigatorActor;
				Payload.EffectCauser    = Data.EffectSpec.GetEffectContext().GetEffectCauser();
				Payload.EffectContext   = Data.EffectSpec.GetEffectContext();
				Payload.DamageTypeTag   = GCContext ? GCContext->GetDamageTypeTag() : FGameplayTag::EmptyTag;

				OnDamageConfirmed.Broadcast(Payload);
			}
		}
	}

	// =========================================================
	// NEW PIPELINE: IncomingHealing → Health
	// =========================================================
	if (Data.EvaluatedData.Attribute == GetIncomingHealingAttribute())
	{
		const float HealingToApply = GetIncomingHealing();
		SetIncomingHealing(0.f);

		if (HealingToApply > 0.f)
		{
			SetHealth(FMath::Clamp(GetHealth() + HealingToApply, 0.f, GetMaxHealth()));
		}
	}

	// When the GE initializes the attribute for the first time, notify UI to start binding real values.
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
