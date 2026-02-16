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
	OnAscInitialized.Broadcast(GetAbilitySystemComponent(),GetAttributeSet());
	
	//only in server,giving abilities,initialized Attribute,
	if (HasAuthority())
	{
		//inherits from MyBaseCharacter
		GiveStartupAbilities(); 
		//Initialize Attribute by GE
		InitializeAttribute(); 
	}
	
	//Subscribe the Delegate Listen the Attribute change.
	UGC_AttributeSet* GC_AS = Cast<UGC_AttributeSet>(GetAttributeSet());
	if (!GC_AS ) return;
	GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(GC_AS->GetHealthAttribute()).AddUObject(this,&ThisClass::OnHealthChanged);
	
	//Listen Dead Tag change by delegate
	GetAbilitySystemComponent()->RegisterGameplayTagEvent(GCTags::State::Dead,EGameplayTagEventType::NewOrRemoved).AddUObject(this,&AGC_EnemyCharacter::OnDeadTagChanged);
}

void AGC_EnemyCharacter::HandleDeath()
{
	Super::HandleDeath();
	
	// Server only
	if (!HasAuthority()) return;
	
	//if HandleDeath already called,just return,avoid multiple handling.
	if(bDeathHandled) return;
	bDeathHandled = true;
	
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
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
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
		// Activate DeathAbility to play Montage.
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(GCTags::GCAbilities::Death));
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
	ResetAttributes();// reset attributes
	
	 //Set the Enemy to spawn location
	SetActorTransform(RespawnTransform);

	//Allow to handle death next time
	bDeathHandled = false;
	
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
	const bool bIsDead = NewCount>0; //NewCount
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
