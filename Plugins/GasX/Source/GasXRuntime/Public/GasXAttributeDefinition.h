// Copyright Epic Games, Inc.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GasXAttributeDefinition.generated.h"

/**
 * Defines a single GAS Attribute for code generation and data-driven setup.
 * 
 * WHY: Centralizes attribute metadata in a format that both generators (editor)
 * and runtime systems can understand. Supports replication config and designer-safe value ranges.
 */
USTRUCT(BlueprintType)
struct GASXRUNTIME_API FGasXAttributeDefinition
{
	GENERATED_BODY()

public:
	/** Display name of the attribute (e.g., "Health", "Stamina"). Must be valid C++ identifier. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GasX|Definition")
	FString AttributeName;

	/** C++ type of the attribute. MVP supports "float" and "int32" only. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GasX|Definition")
	FString AttributeType = TEXT("float");

	/** Default/base value for this attribute */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GasX|Definition")
	double DefaultValue = 100.0;

	/** Minimum allowed value (used in UI and optional GE clamping) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GasX|Definition")
	double MinValue = 0.0;

	/** Maximum allowed value (used in UI and optional GE clamping) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GasX|Definition")
	double MaxValue = 100.0;

	/** If true, this attribute replicates to all clients */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GasX|Replication")
	bool bReplicates = true;

	/** If true, value changes trigger RepNotify (requires OnRep function in generated class) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GasX|Replication")
	bool bRepNotify = true;

	/** Description for designer reference (not code-generated, for comments only) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GasX|Definition")
	FString Description;
};

/**
 * Collection of attribute definitions that forms the schema for a single AttributeSet.
 * 
 * WHY: Provides a cohesive structure for managing related attributes as a group,
 * enabling modular generation and replication of complex attribute sets.
 */
USTRUCT(BlueprintType)
struct GASXRUNTIME_API FGasXAttributeSetSchema
{
	GENERATED_BODY()

public:
	/** Name of the generated AttributeSet class (e.g., "GasXCharacterAttributes") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GasX|Schema")
	FString AttributeSetClassName;

	/** Module in which to generate the class (e.g., "GasXRuntime") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GasX|Schema")
	FString TargetModule = TEXT("GasXRuntime");

	/** Directory relative to module (e.g., "Public/Attributes") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GasX|Schema")
	FString TargetDirectory = TEXT("Public/Attributes");

	/** List of attribute definitions that comprise this schema */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GasX|Schema")
	TArray<FGasXAttributeDefinition> Attributes;

	/** If true, generator will auto-create an Init Gameplay Effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GasX|CodeGen")
	bool bGenerateInitGameplayEffect = true;

	/** If true, generator will create a data-driven metadata DataTable */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GasX|CodeGen")
	bool bGenerateMetadataTable = true;

	/** Description of this attribute set (for documentation) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GasX|Schema")
	FString Description;
};
