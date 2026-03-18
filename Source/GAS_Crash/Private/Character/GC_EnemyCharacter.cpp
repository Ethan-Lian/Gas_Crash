#include "Character/GC_EnemyCharacter.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "AbilitySystem/GC_AbilitySystemComponent.h"
#include "AbilitySystem/GC_AttributeSet.h"
#include "AI/GC_AITypeDefs.h"
#include "AI/GC_EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTags/GCTags.h"
#include "TimerManager.h"

AGC_EnemyCharacter::AGC_EnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	//Create ASC and set Replication,make sure Server data can replicate to client
	AbilitySystemComponent = CreateDefaultSubobject<UGC_AbilitySystemComponent>(FName("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	//Create AttributeSet
	AttributesSet = CreateDefaultSubobject<UGC_AttributeSet>(FName("AttributeSet"));

}

void AGC_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(GetAbilitySystemComponent())) return;
	//Initialize ASC,OwnerActor(数据拥有者)->Enemy,AvatarActor(表现持有者)->Enemy
	GetAbilitySystemComponent()->InitAbilityActorInfo(this,this);
	
	//Initialize HealthComponent to listen health change.
	InitializeHealthComponent();
	
	OnAscInitialized.Broadcast(GetAbilitySystemComponent(),GetAttributeSet());
	
	//only in server, giving startup abilities and startup passive effects.
	if (HasAuthority())
	{
		//Set a loose tag count on the ASC to identify this as a enemy character, this is replicated to clients.
		GetAbilitySystemComponent()->SetLooseGameplayTagCount(
			GCTags::GCIdentity::Enemy,
			1,
			EGameplayTagReplicationState::TagAndCountToAll);

		// AbilitySet now owns startup passive effects and initialization GE.
		GiveStartupAbilitySets();
	}
	
	//Listen Dead Tag change by delegate
	GetAbilitySystemComponent()->RegisterGameplayTagEvent(GCTags::GCEvents::Enemy::Dead,EGameplayTagEventType::NewOrRemoved).AddUObject(this,&AGC_EnemyCharacter::OnDeadTagChanged);
}

void AGC_EnemyCharacter::HandleDeath()
{
	// Gate before Super so we can reuse base-state without relying on duplicated member.
	if (!HasAuthority() || bDeathHandled) return;
	Super::HandleDeath();

	GetWorldTimerManager().ClearTimer(KnockbackRecoveryTimerHandle);

	if (bKnockbackLandedBound)
	{
		LandedDelegate.RemoveDynamic(this, &AGC_EnemyCharacter::OnKnockbackLanded);
		bKnockbackLandedBound = false;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->SetLooseGameplayTagCount(
			GCTags::State::CC::Knockback,
			0,
			EGameplayTagReplicationState::TagAndCountToAll);
	}

	bKnockbackPausedBrainLogic = false;

	
	//When Enemy dead Broadcast to spawner.
	OnEnemyDied.Broadcast(this);
	
	//Give Dead Tag by Effect and Activate Death Gameplay Ability
	Handle_DeathAbilityAndDeathEffect();
}

UAbilitySystemComponent* AGC_EnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAttributeSet* AGC_EnemyCharacter::GetAttributeSet() const
{
	return AttributesSet;
}

void AGC_EnemyCharacter::Handle_DeathAbilityAndDeathEffect()
{
	if (!HasAuthority()) return;
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;	

	//Activate DeathAbility to play Montage.
	ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(GCTags::GCAbilities::Enemy::Death));

	//Apply DeathEffect (adds Dead tag, disables input)	
	if (GE_Death)
	{
		//Give Dead Tag to mark this character is dead by GE_Death.
		FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GE_Death,1.f,EffectContextHandle);
		if (SpecHandle.IsValid())
		{
			// Cache activated DeathEffectHandle it will be used to remove in HandleRespawn.
			ActivateDeathEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}	
	}
}

void AGC_EnemyCharacter::StopAILogicAndClearBlackboradValue()
{
	if(AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();

		if(UBrainComponent* BrainComponent = AIController->GetBrainComponent())
		{
			BrainComponent->StopLogic("Enemy Died");
		}
		
		//Clear all Blackboard keys when Enemy died.
		if(UBlackboardComponent* BB = AIController->GetBlackboardComponent())
		{
			BB->ClearValue(BBKeys::TargetActor);
			BB->ClearValue(BBKeys::bCanAttack);
		}
	}
}


