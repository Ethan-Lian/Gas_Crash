#include "AbilitySystem/GC_AbilitySystemComponent.h"
#include "GameplayTags/GCTags.h"


//when character get a new ability , check if this ability has special Tag,if true,auto activate this ability.

//On Server
//when ASC->giveAbility called ,this function will be triggered. this function is a part of GiveAbility,default is null.
void UGC_AbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);
	
	// self define function,check if Ability need to activate.
	HandleAutoActivatedAbilities(AbilitySpec);
}


//On Client
//when Server Replicate AbilityList to Client,Client Call this function to make sure client also can autoactivate abilities.
void UGC_AbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();
	
	//1.作用域锁 (RAII 模式)
	FScopedAbilityListLock ActivateScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		HandleAutoActivatedAbilities(AbilitySpec);
	}
}


void UGC_AbilitySystemComponent::HandleAutoActivatedAbilities(const FGameplayAbilitySpec& AbilitySpec)
{
	if (!IsValid(AbilitySpec.Ability)) return;
	// loop Ability's Tag
	for (const FGameplayTag& Tag : AbilitySpec.Ability->GetAssetTags())
	{
		//if Tag matched AutoActivateAbility Tag,it will activate this ability 
		if (Tag.MatchesTagExact(GCTags::GCAbilities::ActivateOnGiven))
		{
			TryActivateAbility(AbilitySpec.Handle);
			return;
		}
	}
}
