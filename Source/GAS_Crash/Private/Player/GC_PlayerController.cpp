#include "Player/GC_PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GameFramework/Character.h"

void AGC_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	//通过本地玩家类的get方法来获取强化输入本地玩家子系统,但是这是干什么的?
	UEnhancedInputLocalPlayerSubsystem* InputLocalPlayerSubsystem = 
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	
	if (!IsValid(InputLocalPlayerSubsystem)) return;
	
	//获取输入映射文本,来添加到本地玩家子系统
	for (UInputMappingContext* Context : InputMappingContexts)
	{
		InputLocalPlayerSubsystem->AddMappingContext(Context,0);
	}
	
	//获取EnhanceInputComponent
	UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(InputComponent);
	
	if (!IsValid(EnhancedInputComponent)) return;
	//给增强输入组件绑定输入动作
	EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Started,this,&ThisClass::Jump);//this和&ThisClass::jump是啥意思
	EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Completed,this,&AGC_PlayerController::JumpStop);
	EnhancedInputComponent->BindAction(MoveAction,ETriggerEvent::Triggered,this,&AGC_PlayerController::Move);
	EnhancedInputComponent->BindAction(LookAction,ETriggerEvent::Triggered,this,&AGC_PlayerController::Look);
	EnhancedInputComponent->BindAction(PrimaryAction,ETriggerEvent::Started,this,&AGC_PlayerController::Primary);
}

void AGC_PlayerController::Jump() 
{
	if (!IsValid(GetCharacter())) return;
	GetCharacter()->Jump();
}

void AGC_PlayerController::JumpStop()
{
	if (!IsValid(GetCharacter())) return;
	GetCharacter()->StopJumping();
}

void AGC_PlayerController::Move(const FInputActionValue& value)
{
	//ue5编辑器里面InputAction设置的y是向前
	//2维系统里面x是左右,y是上下,3维系统里x是向前,y是右
	if (!IsValid(GetPawn())) return;
	
	//设置的InputAction里面需要的是一个2维向量,y是向前,x是左右
	const FVector2D MovementVector = value.Get<FVector2D>();
	
	//find which way is forward and right
	const FRotator YawRotation(0.f,GetControlRotation().Yaw,0.f);//这里为什么不是value.Yaw,不是通过输入的3维向量来获取yaw吗
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);//这里为什么不是EAxis::Y,y不是向前的吗
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);//为什么获取单位x,y方向向量,获取方向向量是怎么添加到角色移动能力上的
	
	//Add MovementInput
	GetPawn()->AddMovementInput(ForwardDirection,MovementVector.Y);
	GetPawn()->AddMovementInput(RightDirection,MovementVector.X);
	
}

void AGC_PlayerController::Look(const FInputActionValue& value)
{
	if (!IsValid(GetPawn())) return;
	
	const FVector2D MovementVector = value.Get<FVector2D>();
	AddYawInput(MovementVector.X);
	AddPitchInput(MovementVector.Y);
}

void AGC_PlayerController::Primary()
{
	if (!IsValid(GetPawn())) return;
	UE_LOG(LogTemp,Warning,TEXT("PRIMARY"));
}