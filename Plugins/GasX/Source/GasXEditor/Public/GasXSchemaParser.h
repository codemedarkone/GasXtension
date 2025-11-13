// Copyright Epic Games, Inc.

#pragma once

#include "CoreMinimal.h"
#include "GasXAttributeDefinition.h"

/**
 * Utility for parsing JSON schema files into FGasXAttributeSetSchema structs.
 * 
 * WHY: Provides editor-only JSON parsing to load schema definitions from disk.
 * Runtime modules must never depend on this.
 */
class GASXEDITOR_API FGasXSchemaParser
{
public:
	/**
	 * Load and parse a JSON schema file.
	 * 
	 * @param JsonFilePath Absolute path to the .json schema file
	 * @param OutSchema The parsed schema structure
	 * @param OutError Error message if parsing fails
	 * @return true if parsing succeeded, false otherwise
	 */
	static bool LoadSchemaFromJson(const FString& JsonFilePath, FGasXAttributeSetSchema& OutSchema, FString& OutError);

private:
	static bool ParseAttributeDefinition(const TSharedPtr<FJsonObject>& JsonObj, FGasXAttributeDefinition& OutAttr);
};
