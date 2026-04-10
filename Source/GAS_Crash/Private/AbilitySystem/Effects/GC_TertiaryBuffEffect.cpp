#include "AbilitySystem/Effects/GC_TertiaryBuffEffect.h"

#include "AbilitySystem/GC_AttributeSet.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "GameplayTags/GCTags.h"

UGC_TertiaryBuffEffect::UGC_TertiaryBuffEffect(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(5.0f));

	FGameplayModifierInfo& ArmorModifier = Modifiers.AddDefaulted_GetRef();
	ArmorModifier.Attribute = UGC_AttributeSet::GetArmorAttribute();
	ArmorModifier.ModifierOp = EGameplayModOp::Additive;
	ArmorModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(50.0f));

	FInheritedTagContainer GrantedTargetTags;
	GrantedTargetTags.AddTag(GCTags::State::Buff::SuperArmor);

	UTargetTagsGameplayEffectComponent* TargetTagsComponent =
		ObjectInitializer.CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(this, TEXT("TargetTagsComponent"));
	GEComponents.Add(TargetTagsComponent);
	TargetTagsComponent->SetAndApplyTargetTagChanges(GrantedTargetTags);
}
