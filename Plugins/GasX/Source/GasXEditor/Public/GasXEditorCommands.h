// Copyright Epic Games, Inc.

#pragma once

#include "CoreMinimal.h"

/**
 * Editor console commands for GasX AttributeSet generation.
 * 
 * Usage in Editor:
 * GasX.GenerateAttributeSet <SchemaJsonPath>
 * 
 * Example:
 * GasX.GenerateAttributeSet "D:/Documents/GasXtension/Plugins/GasX/Schemas/Attributes/PlayerCoreAttributes.json"
 */
class GASXEDITOR_API FGasXEditorCommands
{
public:
	static void RegisterCommands();
	static void UnregisterCommands();

private:
	static void GenerateAttributeSetCommand(const TArray<FString>& Args);
	
	static FAutoConsoleCommand GenerateAttributeSetCmd;
};
