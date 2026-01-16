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
	//get playercharacter own Playerstate,because GC_Playerstate created the ASC,so you need get this first
	AGC_PlayerState* GCPlayerState = Cast<AGC_PlayerState>(GetPlayerState());
	if (!IsValid(GCPlayerState)) return nullptr;
	
	return GCPlayerState->GetAbilitySystemComponent();
}

void AGC_PlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	//set InOwnerActor and InAvatarActor by InitAbilityActorInfo function
	if (!IsValid(GetAbilitySystemComponent())) return;
	
	//为什么这里直接可以getPlayerState,   playerstate是怎么连接到character的??
	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(),this);
}

void AGC_PlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (!IsValid(GetAbilitySystemComponent())) return;
	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(),this);
}
