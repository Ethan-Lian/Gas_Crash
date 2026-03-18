#pragma once
#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "DamageType/GC_DamageFeedbackTypes.h"
#include "GC_AttributeSet.generated.h"

//Use macro auto spawn code
#define ATTRIBUTE_ACCESSORS(ClassName,PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName,PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName) 

//Create a dynamic multicast delegate to notify WidgetComponent that AttributeSet has been initialized by GE, 
//and the Attribute values are ready to use.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAttributeSetInitialized);

//Create a native delegate to confirm damage type, this is used to send damage feedback data from AttributeSet to HealthComponent, 
//because AttributeSet is the only one who can confirm the damage type after GE executed, and HealthComponent is responsible for sending GameplayCue and GameplayEvent, so we need this delegate to send the damage feedback data to HealthComponent.
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDamageConfirmedNative,const FGC_DamageFeedbackData&);


UCLASS()
class GAS_CRASH_API UGC_AttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	
	//================Core Attribute=================
	ATTRIBUTE_ACCESSORS(ThisClass,Health);
	ATTRIBUTE_ACCESSORS(ThisClass,MaxHealth);
	ATTRIBUTE_ACCESSORS(ThisClass,Mana);
	ATTRIBUTE_ACCESSORS(ThisClass,MaxMana);
	ATTRIBUTE_ACCESSORS(ThisClass,Armor);

	// Meta Attributes
	// IncomingDamage is a "scratch pad" value — it is NEVER replicated and NEVER shown in UI.
	// The ExecCalc writes the final computed damage here; PostGameplayEffectExecute reads,
	// applies it to Health, then immediately zeros it out.
	// This decouples the "how much damage" calculation (ExecCalc) from the "what happens
	// to the character" side-effects (PostGEExecute: clamp, shield, death check, feedback).
	ATTRIBUTE_ACCESSORS(ThisClass, IncomingDamage);
	ATTRIBUTE_ACCESSORS(ThisClass, IncomingHealing);

	//================Attribute Define================
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;

	UPROPERTY(BlueprintReadOnly,ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly,ReplicatedUsing=OnRep_Mana)
	FGameplayAttributeData Mana;

	UPROPERTY(BlueprintReadOnly,ReplicatedUsing=OnRep_MaxMana)
	FGameplayAttributeData MaxMana;

	// Armor is captured by the ExecCalc (GC_DamageExecution) from the TARGET's AttributeSet.
	// This is the core reason ExecCalc exists over simple Modifiers — it can read attributes
	// from BOTH source and target to compute cross-actor formulas like damage reduction.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Armor)
	FGameplayAttributeData Armor;

	// Meta Attributes — not replicated, consumed and cleared in PostGameplayEffectExecute.
	UPROPERTY(BlueprintReadOnly, Category = "GC|Attributes|Meta")
	FGameplayAttributeData IncomingDamage;

	UPROPERTY(BlueprintReadOnly, Category = "GC|Attributes|Meta")
	FGameplayAttributeData IncomingHealing;
	
	//================Hook Function========================
	
	// Replicate Server changed Health to client
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldValue) const;
	
	//================Delegate========================

	//AttributeInitialized delegate is used to notify WidgetComponent that AttributeSet has been initialized by GE, and the Attribute values are ready to use. 
	UPROPERTY(BlueprintAssignable)
	FAttributeSetInitialized OnAttributeSetInitialized;
	
	//Damage type confirmation delegate, send to health component when the attribute set confirm the damage type.
	FOnDamageConfirmedNative OnDamageConfirmed;

	//=================================================

	// Replication registration mechanism(机制), register the values that need to be replicated to UE's network system.
	// When the server's attribute value changes, it is replicated to the client, and triggers the OnRep function. 
	// In the OnRep function, the new value is injected into the ASC, the base value is aligned, prediction is handled (rollback), and all delegates subscribed to attribute changes are triggered.
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	//Initialization flag, used to tell WidgetComponent whether the attribute initialization is completed. Why do we need replication here? The client needs to know whether the server has completed initialization to avoid binding too early.
	//ReplicatedUsing: when the value is replicated to client, it will call the function OnRep_AttributeInitialized 
	UPROPERTY(ReplicatedUsing = OnRep_AttributeInitialized)
	bool bAttributeInitialized = false;

	//Notify WidgetComponent that AttributeSet has been initialized by GE, and the Attribute values are ready to use.
	UFUNCTION()
	void OnRep_AttributeInitialized();
	
	// ⭐⭐⭐ Interview Point: PreAttributeChange vs PostGameplayEffectExecute
	// PreAttributeChange clamps the CURRENT VALUE (the value seen by queries like GetMana()).
	// It does NOT modify the BaseValue — if you need BaseValue correction, use PostGEExecute with SetHealth().
	// This is defense-in-depth for the Cost system: prevents Mana from displaying as negative
	// even if a Cost GE overshoots. CheckCost via CanApplyAttributeModifiers() already blocks
	// activation when resources are insufficient, but edge cases (prediction, simultaneous GEs)
	// can still produce negative intermediate values.
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	//Callback function after attribute changed, only triggered when the value change is effective,
	//trigger timing: after GE modify BaseValue, before attribute replication, always triggered on server, only triggered on client when local prediction succeed.
	virtual void  PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
