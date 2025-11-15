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

    // WHY: All AttributeSet instantiation must occur server-side to prevent replication conflicts
    // Clients receive AttributeSet data via replication from the server's spawned instances
    if (!Owner->HasAuthority())
    {
        UE_LOG(LogGASInit, Verbose, TEXT("[CLIENT] Skipping attribute init on client for %s"), *Owner->GetName());
        return;
    }
    
    UE_LOG(LogGASInit, Log, TEXT("[SERVER] ExecuteBootstrap for %s"), *Owner->GetName());

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

        // WHY: Check for existing AttributeSet instance to ensure idempotency
        // Prevents duplicate AttributeSets on PIE restart, Game Feature re-activation, or respawn
        if (HasAttributeSet(ASC, SetClass))
        {
            UE_LOG(LogGASInit, Log, TEXT("[SERVER] AttributeSet %s already present on %s - skipping duplicate"), *SetClass->GetName(), *Owner->GetName());
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
    // WHY: Must prevent duplicate AttributeSets by checking ASC's spawned attributes array
    // Iterates GetSpawnedAttributes() and compares by class to ensure idempotency
    // Required by Requirements.md R14 and Priority Adjustments 1.1
    if (!ASC || !AttributeSetClass)
    {
        return false;
    }

    // WHAT: Iterate all spawned attribute sets and check for exact class match
    const TArray<UAttributeSet*>& SpawnedAttributes = ASC->GetSpawnedAttributes();
    for (UAttributeSet* ExistingSet : SpawnedAttributes)
    {
        if (ExistingSet && ExistingSet->GetClass() == AttributeSetClass)
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

    // WHY: Attribute initialization must only occur on the server to prevent replication conflicts
    // All Init paths (DataTable, GameplayEffect) modify attribute values and must be authority-only
    AActor* Owner = GetOwner();
    if (!Owner || !Owner->HasAuthority())
    {
        UE_LOG(LogGASInit, Verbose, TEXT("Skipping InitializeAttributes on client for %s"), Owner ? *Owner->GetName() : TEXT("null owner"));
        return;
    }

    // If DataTable init is requested, enumerate rows (MVP). Detailed mapping is handled by generator-generated code later.
    if (bUseInitStatsDataTable && AttributeMetadataTable)
    {
        UE_LOG(LogGASInit, Log, TEXT("[SERVER] Applying AttributeMetadataTable initialization for %s"), *Owner->GetName());
        TArray<FName> RowNames = AttributeMetadataTable->GetRowNames();
        for (const FName& RowName : RowNames)
        {
            UE_LOG(LogGASInit, Verbose, TEXT("Attribute metadata row: %s"), *RowName.ToString());
        }
    }

    // If InitGameplayEffect is requested and available, use it
    if (bUseInitGameplayEffect && InitGameplayEffect)
    {
        UE_LOG(LogGASInit, Log, TEXT("[SERVER] Applying InitGameplayEffect to %s"), *Owner->GetName());
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(InitGameplayEffect, 1.0f, Context);
        if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
        {
            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        }
    }

    // Fallback: nothing to do - generated AttributeSet classes may initialize their own defaults in constructors
}
