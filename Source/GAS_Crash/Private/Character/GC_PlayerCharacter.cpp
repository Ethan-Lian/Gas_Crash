#include "GAS_Crash/Public/Character/GC_PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GC_AttributeSet.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameplayTags/GCTags.h"
#include "GameMode/GC_GameMode.h"
#include "Player/GC_PlayerState.h"

AGC_PlayerCharacter::AGC_PlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	GetCapsuleComponent()->InitCapsuleSize(42.f,96.f);
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0,540.f,0);
	
	//create camera and springarm
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(FName("SpringArm"));
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->TargetArmLength = 600.f;
	SpringArmComponent->bUsePawnControlRotation = true;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent,USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;
}


UAbilitySystemComponent* AGC_PlayerCharacter::GetAbilitySystemComponent() const
{
	//TryGet PlayerCharacter owned PlayerState,because GC_PlayerState created the ASC,so you need get this first
	AGC_PlayerState* GCPlayerState = Cast<AGC_PlayerState>(GetPlayerState());
	if (!IsValid(GCPlayerState)) return nullptr;
	
	return GCPlayerState->GetAbilitySystemComponent();
}


// Init in Server
void AGC_PlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	//set InOwnerActor and InAvatarActor by InitAbilityActorInfo function
	//HasAuthority() : check if the code running on the server
	//GAS authority operations(Giving Ability,Apply Effect) must run on the server 
	if (!IsValid(GetAbilitySystemComponent()) || !HasAuthority()) return;
	
	//Initialize ASC's owner and avatar
	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(),this);

	//Identity belongs to the ASC layer so other GAS systems can query it consistently.
	//Set a loose tag count on the ASC to identify this as a player character, this is replicated to clients.
	GetAbilitySystemComponent()->SetLooseGameplayTagCount(
		GCTags::GCIdentity::Player,
		1,
		EGameplayTagReplicationState::TagAndCountToAll);
	
	//Delegate ASC and AS
	OnAscInitialized.Broadcast(GetAbilitySystemComponent(),GetAttributeSet());
	
	//StartupAbilities,only server can Give Ability.
	GiveStartupAbilities();
	
	//Initialize Attribute by GE
	InitializeAttribute();

	//Subscribe the Delegate Listen the Attribute change.
	UGC_AttributeSet* GCAS = Cast<UGC_AttributeSet>(GetAttributeSet());
	if (!GCAS) return;
	GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(GCAS->GetHealthAttribute()).AddUObject(this,&ThisClass::OnHealthChanged);

	//Listen Dead Tag change by delegate (for UI updates, animation state, etc.)
	GetAbilitySystemComponent()->RegisterGameplayTagEvent(GCTags::GCEvents::player::Dead,EGameplayTagEventType::NewOrRemoved).AddUObject(this,&AGC_PlayerCharacter::OnDeadTagChanged);
}

//Init in Client , client need to know who is avatar and owner,so it can show UI and play Animations.
void AGC_PlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (!IsValid(GetAbilitySystemComponent())) return;
	
	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(),this);
	
	OnAscInitialized.Broadcast(GetAbilitySystemComponent(),GetAttributeSet());

	//Subscribe the Delegate Listen the Attribute change.
	UGC_AttributeSet* GCAS = Cast<UGC_AttributeSet>(GetAttributeSet());
	if (!GCAS) return;
	GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(GCAS->GetHealthAttribute()).AddUObject(this,&ThisClass::OnHealthChanged);

	//Listen Dead Tag change by delegate (for UI updates, animation state, etc.)
	GetAbilitySystemComponent()->RegisterGameplayTagEvent(GCTags::GCEvents::player::Dead,EGameplayTagEventType::NewOrRemoved).AddUObject(this,&AGC_PlayerCharacter::OnDeadTagChanged);
}

UAttributeSet* AGC_PlayerCharacter::GetAttributeSet() const
{
	AGC_PlayerState* GC_PlayerState = Cast<AGC_PlayerState>(GetPlayerState());
	if (!IsValid(GC_PlayerState)) return nullptr;
	
	return GC_PlayerState->GetAttributeSet();
}

void AGC_PlayerCharacter::HandleDeath()
{
	Super::HandleDeath();
	
	if (!bDeathHandled) return;
	
	// Apply DeathEffect and Activate Death Ability
	ApplyDeathEffectAndActivateDeathEffect();

	// Notify GameMode for respawn
	if (HasAuthority())
	{
		if (AGC_GameMode* GCGameMode = GetWorld()->GetAuthGameMode<AGC_GameMode>())
		{
			GCGameMode->RequestRespawnForPlayer(this);
		}
	}
}

void AGC_PlayerCharacter::HandleRespawn()
{
	Super::HandleRespawn();

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	//remove DeathEffect (remove dead tag)
	if (ActiveDeathEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(ActiveDeathEffectHandle);
		ActiveDeathEffectHandle.Invalidate();
	}

	//Re-enable movement and collision
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
	}

	//Set actor collision
	SetActorEnableCollision(true);
}

void AGC_PlayerCharacter::ApplyDeathEffectAndActivateDeathEffect()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;
	
	//Activate Death Ability (montage, camera)
	ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(GCTags::GCAbilities::player::Death));
	
	//Apply DeathEffect (adds Dead tag, disables input)
	if (DeathEffect)
	{
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);
		
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DeathEffect,1.f,ContextHandle);
		
		if (SpecHandle.IsValid())
		{
			ActiveDeathEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void AGC_PlayerCharacter::OnDeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	const bool bIsDead = NewCount > 0;

	// bAlive is driven by replicated Dead tag
	// This ensures client and server stay in sync
	SetAlive(!bIsDead);
}

