#include "AbilitySystem/GC_HealthComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/GC_AttributeSet.h"
#include "Character/MyBaseCharacter.h"

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
}

void UGC_HealthComponent::UninitializeFromAbilitySystem()
{
	if (AbilitySystemComponent && AttributeSet)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).RemoveAll(this);
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

