#if WITH_AUTOMATION_TESTS

#include "GasXAttributeBootstrapComponent.h"
#include "AbilitySystemComponent.h"
#include "Attributes/PlayerCoreAttributes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGasXBootstrapAddsPlayerCoreAttributesTest,
	"GasX.Runtime.Bootstrap.AddsPlayerCoreAttributes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGasXBootstrapAddsPlayerCoreAttributesTest::RunTest(const FString& Parameters)
{
	// WHY: Follow Epic's pattern for GAS tests - use CreateNewWorldContext
	// Creates a minimal game world for testing without full initialization overhead
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Created transient world"), World))
	{
		return false;
	}

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	FURL URL;
	World->InitializeActorsForPlay(URL);
	World->BeginPlay();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("GasXBootstrapTestActor");
	AActor* Owner = World->SpawnActor<AActor>(SpawnParams);
	if (!TestNotNull(TEXT("Spawned test actor"), Owner))
	{
		World->DestroyWorld(false);
		World->RemoveFromRoot();
		return false;
	}

	UAbilitySystemComponent* ASC = NewObject<UAbilitySystemComponent>(Owner);
	Owner->AddInstanceComponent(ASC);
	ASC->RegisterComponentWithWorld(World);

	UGasXAttributeBootstrapComponent* Bootstrap = NewObject<UGasXAttributeBootstrapComponent>(Owner);
	Owner->AddInstanceComponent(Bootstrap);
	Bootstrap->RegisterComponentWithWorld(World);
	Bootstrap->TestAddAttributeSetType(UPlayerCoreAttributes::StaticClass());

	auto CountPlayerCoreSets = [&ASC]()
	{
		int32 Count = 0;
		for (UAttributeSet* Set : ASC->GetSpawnedAttributes())
		{
			if (Set && Set->IsA(UPlayerCoreAttributes::StaticClass()))
			{
				++Count;
			}
		}
		return Count;
	};

	Bootstrap->RunBootstrapForTests();
	TestEqual(TEXT("Bootstrap adds PlayerCoreAttributes once"), CountPlayerCoreSets(), 1);

	Bootstrap->RunBootstrapForTests();
	TestEqual(TEXT("Bootstrap prevents duplicate PlayerCoreAttributes"), CountPlayerCoreSets(), 1);

	Bootstrap->DestroyComponent();
	ASC->DestroyComponent();
	Owner->Destroy();

	World->EndPlay(EEndPlayReason::Quit);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
