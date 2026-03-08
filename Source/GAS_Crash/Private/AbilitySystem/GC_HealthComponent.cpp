#include "AbilitySystem/GC_HealthComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GC_AttributeSet.h"
#include "Character/MyBaseCharacter.h"
#include "GameplayTags/GCTags.h"

UGC_HealthComponent::UGC_HealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SetIsReplicatedByDefault(false);
}

void UGC_HealthComponent::InitializeWithAbilitySystem(UAbilitySystemComponent* InASC, UGC_AttributeSet* InAttributeSet)
{
	UninitializeFromAbilitySystem();
	
	AbilitySystemComponent = InASC;
	
	AttributeSet = InAttributeSet;
	
	if (!AbilitySystemComponent || !AttributeSet) return;
	
	//Subscribe the Delegate Listen the Attribute change.
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(
		this,
		&UGC_HealthComponent::HandleHealthAttributeChanged);
	
	//Subscribe the delegate handle Damage
	AttributeSet->OnDamageConfirmed.AddUObject(this,&UGC_HealthComponent::HandleDamageConfirmed);
}

void UGC_HealthComponent::UninitializeFromAbilitySystem()
{
	if (AbilitySystemComponent && AttributeSet)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).RemoveAll(this);
		AttributeSet->OnDamageConfirmed.RemoveAll(this);
	}
	
	AbilitySystemComponent = nullptr;
	AttributeSet = nullptr;
}

void UGC_HealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeFromAbilitySystem();
	Super::EndPlay(EndPlayReason);
}

void UGC_HealthComponent::HandleHealthAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	if (ChangeData.NewValue <= 0.f)
	{
		if (AMyBaseCharacter* BaseCharacter = Cast<AMyBaseCharacter>(GetOwner()))
		{
			//HealthComponent only Listen attribute change,but handle death logic in character.
			BaseCharacter->HandleDeath();
		}
	}
}


void UGC_HealthComponent::HandleDamageConfirmed(const FGC_DamageFeedbackData& DamageData)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !AbilitySystemComponent || !OwnerActor->HasAuthority()) return;
	
	const AMyBaseCharacter* BaseCharacter = Cast<AMyBaseCharacter>(OwnerActor);
	if (!BaseCharacter || !BaseCharacter->IsAlive()) return;
	
	//if the damage is fatal, we only play death effect and death animation, no hit react, so return early without send hit react event.
	if (DamageData.bIsFatal) return;
	
	const FGameplayTag CueTag = ResolveCueTagFromDamageType(DamageData.DamageTypeTag);
	if (CueTag.IsValid())
	{
		FGameplayCueParameters CueParameters(DamageData.EffectContext);
		CueParameters.RawMagnitude = DamageData.DamageMagnitude;
		CueParameters.Instigator = DamageData.Instigator.Get();
		CueParameters.EffectCauser = DamageData.EffectCauser.Get();
		
		//Try to get hit result from EffectContext, if there is a hit result, use the impact point and impact normal for better visual effect, 
		//otherwise fallback to the effect causer's location and use the direction from effect causer to owner as normal.
		if (const FHitResult* HitResult = DamageData.EffectContext.GetHitResult())
		{
			CueParameters.Location = HitResult->ImpactPoint.IsNearlyZero()
			? HitResult->Location
			: HitResult->ImpactPoint;
			
			CueParameters.Normal = HitResult->ImpactNormal.IsNearlyZero()
			? HitResult->Normal
			: HitResult->ImpactNormal;
		}
		else if (DamageData.EffectCauser.IsValid())
		{
			CueParameters.Location = DamageData.EffectCauser->GetActorLocation();
			
			const FVector ToOwner =(OwnerActor->GetActorLocation() - DamageData.EffectCauser->GetActorLocation()).GetSafeNormal();
			CueParameters.Normal = -ToOwner;
		}
		
		AbilitySystemComponent->ExecuteGameplayCue(CueTag,CueParameters);
	}
	
	if (AbilitySystemComponent->HasMatchingGameplayTag(GCTags::GCIdentity::Player))
	{
		SendHitReactEvent(DamageData);
	}
}

FGameplayTag UGC_HealthComponent::ResolveCueTagFromDamageType(const FGameplayTag& DamageTypeTag) const
{
	if (DamageTypeTag == GCTags::SetByCaller::Melee)
	{
		return GCTags::GameplayCue::Character_DamageTaken_Melee;
	}
	
	if (DamageTypeTag == GCTags::SetByCaller::Projectile)
	{
		return GCTags::GameplayCue::Character_DamageTaken_Projectile;
	}
	
	return FGameplayTag();
}

void UGC_HealthComponent::SendHitReactEvent(const FGC_DamageFeedbackData& DamageData) const
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;
	
	FGameplayEventData Payload;
	Payload.Instigator = DamageData.Instigator.Get();
	Payload.Target = OwnerActor;
	Payload.EventTag = GCTags::GCEvents::player::HitReact;
	Payload.ContextHandle = DamageData.EffectContext;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		OwnerActor,
		GCTags::GCEvents::player::HitReact,
		Payload);
}

