#include "GAS_Crash/Public/Character/GC_PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
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
	
	//StartupAbilities,only server can Give Ability.
	GiveStartupAbilities();
}

//Init in Client , client need to know who is avatar and owner,so it can show UI and play animaitons.
void AGC_PlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (!IsValid(GetAbilitySystemComponent())) return;
	
	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(),this);
}


