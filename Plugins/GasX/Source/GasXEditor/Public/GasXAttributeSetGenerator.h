// Copyright Epic Games, Inc.

#pragma once

#include "CoreMinimal.h"

/**
 * Placeholder generator that can later emit Attribute Set classes or data assets from schema definitions.
 */
class GASXEDITOR_API FGasXAttributeSetGenerator
{
public:
	void GenerateForPath(const FString& TargetPath) const;
};
