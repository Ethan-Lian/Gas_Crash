#include "AbilitySystem/GC_AbilitySystemComponent.h"
#include "GameplayTags/GCTags.h"
#include "AbilitySystem/Abilities/GC_GameplayAbility.h"

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

	// 作用域锁 (RAII 模式)
	FScopedAbilityListLock ActivateScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// Only attempt auto-activation for specs we haven't seen before.
		// Without this guard, every replication of ActivatableAbilities (triggered by ANY new grant)
		// would redundantly call TryActivateAbility() on all existing specs.
		if (AutoActivatedSpecHandles.Contains(AbilitySpec.Handle))
		{
			continue;
		}
		// Only mark as handled if activation succeeded (or ability has no ActivateOnGiven tag).
		// If TryActivateAbility fails (e.g. InitAbilityActorInfo not yet called on client),
		// the next OnRep will retry.
		if (HandleAutoActivatedAbilities(AbilitySpec))
		{
			AutoActivatedSpecHandles.Add(AbilitySpec.Handle);
		}
	}
}

bool UGC_AbilitySystemComponent::HandleAutoActivatedAbilities(const FGameplayAbilitySpec& AbilitySpec)
{
	if (!IsValid(AbilitySpec.Ability)) return false;
	// loop Ability's Tag
	for (const FGameplayTag& Tag : AbilitySpec.Ability->GetAssetTags())
	{
		//if Tag matched AutoActivateAbility Tag,it will activate this ability
		if (Tag.MatchesTagExact(GCTags::GCAbilities::ActivateOnGiven))
		{
			return TryActivateAbility(AbilitySpec.Handle);
		}
	}
	// No ActivateOnGiven tag found — not an auto-activate ability, consider it handled.
	return true;
}

void UGC_AbilitySystemComponent::SetAbilityLevel(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level)
{
	//Set Level (about data) must in Server
	if (IsValid(GetAvatarActor()) && !GetAvatarActor()->HasAuthority()) return;
	
	//Get the AbilitySpec
	FGameplayAbilitySpec* Spec = FindAbilitySpecFromClass(AbilityClass);
	
	//Set the Ability's Level
	if (Spec)
	{
		Spec->Level = Level;
		MarkAbilitySpecDirty(*Spec);
	}
}

void UGC_AbilitySystemComponent::IncreaseAbilityLevel(TSubclassOf<UGameplayAbility> AbilityClass, int32 IncreaseLevel)
{
	if (IsValid(GetAvatarActor()) && !GetAvatarActor()->HasAuthority()) return;
	
	if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromClass(AbilityClass))
	{
		Spec->Level +=IncreaseLevel;
		MarkAbilitySpecDirty(*Spec);
	}
}

void UGC_AbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);
	
	if (Spec.IsActive())
	{
		// Input replication needs the activation prediction key of the running ability instance.
		// For this project's instanced abilities, the primary instance is the authoritative source.
		const UGameplayAbility* AbilityInstance = Spec.GetPrimaryInstance();
		const FPredictionKey PredictionKey = AbilityInstance
			? AbilityInstance->GetCurrentActivationInfo().GetActivationPredictionKey()
			: FPredictionKey();

		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, PredictionKey);
	}
}

void UGC_AbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);
	
	if (Spec.IsActive())
	{
		// Input replication needs the activation prediction key of the running ability instance.
		// For this project's instanced abilities, the primary instance is the authoritative source.
		const UGameplayAbility* AbilityInstance = Spec.GetPrimaryInstance();
		const FPredictionKey PredictionKey = AbilityInstance
			? AbilityInstance->GetCurrentActivationInfo().GetActivationPredictionKey()
			: FPredictionKey();

		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, PredictionKey);
	}
}

void UGC_AbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	FScopedAbilityListLock AbilityListLock(*this);

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
		}
	}
}

void UGC_AbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	FScopedAbilityListLock AbilityListLock(*this);

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandles.Remove(AbilitySpec.Handle);
		}
	}
}

void UGC_AbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	(void)DeltaTime;

	if (bGamePaused)
	{
		ClearAbilityInput();
		return;
	}

	TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;

	// 1. Handle held input for abilities that want continuous activation attempts.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (!AbilitySpec || !AbilitySpec->Ability || AbilitySpec->IsActive())
		{
			continue;
		}

		const UGC_GameplayAbility* GCAbilityCDO = Cast<UGC_GameplayAbility>(AbilitySpec->Ability);
		if (GCAbilityCDO && GCAbilityCDO->GetActivationPolicy() == EGC_AbilityActivationPolicy::WhileInputActive)
		{
			AbilitiesToActivate.AddUnique(SpecHandle);
		}
	}

	// 2. Handle input pressed this frame.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (!AbilitySpec || !AbilitySpec->Ability)
		{
			continue;
		}

		AbilitySpec->InputPressed = true;

		if (AbilitySpec->IsActive())
		{
			AbilitySpecInputPressed(*AbilitySpec);
			continue;
		}

		const UGC_GameplayAbility* GCAbilityCDO = Cast<UGC_GameplayAbility>(AbilitySpec->Ability);
		if (GCAbilityCDO && GCAbilityCDO->GetActivationPolicy() == EGC_AbilityActivationPolicy::OnInputTriggered)
		{
			AbilitiesToActivate.AddUnique(SpecHandle);
		}
	}

	// 3. Try activate after we finish collecting handles.
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	// 4. Handle input released this frame.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (!AbilitySpec || !AbilitySpec->Ability)
		{
			continue;
		}

		AbilitySpec->InputPressed = false;

		if (AbilitySpec->IsActive())
		{
			AbilitySpecInputReleased(*AbilitySpec);
		}
	}

	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UGC_AbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

void UGC_AbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	AutoActivatedSpecHandles.Remove(AbilitySpec.Handle);
	Super::OnRemoveAbility(AbilitySpec);
}
