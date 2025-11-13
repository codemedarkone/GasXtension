// Copyright Epic Games, Inc.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GasXDebugAttributes.generated.h"

/**
 * Small hand-authored AttributeSet used to validate bootstrap and replication behavior.
 *
 * WHY: Provide a minimal, testable AttributeSet (Health, Stamina) so runtime guardrails
 * can be validated before implementing the full generator.
 */
UCLASS()
class GASXRUNTIME_API UGasXDebugAttributes : public UAttributeSet
{
    GENERATED_BODY()

public:
    UGasXDebugAttributes();

    /** Health attribute */
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
    FGameplayAttributeData Health;

    /** Stamina attribute */
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Stamina)
    FGameplayAttributeData Stamina;

    // Accessors
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UGasXDebugAttributes, Health)
    GAMEPLAYATTRIBUTE_VALUE_GETTER(Health)
    GAMEPLAYATTRIBUTE_VALUE_SETTER(Health)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(Health)

    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UGasXDebugAttributes, Stamina)
    GAMEPLAYATTRIBUTE_VALUE_GETTER(Stamina)
    GAMEPLAYATTRIBUTE_VALUE_SETTER(Stamina)
    GAMEPLAYATTRIBUTE_VALUE_INITTER(Stamina)

    // RepNotifies
    UFUNCTION()
    virtual void OnRep_Health(const FGameplayAttributeData& OldValue);

    UFUNCTION()
    virtual void OnRep_Stamina(const FGameplayAttributeData& OldValue);

    // Convenience setters used by bootstrap for MVP initialization
    void SetHealthValue(float NewValue);
    void SetStaminaValue(float NewValue);

    // Net replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
