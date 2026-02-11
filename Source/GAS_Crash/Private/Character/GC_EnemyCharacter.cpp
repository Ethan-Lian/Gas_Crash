#include "Character/GC_EnemyCharacter.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "AbilitySystem/GC_AbilitySystemComponent.h"
#include "AbilitySystem/GC_AttributeSet.h"
#include "AI/GC_AITypeDefs.h"
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
	
}

void AGC_EnemyCharacter::HandleDeath()
{
	Super::HandleDeath();
	
	//if HandleDeath already called,just return,avoid multiple handling.
	if(bDeathHandled) return;
	bDeathHandled = true;
	
	//Stop AI Logic and Movement when enemy died,prevent weird behavior like keep chasing player after death.
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
	
	//Stop Character Movement when dead,prevent movement after death.
	if(UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}
	
	// Only in Server Handle GA and GE.
	Handle_DeathAbiltyAndDeathEffect();
}

UAbilitySystemComponent* AGC_EnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAttributeSet* AGC_EnemyCharacter::GetAttributeSet() const
{
	return AttributesSet;
}

void AGC_EnemyCharacter::Handle_DeathAbiltyAndDeathEffect()
{
	if (HasAuthority())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			if (GE_Death)
			{
				//Give DeadTag to mark this character is dead by GE_Death.
				FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GE_Death,1.f,EffectContextHandle);
				if (SpecHandle.IsValid())
				{
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
			// Activate DeathAbility to play Montage.
			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(GCTags::GCAbilities::Death));
		}
	}
}
