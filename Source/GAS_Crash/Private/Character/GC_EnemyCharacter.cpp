#include "Character/GC_EnemyCharacter.h"
#include "AbilitySystem/GC_AbilitySystemComponent.h"
#include "AbilitySystem/GC_AttributeSet.h"
#include "Perception/PawnSensingComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/GC_PlayerCharacter.h"
#include "AI/GC_AITypeDefs.h"

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

	//Subscribe Delegates from PawnSensingComponent,Bind Callback Functions
	if (PawnSensingComponent)
	{
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &AGC_EnemyCharacter::OnSeePawn);
	}
	
	//Build Ability Cache
	BuildAbilityCache();
	
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
	Blackboard->SetValueAsObject(BBKeys::TargetActor,Player);
	
}

void AGC_EnemyCharacter::BuildAbilityCache()
{
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyCharacter] Cannot build ability cache: ASC is null"));
		return;
	}
	
	//Clear Old Cache
	AbilityHandleCache.Empty();
	
	//Loop All Given Abilities, build Tag -> Handle mapping.
	TArray<FGameplayAbilitySpec> AllSpecs = AbilitySystemComponent->GetActivatableAbilities();
	
	for (const FGameplayAbilitySpec& Spec : AllSpecs)
	{
		if (!Spec.Ability) continue;
		
		for (const FGameplayTag& Tag : Spec.Ability->GetAssetTags())
		{
			AbilityHandleCache.Add(Tag,Spec.Handle);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("[EnemyCharacter] %s: Built ability cache with %d mappings"), *GetName(), AbilityHandleCache.Num());
}


bool AGC_EnemyCharacter::IsAbilityReady(const FGameplayTag& AbilityTag) const
{
	if (!AbilitySystemComponent) return false;
	//Find Ability Handle from Cache(Tag -> Handle) O(1) complexity
	const FGameplayAbilitySpecHandle* FoundHandlePtr = AbilityHandleCache.Find(AbilityTag);
	if (!FoundHandlePtr) return false;
	
	//GetAbilitySpec
	FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(*FoundHandlePtr);
	if (!AbilitySpec) return false;
	
	//Get ActorInfo
	const FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent->AbilityActorInfo.Get();
	if (!ActorInfo) return false;
	
	// 核心：CanActivateAbility 会帮你检查：
	// - Cooldown (基于 CD GE)
	// - Cost (基于 Cost GE)
	// - Tags (基于 Ability 的 Cancel/Block 机制)
	return AbilitySpec->Ability->CanActivateAbility(*FoundHandlePtr, ActorInfo);
}

bool AGC_EnemyCharacter::TryActivateAbilityByTag(const FGameplayTag& AbilityTag)
{
	if(!AbilitySystemComponent) return false;
	//Find Ability Handle from Cache(Tag -> Handle) O(1) complexity
	const FGameplayAbilitySpecHandle* SpecHandle = AbilityHandleCache.Find(AbilityTag);
	if (!SpecHandle)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyCharacter] Ability with tag '%s' not found"), *AbilityTag.ToString());
		return false;
	}
	
	// 2. 深度预检 (包含了 CD, Cost, 和状态阻挡)
	if (!IsAbilityReady(AbilityTag)) 
	{
		return false; 
	}
	
	//Try to Activate Ability
	return AbilitySystemComponent->TryActivateAbility(*SpecHandle);
}

bool AGC_EnemyCharacter::HasAnyAbilityReady(const FGameplayTagContainer& AbilityTags)
{
	for (const FGameplayTag& Tag : AbilityTags)
	{
		if (IsAbilityReady(Tag))
		{
			return true;
		}
	}
	return false;
}

