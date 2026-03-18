#include "GameObjects/GC_Projectile.h"
#include "AbilitySystem/GC_GameplayEffectContext.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/GC_PlayerCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayTags/GCTags.h"

AGC_Projectile::AGC_Projectile()
{
	PrimaryActorTick.bCanEverTick = false;
	MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovementComponent"));
}

void AGC_Projectile::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	
	// check if the overlapped actor is player character and if the player is alive
	AGC_PlayerCharacter* Player = Cast<AGC_PlayerCharacter>(OtherActor);
	if (!IsValid(Player) || !Player->IsAlive()) return;
	if (!DamageEffect) return;
	
	// Get the target's  ASC and check if we have authority to apply the effect
	UAbilitySystemComponent* TargetASC = Player->GetAbilitySystemComponent();
	if (!IsValid(TargetASC) || !HasAuthority()) return;
	
	// Get the source actor and source ASC, make the damage effect spec and apply it to the target
	AActor* SourceActor = GetInstigator();
	if(!IsValid(SourceActor)) {SourceActor = GetOwner();};
	if(!IsValid(SourceActor)) return;

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor);
	if(!IsValid(SourceASC)) return;

	// Make the damage effect spec.
	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddInstigator(SourceActor, this);
	ContextHandle.AddSourceObject(this);

	if (FGC_GameplayEffectContext* GCContext = FGC_GameplayEffectContext::ExtractEffectContext(ContextHandle))
	{
		GCContext->SetDamageTypeTag(GCTags::DamageType::Projectile);
	}

	FGameplayEffectSpecHandle DamageEffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1.0f, ContextHandle);
	if (!DamageEffectSpecHandle.IsValid()) return;

	// Unified SetByCaller::Damage tag — ExecCalc reads this.
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageEffectSpecHandle, GCTags::SetByCaller::Damage, Damage);

	// Apply the damage effect spec to the target.
	SourceASC->ApplyGameplayEffectSpecToTarget(*DamageEffectSpecHandle.Data.Get(), TargetASC);
	
	//Spawn Boom ParticleImpactEffect
	SpawnImpactEffects();
	
	Destroy();
}