void AGC_EnemyCharacter::HandleRespawn()
{
	if (!HasAuthority()) return;//server only
	Super::HandleRespawn(); //bAlive = true
	
	GetWorldTimerManager().ClearTimer(KnockbackRecoveryTimerHandle);

	if (bKnockbackLandedBound)
	{
		LandedDelegate.RemoveDynamic(this, &AGC_EnemyCharacter::OnKnockbackLanded);
		bKnockbackLandedBound = false;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->SetLooseGameplayTagCount(
			GCTags::State::CC::Knockback,
			0,
			EGameplayTagReplicationState::TagAndCountToAll);
	}

	bKnockbackPausedBrainLogic = false;

	//Set the Enemy to spawn location
	SetActorTransform(RespawnTransform);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		//Clear death effect in order to remove dead tag
		if(ActivateDeathEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(ActivateDeathEffectHandle);
			ActivateDeathEffectHandle.Invalidate();
		}
	}
	
	// Restart Behavior Tree and Brain Logic
	if (AGC_EnemyAIController* AI = Cast<AGC_EnemyAIController>(GetController()))
	{
		AI->RestartAfterRespawn();
	}
}

void AGC_EnemyCharacter::SetRespawnTransform(const FTransform& InTransform)
{
	RespawnTransform = InTransform;
}

void AGC_EnemyCharacter::OnDeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	const bool bIsDead = NewCount>0;
	// bAlive driven by replicated Tag    
	SetAlive(!bIsDead);
	
	if (bIsDead)
	{
		//Stop Character Movement when dead,prevent movement after death.
		if (UCharacterMovementComponent* MC = GetCharacterMovement())
		{
			MC->DisableMovement();
		}
		//Stop AI Logic and Movement when enemy died,prevent weird behavior like keep chasing player after death.
		//(only meaningful on Server, no operation on Client,AIController only exist on Server)
		StopAILogicAndClearBlackboradValue();
	}else
	{
		//Reset Character Movement by set walk mode, Clear velocity vector make sure it can't slide suddenly a
		if (UCharacterMovementComponent* MC = GetCharacterMovement())
		{
			MC->SetMovementMode(MOVE_Walking);
			MC->StopMovementImmediately();
		}
	}
}


void AGC_EnemyCharacter::EnterKnockbackState(const FVector& LaunchVelocity, float RecoveryDelay)
{
	if (!HasAuthority()) return;
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC || !IsAlive()) return;
	
	ASC->SetLooseGameplayTagCount(GCTags::State::CC::Knockback,1,EGameplayTagReplicationState::TagAndCountToAll);
	
	//Handle AI Logic
	PauseAILogicForKnockback();
	
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopActiveMovement();
		MovementComponent->StopMovementImmediately();
	}
	
	if (!bKnockbackLandedBound)
	{
		LandedDelegate.AddUniqueDynamic(this,&AGC_EnemyCharacter::OnKnockbackLanded);
		bKnockbackLandedBound = true;
	}
	
	LaunchCharacter(LaunchVelocity,true,true);
	
	GetWorldTimerManager().ClearTimer(KnockbackRecoveryTimerHandle);
	GetWorldTimerManager().SetTimer(
		KnockbackRecoveryTimerHandle,
		this,
		&AGC_EnemyCharacter::HandleKnockbackRecoveryTimeout,
		FMath::Max(RecoveryDelay,0.15f),
		false);
}

void AGC_EnemyCharacter::OnKnockbackLanded(const FHitResult& HitResult)
{
	if (!HasAuthority()) return;
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;
	
	if (ASC->HasMatchingGameplayTag(GCTags::State::CC::Knockback))
	{
		ExitKnockback();
	}
}

void AGC_EnemyCharacter::ExitKnockback()
{
	if (!HasAuthority()) return;
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;
	
	if (!ASC->HasMatchingGameplayTag(GCTags::State::CC::Knockback)) return;
	
	GetWorldTimerManager().ClearTimer(KnockbackRecoveryTimerHandle);
	
	if (bKnockbackLandedBound)
	{
		LandedDelegate.RemoveDynamic(this,&AGC_EnemyCharacter::OnKnockbackLanded);
		bKnockbackLandedBound = false;
	}
	
	ASC->SetLooseGameplayTagCount(GCTags::State::CC::Knockback,0,EGameplayTagReplicationState::TagAndCountToAll);
	
	if (IsAlive())
	{
		ResumeAILogicAfterKnockback();
	}
	else
	{
		bKnockbackPausedBrainLogic = false;
	}
}

void AGC_EnemyCharacter::HandleKnockbackRecoveryTimeout()
{
	ExitKnockback();
}

void AGC_EnemyCharacter::PauseAILogicForKnockback()
{
	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI) return;
	
	AI->StopMovement();
	
	UBrainComponent* BrainComponent = AI->GetBrainComponent();
	if (BrainComponent)
	{
		if (!bKnockbackPausedBrainLogic)
		{
			BrainComponent->PauseLogic(TEXT("knockback"));
			bKnockbackPausedBrainLogic = true;
		}
	}
	
}

void AGC_EnemyCharacter::ResumeAILogicAfterKnockback()
{
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* BrainComponent = AIController->GetBrainComponent())
		{
			if (bKnockbackPausedBrainLogic)
			{
				BrainComponent->ResumeLogic(TEXT("KnockbackEnded"));
				bKnockbackPausedBrainLogic = false;
			}
		}
	}
}
