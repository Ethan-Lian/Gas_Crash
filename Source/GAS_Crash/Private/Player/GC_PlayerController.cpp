#include "Player/GC_PlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GC_AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameplayTags/GCTags.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Player/GC_InputConfig.h"

void AGC_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputLocalPlayerSubsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			for (UInputMappingContext* Context : InputMappingContexts)
			{
				if (IsValid(Context))
				{
					InputLocalPlayerSubsystem->AddMappingContext(Context, 0);
				}
			}
		}
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!IsValid(EnhancedInputComponent) || !IsValid(InputConfig))
	{
		return;
	}

	BindNativeInputActions(EnhancedInputComponent);
	BindAbilityInputActions(EnhancedInputComponent);
}

void AGC_PlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (UGC_AbilitySystemComponent* GCASC = GetGCAbilitySystemComponent())
	{
		if (IsDied())
		{
			GCASC->ClearAbilityInput();
			return;
		}

		GCASC->ProcessAbilityInput(DeltaTime, IsPaused());
	}
}

void AGC_PlayerController::BindNativeInputActions(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (!IsValid(InputConfig) || !IsValid(EnhancedInputComponent))
	{
		return;
	}

	if (const UInputAction* JumpIA = InputConfig->FindNativeInputActionForTag(GCTags::InputTag::Jump, false))
	{
		EnhancedInputComponent->BindAction(JumpIA, ETriggerEvent::Started, this, &ThisClass::Input_JumpPressed);
		EnhancedInputComponent->BindAction(JumpIA, ETriggerEvent::Completed, this, &ThisClass::Input_JumpReleased);
	}

	if (const UInputAction* MoveIA = InputConfig->FindNativeInputActionForTag(GCTags::InputTag::Move, false))
	{
		EnhancedInputComponent->BindAction(MoveIA, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
	}

	if (const UInputAction* LookIA = InputConfig->FindNativeInputActionForTag(GCTags::InputTag::Look, false))
	{
		EnhancedInputComponent->BindAction(LookIA, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
	}
}

void AGC_PlayerController::BindAbilityInputActions(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (!IsValid(InputConfig) || !IsValid(EnhancedInputComponent))
	{
		return;
	}

	for (const FGC_InputAction& Action : InputConfig->AbilityInputActions)
	{
		if (!Action.InputAction || !Action.InputTag.IsValid())
		{
			continue;
		}

		EnhancedInputComponent->BindAction(
			Action.InputAction,
			ETriggerEvent::Started,
			this,
			&ThisClass::Input_AbilityInputTagPressed,
			Action.InputTag);

		EnhancedInputComponent->BindAction(
			Action.InputAction,
			ETriggerEvent::Completed,
			this,
			&ThisClass::Input_AbilityInputTagReleased,
			Action.InputTag);

		// Canceled fires when a hold trigger releases early or input is interrupted.
		// Without this, the spec stays stuck in InputHeldSpecHandles.
		EnhancedInputComponent->BindAction(
			Action.InputAction,
			ETriggerEvent::Canceled,
			this,
			&ThisClass::Input_AbilityInputTagReleased,
			Action.InputTag);
	}
}

void AGC_PlayerController::Input_JumpPressed()
{
	if (IsDied())
	{
		return;
	}

	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		ControlledCharacter->Jump();
	}
}

void AGC_PlayerController::Input_JumpReleased()
{
	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		ControlledCharacter->StopJumping();
	}
}

void AGC_PlayerController::Input_Move(const FInputActionValue& Value)
{
	if (!IsValid(GetPawn()) || IsDied())
	{
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	GetPawn()->AddMovementInput(ForwardDirection, MovementVector.Y);
	GetPawn()->AddMovementInput(RightDirection, MovementVector.X);
}

void AGC_PlayerController::Input_Look(const FInputActionValue& Value)
{
	if (!IsValid(GetPawn()) || IsDied())
	{
		return;
	}

	const FVector2D LookVector = Value.Get<FVector2D>();
	AddYawInput(LookVector.X);
	AddPitchInput(LookVector.Y);
}

void AGC_PlayerController::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (IsDied())
	{
		return;
	}

	if (UGC_AbilitySystemComponent* GCASC = GetGCAbilitySystemComponent())
	{
		GCASC->AbilityInputTagPressed(InputTag);
	}
}

void AGC_PlayerController::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (UGC_AbilitySystemComponent* GCASC = GetGCAbilitySystemComponent())
	{
		GCASC->AbilityInputTagReleased(InputTag);
	}
}

bool AGC_PlayerController::IsDied() const
{
	const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn());
	return ASC && ASC->HasMatchingGameplayTag(GCTags::GCEvents::player::Dead);
}

UGC_AbilitySystemComponent* AGC_PlayerController::GetGCAbilitySystemComponent() const
{
	return Cast<UGC_AbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()));
}
