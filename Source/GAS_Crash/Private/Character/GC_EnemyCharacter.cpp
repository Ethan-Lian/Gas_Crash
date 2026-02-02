#include "Character/GC_EnemyCharacter.h"
#include "AbilitySystem/GC_AbilitySystemComponent.h"
#include "AbilitySystem/GC_AttributeSet.h"
#include "Perception/PawnSensingComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/GC_PlayerCharacter.h"

AGC_EnemyCharacter::AGC_EnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	//Create ASC and set Replication,make sure Server data can replicate to client
	AbilitySystemComponent = CreateDefaultSubobject<UGC_AbilitySystemComponent>(FName("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	//Create AttributeSet
	AttributesSet = CreateDefaultSubobject<UGC_AttributeSet>(FName("AttributeSet"));

	//Create PawnSensing Component
	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
	PawnSensingComponent->SightRadius = 1000.f;
	PawnSensingComponent->SetPeripheralVisionAngle(90.f);
}

void AGC_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(GetAbilitySystemComponent())) return;
	//Initialize ASC,OwnerActor(数据拥有者)->Enemy,AvatarActor(表现持有者)->Enemy
	GetAbilitySystemComponent()->InitAbilityActorInfo(this,this);
	OnAscInitialized.Broadcast(GetAbilitySystemComponent(),GetAttributeSet());

	if (!HasAuthority()) return;  //if not in sever,don't give gameability
	GiveStartupAbilities(); //inherits from MyBaseCharacter
	InitializeAttribute(); //Initialize Attribute by GE

	//Subscribe the Delegate Listen the Attribute change.
	UGC_AttributeSet* GC_AS = Cast<UGC_AttributeSet>(GetAttributeSet());
	if (!GC_AS ) return;
	GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(GC_AS->GetHealthAttribute()).AddUObject(this,&ThisClass::OnHealthChanged);

	//Subscribe Delegates from PawnSensingComponent,Bind Callback Functions
	if (PawnSensingComponent)
	{
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &AGC_EnemyCharacter::OnSeePawn);
	}
}

UAbilitySystemComponent* AGC_EnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAttributeSet* AGC_EnemyCharacter::GetAttributeSet() const
{
	return AttributesSet;
}

float AGC_EnemyCharacter::GetRandomAttackDelay() const
{
	return FMath::FRandRange(MinAttackDelay, MaxAttackDelay);
}

void AGC_EnemyCharacter::OnSeePawn(APawn* SeenPawn)
{
	//only follow player,not other enemy.
	if (!SeenPawn || SeenPawn == this) return;

	AGC_PlayerCharacter* Player = Cast<AGC_PlayerCharacter>(SeenPawn);
	if (!Player) return;

	//Get AIController and BlackBoard
	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController) return;

	UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
	if (!Blackboard) return;

	//Set TargetToFollow in BlackBoard
	Blackboard->SetValueAsObject(FName("TargetToFollow"), Player);
	
}
