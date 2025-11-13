// Copyright Epic Games, Inc.

#include "Modules/ModuleManager.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogGasXRuntime, Log, All);

class FGasXRuntimeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UE_LOG(LogGasXRuntime, Log, TEXT("GasXRuntime module starting up"));
	}

	virtual void ShutdownModule() override
	{
		UE_LOG(LogGasXRuntime, Log, TEXT("GasXRuntime module shutting down"));
	}
};

IMPLEMENT_MODULE(FGasXRuntimeModule, GasXRuntime)
