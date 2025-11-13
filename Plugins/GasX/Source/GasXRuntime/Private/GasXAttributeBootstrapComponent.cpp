// Copyright Epic Games, Inc.

#include "GasXAttributeBootstrapComponent.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogGasXAttributeBootstrap, Log, All);

UGasXAttributeBootstrapComponent::UGasXAttributeBootstrapComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGasXAttributeBootstrapComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AttributeSetTypes.Num() == 0)
	{
		UE_LOG(LogGasXAttributeBootstrap, Verbose, TEXT("No attribute sets configured on %s"), *GetName());
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		UE_LOG(LogGasXAttributeBootstrap, Warning, TEXT("Bootstrap component has no owner."));
		return;
	}

	UAbilitySystemComponent* AbilitySystem = Cast<UAbilitySystemComponent>(OwnerActor->GetComponentByClass(UAbilitySystemComponent::StaticClass()));
	if (!AbilitySystem)
	{
		UE_LOG(LogGasXAttributeBootstrap, Warning, TEXT("Actor %s does not expose an AbilitySystemComponent."), *OwnerActor->GetName());
		return;
	}

	for (const TSoftClassPtr<UAttributeSet>& AttributeSetType : AttributeSetTypes)
	{
		if (!AttributeSetType.IsValid())
		{
			continue;
		}

		UClass* AttributeClass = AttributeSetType.Get();
		if (!AttributeClass)
		{
			AttributeClass = AttributeSetType.LoadSynchronous();
		}

		if (!AttributeClass)
		{
			UE_LOG(LogGasXAttributeBootstrap, Warning, TEXT("Failed to load attribute set from %s"), *AttributeSetType.ToString());
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(AbilitySystem->GetOwner(), AttributeClass);
		if (!NewSet)
		{
			continue;
		}

		AbilitySystem->AddAttributeSetSubobject(NewSet);
		UE_LOG(LogGasXAttributeBootstrap, Display, TEXT("Spawned attribute set %s on %s"), *AttributeClass->GetName(), *OwnerActor->GetName());
	}
}
