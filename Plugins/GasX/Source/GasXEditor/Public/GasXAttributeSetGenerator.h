// Copyright Epic Games, Inc.

#pragma once

#include "CoreMinimal.h"
#include "GasXAttributeDefinition.h"

/**
 * Generates C++ AttributeSet classes from FGasXAttributeSetSchema definitions.
 * 
 * WHY: Automates the repetitive and error-prone task of writing AttributeSet boilerplate,
 * ensuring consistency with Epic's GAS patterns and replication rules.
 * 
 * NOTE: This is editor-only code. Runtime uses generated classes, never the generator itself.
 */
class GASXEDITOR_API FGasXAttributeSetGenerator
{
public:
	/**
	 * Generate C++ AttributeSet class files from a schema definition.
	 * 
	 * @param Schema The attribute definition schema to code-gen from
	 * @param OutputHeaderPath Full path where the .h file should be written
	 * @param OutputSourcePath Full path where the .cpp file should be written
	 * @return true if generation succeeded; false on error
	 */
	bool GenerateAttributeSet(
		const FGasXAttributeSetSchema& Schema,
		const FString& OutputHeaderPath,
		const FString& OutputSourcePath);

private:
	/**
	 * Validate that a schema is well-formed before generation.
	 * WHY: Fail fast on empty names, duplicates, invalid identifiers, or reserved keywords.
	 */
	bool ValidateSchema(const FGasXAttributeSetSchema& Schema, FString& OutError) const;

	/**
	 * Generate the header file content for an AttributeSet.
	 */
	FString GenerateHeaderContent(const FGasXAttributeSetSchema& Schema) const;

	/**
	 * Generate the implementation file content for an AttributeSet.
	 */
	FString GenerateSourceContent(const FGasXAttributeSetSchema& Schema) const;

	/**
	 * Generate a single attribute property declaration with replication setup.
	 */
	FString GenerateAttributeProperty(const FGasXAttributeDefinition& Attribute) const;

	/**
	 * Generate OnRep function declaration for a single attribute.
	 */
	FString GenerateOnRepDeclaration(const FGasXAttributeDefinition& Attribute) const;

	/**
	 * Generate accessor macros for a single attribute.
	 */
	FString GenerateAccessors(const FGasXAttributeDefinition& Attribute, const FString& ClassName) const;

	/**
	 * Generate OnRep implementation that broadcasts the attribute change.
	 */
	FString GenerateOnRepImplementation(const FGasXAttributeDefinition& Attribute, const FString& ClassName) const;

	/**
	 * Generate replication setup for all attributes.
	 */
	FString GenerateReplicationSetup(const FGasXAttributeSetSchema& Schema) const;

	/**
	 * Ensure the output directory exists.
	 */
	bool EnsureOutputDirectory(const FString& DirectoryPath) const;

	/**
	 * Check if a name is a reserved C++ keyword.
	 */
	bool IsReservedKeyword(const FString& Name) const;

	/**
	 * Check if a name is a valid C++ identifier.
	 */
	bool IsValidIdentifier(const FString& Name) const;
};
