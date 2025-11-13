// Copyright Epic Games, Inc.

#pragma once

#include "Components/ActorComponent.h"
#include "AttributeSet.h"
#include "Engine/DataTable.h"
#include "GameplayEffect.h"
#include "GasXAttributeBootstrapComponent.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;

/**
 * Lightweight helper that spawns Attribute Sets on the owner's Ability System Component
 * so runtimes without save data still boot with sensible defaults.
 *
 * WHY: Provides a server-authoritative, idempotent way to attach generated or
 * hand-authored AttributeSets and initialize them via DataTable or GameplayEffect.
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

	/** Optional metadata DataTable used to initialize attributes (editor-generated). */
	UPROPERTY(EditAnywhere, Category = "GasX|Init")
	UDataTable* AttributeMetadataTable = nullptr;

	/** Optional Init GameplayEffect class to apply on initialization. Editor asset (optional). */
	UPROPERTY(EditAnywhere, Category = "GasX|Init")
	TSubclassOf<UGameplayEffect> InitGameplayEffect;

	/** If true, apply `InitGameplayEffect` when available (server-only). */
	UPROPERTY(EditAnywhere, Category = "GasX|Init")
	bool bUseInitGameplayEffect = false;

	/** If true, populate attribute values from `AttributeMetadataTable` on init. */
	UPROPERTY(EditAnywhere, Category = "GasX|Init")
	bool bUseInitStatsDataTable = false;

private:
	/** Check if the ASC already contains an instance of AttributeSetClass. Prevents duplicates. */
	bool HasAttributeSet(UAbilitySystemComponent* ASC, TSubclassOf<UAttributeSet> AttributeSetClass) const;

	/** Initialize attributes on the ASC when required (server-only). */
	void InitializeAttributes(UAbilitySystemComponent* ASC);
};
