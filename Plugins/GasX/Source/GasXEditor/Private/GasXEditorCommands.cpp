// Copyright Epic Games, Inc.

#include "GasXEditorCommands.h"
#include "GasXAttributeSetGenerator.h"
#include "GasXSchemaParser.h"
#include "Misc/Paths.h"

FAutoConsoleCommand FGasXEditorCommands::GenerateAttributeSetCmd(
	TEXT("GasX.GenerateAttributeSet"),
	TEXT("Generate an AttributeSet from a JSON schema file. Usage: GasX.GenerateAttributeSet <path-to-json>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FGasXEditorCommands::GenerateAttributeSetCommand)
);

void FGasXEditorCommands::RegisterCommands()
{
	// Commands are auto-registered via FAutoConsoleCommand
	UE_LOG(LogTemp, Log, TEXT("GasX Editor Commands registered"));
}

void FGasXEditorCommands::UnregisterCommands()
{
	// Auto-cleanup
}

void FGasXEditorCommands::GenerateAttributeSetCommand(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		UE_LOG(LogTemp, Error, TEXT("Usage: GasX.GenerateAttributeSet <path-to-json-schema>"));
		return;
	}

	FString JsonPath = Args[0];
	
	// WHY: Remove surrounding quotes if present and handle both absolute and relative paths
	JsonPath = JsonPath.TrimQuotes();
	
	// If it's not an absolute path, make it relative to the project directory
	if (!FPaths::IsRelative(JsonPath))
	{
		// Already absolute - use as-is but normalize separators
		JsonPath = FPaths::ConvertRelativePathToFull(JsonPath);
	}
	else
	{
		// Relative path - resolve from project directory
		JsonPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), JsonPath);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Checking schema file at: %s"), *JsonPath);
	
	if (!FPaths::FileExists(JsonPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Schema file not found: %s"), *JsonPath);
		UE_LOG(LogTemp, Error, TEXT("Current working directory: %s"), *FPaths::ProjectDir());
		return;
	}

	FGasXAttributeSetSchema Schema;
	FString ParseError;
	if (!FGasXSchemaParser::LoadSchemaFromJson(JsonPath, Schema, ParseError))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse schema: %s"), *ParseError);
		return;
	}

	// Determine output paths based on schema
	FString PluginDir = FPaths::ProjectPluginsDir() / TEXT("GasX");
	FString ModuleDir = PluginDir / TEXT("Source") / Schema.TargetModule;
	FString OutputDir = ModuleDir / Schema.TargetDirectory;
	
	FString HeaderPath = OutputDir / (Schema.AttributeSetClassName + TEXT(".h"));
	FString SourcePath = FPaths::Combine(ModuleDir, TEXT("Private"), TEXT("Attributes"), Schema.AttributeSetClassName + TEXT(".cpp"));

	FGasXAttributeSetGenerator Generator;
	if (Generator.GenerateAttributeSet(Schema, HeaderPath, SourcePath))
	{
		UE_LOG(LogTemp, Display, TEXT("Successfully generated AttributeSet: %s"), *Schema.AttributeSetClassName);
		UE_LOG(LogTemp, Display, TEXT("  Header: %s"), *HeaderPath);
		UE_LOG(LogTemp, Display, TEXT("  Source: %s"), *SourcePath);
		UE_LOG(LogTemp, Display, TEXT("Rebuild the project to compile the generated code."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to generate AttributeSet"));
	}
}
