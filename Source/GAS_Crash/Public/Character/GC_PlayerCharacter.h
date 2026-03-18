#pragma once
#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "MyBaseCharacter.h"
#include "GC_PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
struct FGameplayTag;

UCLASS()
class GAS_CRASH_API AGC_PlayerCharacter : public AMyBaseCharacter
{
	GENERATED_BODY()

public:
	AGC_PlayerCharacter();
		
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	//PlayerState初始化接口,ASC注册,赋予初始能力等等,只在服务器执行
	virtual void PossessedBy(AController* NewController) override;
	
	//PlayerState Replication 复制接口,只在客户端执行
	virtual void OnRep_PlayerState() override;
	
	//Getter for AttributeSet
	virtual UAttributeSet* GetAttributeSet() const override;
	
	//Handle Death,Server only
	virtual void HandleDeath() override;

	//Handle Respawn,Server only
	virtual void HandleRespawn() override;

protected:
	//Handle Dead Tag Changed, called when Dead Tag count changed
	void OnDeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

protected:
	//Death Effect (adds Dead Tag)
	UPROPERTY(EditDefaultsOnly,Category = "GC|Effects")
	TSubclassOf<UGameplayEffect> DeathEffect;
	
	//Cache death effect handle for removal on respawn
	FActiveGameplayEffectHandle ActiveDeathEffectHandle;
private:
	UPROPERTY(VisibleAnywhere,Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent;
	
	UPROPERTY(VisibleAnywhere,Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent;	

	//Apply DeathEffect and Activate DeathAbility when Enemy died.
	void ApplyDeathEffectAndActivateDeathEffect();
};
