#include "Player/GC_PlayerController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GameFramework/Character.h"
#include "GameplayTags/GCTags.h"
void AGC_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	//Enhanced Input Subsystem is used to manage input mappings and contexts for the local player
	UEnhancedInputLocalPlayerSubsystem* InputLocalPlayerSubsystem = 
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	
	if (!IsValid(InputLocalPlayerSubsystem)) return;
	
	//Get Input Mapping Contexts and add to subsystem
	for (UInputMappingContext* Context : InputMappingContexts)
	{
		InputLocalPlayerSubsystem->AddMappingContext(Context,0);
	}
	
	//Get EnhanceInputComponent
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	
	if (!IsValid(EnhancedInputComponent)) return;

	//Binding input actions to ability activation
	EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Started,this,&ThisClass::Jump);//this和&ThisClass::jump是啥意思
	EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Completed,this,&AGC_PlayerController::JumpStop);
	EnhancedInputComponent->BindAction(MoveAction,ETriggerEvent::Triggered,this,&AGC_PlayerController::Move);
	EnhancedInputComponent->BindAction(LookAction,ETriggerEvent::Triggered,this,&AGC_PlayerController::Look);
	EnhancedInputComponent->BindAction(PrimaryAction,ETriggerEvent::Triggered,this,&AGC_PlayerController::Primary);
	EnhancedInputComponent->BindAction(SecondaryAction,ETriggerEvent::Started,this,&AGC_PlayerController::Secondary);
	EnhancedInputComponent->BindAction(TertiaryAction,ETriggerEvent::Started,this,&AGC_PlayerController::Tertiary);
}

/*
 * Input callback function
 */
void AGC_PlayerController::Jump()
{
	if (!IsValid(GetCharacter())) return;

	//Block jump when dead
	if (IsDied()) return;

	GetCharacter()->Jump();
}

void AGC_PlayerController::JumpStop()
{
	if (!IsValid(GetCharacter())) return;
	
	if (IsDied()) return;
	
	GetCharacter()->StopJumping();
}

void AGC_PlayerController::Move(const FInputActionValue& value)
{
	//ue5编辑器里面InputAction设置的y是向前
	//2维系统里面x是左右,y是上下,3维系统里x是向前,y是右
	if (!IsValid(GetPawn())) return;

	// Block input when player is dead
	if (IsDied()) return;

	//设置的InputAction里面需要的是一个2维向量,y是向前,x是左右
	const FVector2D MovementVector = value.Get<FVector2D>();

	//find which way is forward and right
	const FRotator YawRotation(0.f,GetControlRotation().Yaw,0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	//Add MovementInput
	GetPawn()->AddMovementInput(ForwardDirection,MovementVector.Y);
	GetPawn()->AddMovementInput(RightDirection,MovementVector.X);

}

void AGC_PlayerController::Look(const FInputActionValue& value)
{
	if (!IsValid(GetPawn())) return;

	if (IsDied()) return;
	
	const FVector2D MovementVector = value.Get<FVector2D>();
	AddYawInput(MovementVector.X);
	AddPitchInput(MovementVector.Y);
}

void AGC_PlayerController::Primary()
{
	if (IsDied()) return;
	ActivateAbility(GCTags::GCAbilities::player::Primary);
}

void AGC_PlayerController::Secondary()
{
	if (IsDied()) return;
	ActivateAbility(GCTags::GCAbilities::player::Secondary);
}

void AGC_PlayerController::Tertiary()
{
	if (IsDied()) return;
	ActivateAbility(GCTags::GCAbilities::player::Tertiary);
}


void AGC_PlayerController::ActivateAbility(const FGameplayTag& AbilityTag) const
{
	// Use UAbilitySystemBlueprintLibrary to get ASC
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn());
	if (!IsValid(AbilitySystemComponent)) return;

	// Block ability activation when dead
	// This prevents attacking/using abilities during death state
	if (AbilitySystemComponent->HasMatchingGameplayTag(GCTags::GCEvents::player::Dead)) return;

	//Activate Ability by ASC.
	AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTag.GetSingleTagContainer());
}

bool AGC_PlayerController::IsDied()
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn());
	
	if (ASC && ASC->HasMatchingGameplayTag(GCTags::GCEvents::player::Dead)) return true;
	
	return false;
}
