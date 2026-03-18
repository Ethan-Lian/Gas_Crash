#include "AbilitySystem/Abilities/GC_GameplayAbility.h"
#include "Character/GC_EnemyCharacter.h"
#include "Character/MyBaseCharacter.h"

void UGC_GameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// CommitAbility() = CommitCheck() + CommitExecute()
	// CommitCheck() calls CheckCooldown() (ASC has cooldown tag?) + CheckCost() (CanApplyAttributeModifiers?).
	// CommitExecute() calls ApplyCooldown() (apply Duration GE that grants cooldown tag) + ApplyCost() (apply Instant GE that subtracts Mana).
	// If neither CostGE nor CooldownGE is configured on this ability, CommitAbility is a no-op and returns true.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (bDrawDebugs && IsValid(GEngine))
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, FString::Printf(TEXT("%s Activated"), *GetName()));
	}
}

AMyBaseCharacter* UGC_GameplayAbility::GetBaseCharacter() const
{
	return Cast<AMyBaseCharacter>(GetAvatarActorFromActorInfo());
}

AGC_EnemyCharacter* UGC_GameplayAbility::GetGC_EnemyCharacter() const
{
	return Cast<AGC_EnemyCharacter>(GetAvatarActorFromActorInfo());
}
