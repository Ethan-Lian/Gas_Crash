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

