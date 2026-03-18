#include "GAS_Crash/Public/Character/MyBaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GC_AbilitySet.h"
#include "AbilitySystem/GC_AbilitySystemComponent.h"
#include "AbilitySystem/GC_AttributeSet.h"
#include "AbilitySystem/GC_HealthComponent.h"

AMyBaseCharacter::AMyBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	
	HealthComponent = CreateDefaultSubobject<UGC_HealthComponent>(FName("HealthComponent"));
}

UAbilitySystemComponent* AMyBaseCharacter::GetAbilitySystemComponent() const
{
	return nullptr;
}

void AMyBaseCharacter::GiveStartupAbilitySets()
{
	if (!HasAuthority() || bStartupAbilitySetsGranted) return;

	UGC_AbilitySystemComponent* GCASC = Cast<UGC_AbilitySystemComponent>(GetAbilitySystemComponent());
	if (!IsValid(GCASC)) return;

	for (UGC_AbilitySet* AbilitySet : StartupAbilitySets)
	{
		// Always add a placeholder to maintain 1:1 index correspondence with StartupAbilitySets.
		// This preserves the parallel array invariant for future per-set selective revocation.
		FGC_AbilitySet_GrantedHandles& NewHandles = GrantedAbilitySetHandlesArray.AddDefaulted_GetRef();

		if (!IsValid(AbilitySet)) continue;
		
		AbilitySet->GiveToAbilitySystem(GCASC, &NewHandles, this);
	}

	bStartupAbilitySetsGranted = true;
}

void AMyBaseCharacter::ClearStartupAbilitySets()
{
	if (!HasAuthority() || !bStartupAbilitySetsGranted) return;

	UGC_AbilitySystemComponent* GCASC = Cast<UGC_AbilitySystemComponent>(GetAbilitySystemComponent());
	if (!IsValid(GCASC)) return;

	for (FGC_AbilitySet_GrantedHandles& Handles : GrantedAbilitySetHandlesArray)
	{
		Handles.TakeFromAbilitySystem(GCASC);
	}
	GrantedAbilitySetHandlesArray.Reset();
	bStartupAbilitySetsGranted = false;
}

void AMyBaseCharacter::ResetAttributes()
{
	if (!HasAuthority()) return;
	if (!GetAbilitySystemComponent()) return;
	if (!IsValid(ResetAttributeEffects)) return;
	
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	FGameplayEffectSpecHandle EffectSpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(ResetAttributeEffects,1,ContextHandle);
	
	if (!EffectSpecHandle.IsValid()) return;
	
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
}

void AMyBaseCharacter::InitializeHealthComponent() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	UGC_AttributeSet* GCAttributeSet = Cast<UGC_AttributeSet>(GetAttributeSet());
	
	if (!HealthComponent || !ASC || !GCAttributeSet) return;
	
	HealthComponent->InitializeWithAbilitySystem(ASC,GCAttributeSet);
}

void AMyBaseCharacter::HandleDeath()
{
	//Only execute death logic on the server, and only execute once per death.
	if(!HasAuthority()) return;
	if(bDeathHandled) return; 

	bDeathHandled = true;
	bAlive = false;	
}

void AMyBaseCharacter::HandleRespawn()
{
	//Only execute respawn logic on the server.
	if (!HasAuthority()) return;

	//Reset death state
	bDeathHandled = false;
	bAlive = true;
	
	//reset attributes
	ResetAttributes();
}

void AMyBaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearStartupAbilitySets();
	Super::EndPlay(EndPlayReason);
}

