// Copyright Epic Games, Inc.

#include "Attributes/GasXDebugAttributes.h"
#include "Net/UnrealNetwork.h"

UGasXDebugAttributes::UGasXDebugAttributes()
{
    // Set sensible defaults in constructor
    Health.SetCurrentValue(100.f);
    Stamina.SetCurrentValue(100.f);
}

void UGasXDebugAttributes::OnRep_Health(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGasXDebugAttributes, Health, OldValue);
}

void UGasXDebugAttributes::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGasXDebugAttributes, Stamina, OldValue);
}

void UGasXDebugAttributes::SetHealthValue(float NewValue)
{
    Health.SetCurrentValue(NewValue);
}

void UGasXDebugAttributes::SetStaminaValue(float NewValue)
{
    Stamina.SetCurrentValue(NewValue);
}

void UGasXDebugAttributes::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UGasXDebugAttributes, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGasXDebugAttributes, Stamina, COND_None, REPNOTIFY_Always);
}
