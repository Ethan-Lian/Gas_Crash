#include "GAS_Crash/Public/Character/MyBaseCharacter.h"

#include "AbilitySystemComponent.h"


AMyBaseCharacter::AMyBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	//?
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	
}

UAbilitySystemComponent* AMyBaseCharacter::GetAbilitySystemComponent() const
{
	return nullptr;
}

void AMyBaseCharacter::GiveStartupAbilities()
{
	if (!IsValid(GetAbilitySystemComponent())) return;
	for (const auto& Ability : StartupGameplayAbilities)
	{
		//?为啥要创建, FGameplayAbilitySpec? 这里的auto引用没看明白为什么
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability);
		GetAbilitySystemComponent()->GiveAbility(AbilitySpec);
	}
}

