// Copyright Epic Games, Inc.

#pragma once

#include "Components/ActorComponent.h"
#include "AttributeSet.h"
#include "GasXAttributeBootstrapComponent.generated.h"

class UAttributeSet;

/**
 * Lightweight helper that spawns Attribute Sets on the owner's Ability System Component
 * so runtimes without save data still boot with sensible defaults.
 */
UCLASS(ClassGroup = (GasX), meta = (BlueprintSpawnableComponent))
class GASXRUNTIME_API UGasXAttributeBootstrapComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGasXAttributeBootstrapComponent();

protected:
	virtual void BeginPlay() override;

	/** Attribute sets to instantiate on BeginPlay if the owner exposes an ASC. */
	UPROPERTY(EditAnywhere, Category = "GasX")
	TArray<TSoftClassPtr<UAttributeSet>> AttributeSetTypes;
};
