#include "GAS_Crash/Public/Character/MyBaseCharacter.h"
#include "AbilitySystemComponent.h"
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

void AMyBaseCharacter::GiveStartupAbilities()
{
	if (!IsValid(GetAbilitySystemComponent())) return;
	for (const auto& Ability : StartupGameplayAbilities)
	{
		if (!IsValid(Ability)){continue;}
		
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability);
		GetAbilitySystemComponent()->GiveAbility(AbilitySpec);
	}
}

void AMyBaseCharacter::InitializeAttribute() const
{
	//check
	if (!IsValid(GetAbilitySystemComponent()))
	{
		UE_LOG(LogTemp, Error, TEXT("ASC is invalid in InitializeAttribute"));
		return;
	}
	
	if (!IsValid(InitializeAttributesEffects))
	{
		UE_LOG(LogTemp,Error,TEXT("InitializeAttributesEffects not set for %s"),*GetName());
		return;
	}	
	
	//Create EffectContextHandle 
	FGameplayEffectContextHandle EffectContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this); //mark,this is created from self
	
	//Create EffectSpecHandle
	FGameplayEffectSpecHandle EffectSpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(
		InitializeAttributesEffects,
		1.f,
		EffectContextHandle);
	
	
	if (!EffectSpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create EffectSpec for %s"), *GetName()); 
		return;
	}
	
	//Apply Effect to Character's Attribute
	FActiveGameplayEffectHandle ActiveGameplayEffectHandle = GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
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

