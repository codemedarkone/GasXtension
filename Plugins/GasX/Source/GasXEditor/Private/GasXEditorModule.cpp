// Copyright Epic Games, Inc.

#include "Modules/ModuleManager.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogGasXEditor, Log, All);

class FGasXEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UE_LOG(LogGasXEditor, Log, TEXT("GasXEditor module loaded"));
	}

	virtual void ShutdownModule() override
	{
		UE_LOG(LogGasXEditor, Log, TEXT("GasXEditor module unloaded"));
	}
};

IMPLEMENT_MODULE(FGasXEditorModule, GasXEditor)
