#pragma once
#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GC_AttributeChangeTask.generated.h"

struct FOnAttributeChangeData;
class UAbilitySystemComponent;

// Delegate for Attribute Changed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChanged, FGameplayAttribute, Attribute, float, NewValue,float, OldValue);

UCLASS(BlueprintType,meta=(ExposedAsyncProxy = AsyncTask))
class GAS_CRASH_API UGC_AttributeChangeTask : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	//Attribute Changed Delegate
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChanged OnAttributeChanged;
	
	
	UFUNCTION(BlueprintCallable,meta=(BlueprintInternalUseonly = "true"))
	static UGC_AttributeChangeTask* ListenAttributeChanged(FGameplayAttribute Attribute,UAbilitySystemComponent* AbilitySystemComponent);
	
	UFUNCTION(BlueprintCallable)
	void EndTask ();
	
	TWeakObjectPtr<UAbilitySystemComponent> ASC;
	
	FGameplayAttribute AttributeToListen;
	
	void CBF_AttributeChanged(const FOnAttributeChangeData& Data);
	
};
