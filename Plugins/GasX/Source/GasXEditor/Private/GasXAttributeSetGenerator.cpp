// Copyright Epic Games, Inc.

#include "GasXAttributeSetGenerator.h"

#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogGasXAttributeSetGenerator, Log, All);

void FGasXAttributeSetGenerator::GenerateForPath(const FString& TargetPath) const
{
	UE_LOG(LogGasXAttributeSetGenerator, Verbose, TEXT("GasX attribute generator placeholder invoked for %s"), *TargetPath);
}
