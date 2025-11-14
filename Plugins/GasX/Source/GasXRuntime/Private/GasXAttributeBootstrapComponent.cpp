// Copyright Epic Games, Inc.

#include "GasXAttributeBootstrapComponent.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GameFramework/Actor.h"
#include "GameplayEffect.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "Attributes/GasXDebugAttributes.h"

DEFINE_LOG_CATEGORY_STATIC(LogGASInit, Log, All);

UGasXAttributeBootstrapComponent::UGasXAttributeBootstrapComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGasXAttributeBootstrapComponent::BeginPlay()
{
    Super::BeginPlay();
    ExecuteBootstrap();
}

void UGasXAttributeBootstrapComponent::ExecuteBootstrap()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogGASInit, Warning, TEXT("GasXAttributeBootstrapComponent has no owner."));
        return;
    }

    // Server-only initialization
    if (!Owner->HasAuthority())
    {
        UE_LOG(LogGASInit, Verbose, TEXT("Skipping attribute init on client for %s"), *Owner->GetName());
        return;
    }

    UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
    if (!ASC)
    {
        UE_LOG(LogGASInit, Warning, TEXT("No AbilitySystemComponent found on %s. Skipping attribute initialization."), *Owner->GetName());
        return;
    }

    // Instantiate attribute sets if missing
    for (const TSoftClassPtr<UAttributeSet>& SetClassPtr : AttributeSetTypes)
    {
        if (!SetClassPtr.IsValid())
        {
            UE_LOG(LogGASInit, Warning, TEXT("AttributeSet soft class not valid for owner %s"), *Owner->GetName());
            continue;
        }

        UClass* SetClass = SetClassPtr.Get();
        if (!SetClass)
        {
            UE_LOG(LogGASInit, Warning, TEXT("Unable to load AttributeSet class for %s"), *Owner->GetName());
            continue;
        }

        if (HasAttributeSet(ASC, SetClass))
        {
            UE_LOG(LogGASInit, Verbose, TEXT("AttributeSet %s already present on %s"), *SetClass->GetName(), *Owner->GetName());
            continue;
        }

        UAttributeSet* NewSet = NewObject<UAttributeSet>(ASC, SetClass);
        if (NewSet)
        {
            ASC->AddAttributeSetSubobject(NewSet);
            UE_LOG(LogGASInit, Log, TEXT("Added AttributeSet %s to %s"), *SetClass->GetName(), *Owner->GetName());

            // MVP helper: if this is the debug attribute set, apply simple default values now.
            if (NewSet->IsA(UGasXDebugAttributes::StaticClass()))
            {
                UGasXDebugAttributes* Debug = Cast<UGasXDebugAttributes>(NewSet);
                if (Debug)
                {
                    Debug->SetHealthValue(Debug->GetHealth());
                    Debug->SetStaminaValue(Debug->GetStamina());
                    UE_LOG(LogGASInit, Log, TEXT("Initialized debug attributes on %s"), *Owner->GetName());
                }
            }
        }
    }

    // Initialize attributes according to selected path
    InitializeAttributes(ASC);
}

bool UGasXAttributeBootstrapComponent::HasAttributeSet(UAbilitySystemComponent* ASC, TSubclassOf<UAttributeSet> AttributeSetClass) const
{
    if (!ASC || !AttributeSetClass)
    {
        return false;
    }

    const TArray<UAttributeSet*>& Spawned = ASC->GetSpawnedAttributes();
    for (UAttributeSet* Set : Spawned)
    {
        if (Set && Set->IsA(AttributeSetClass))
        {
            return true;
        }
    }

    return false;
}

void UGasXAttributeBootstrapComponent::InitializeAttributes(UAbilitySystemComponent* ASC)
{
    if (!ASC)
    {
        return;
    }

    // If DataTable init is requested, enumerate rows (MVP). Detailed mapping is handled by generator-generated code later.
    if (bUseInitStatsDataTable && AttributeMetadataTable)
    {
        UE_LOG(LogGASInit, Log, TEXT("Applying AttributeMetadataTable initialization (MVP - enumerating rows)."));
        TArray<FName> RowNames = AttributeMetadataTable->GetRowNames();
        for (const FName& RowName : RowNames)
        {
            UE_LOG(LogGASInit, Verbose, TEXT("Attribute metadata row: %s"), *RowName.ToString());
        }
    }

    // If InitGameplayEffect is requested and available, use it
    if (bUseInitGameplayEffect && InitGameplayEffect)
    {
        UE_LOG(LogGASInit, Log, TEXT("Applying InitGameplayEffect to ASC."));
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(InitGameplayEffect, 1.0f, Context);
        if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
        {
            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        }
    }

    // Fallback: nothing to do - generated AttributeSet classes may initialize their own defaults in constructors
}
