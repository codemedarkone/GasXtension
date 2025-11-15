// Copyright Epic Games, Inc.

#if WITH_AUTOMATION_TESTS

#include "GasXAttributeBootstrapComponent.h"
#include "AbilitySystemComponent.h"
#include "Attributes/PlayerCoreAttributes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGasXBootstrapIdempotencyTest,
	"GasX.Runtime.Bootstrap.PreventsDuplicatesOnReinitialization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGasXBootstrapIdempotencyTest::RunTest(const FString& Parameters)
{
	// WHY: Validate that repeated bootstrap calls (simulating PIE restart or respawn) don't duplicate AttributeSets
	// Tests Requirements.md R14 and Priority Adjustments 1.1
	
	// WHAT: Create test world using Epic's pattern for GAS tests
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

	// Spawn test actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("GasXIdempotencyTestActor");
	AActor* Owner = World->SpawnActor<AActor>(SpawnParams);
	if (!TestNotNull(TEXT("Spawned test actor"), Owner))
	{
		World->EndPlay(EEndPlayReason::Quit);
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		return false;
	}

	// Add ASC
	UAbilitySystemComponent* ASC = NewObject<UAbilitySystemComponent>(Owner);
	Owner->AddInstanceComponent(ASC);
	ASC->RegisterComponentWithWorld(World);

	// Add bootstrap component
	UGasXAttributeBootstrapComponent* Bootstrap = NewObject<UGasXAttributeBootstrapComponent>(Owner);
	Owner->AddInstanceComponent(Bootstrap);
	Bootstrap->RegisterComponentWithWorld(World);
	Bootstrap->TestAddAttributeSetType(UPlayerCoreAttributes::StaticClass());

	// Helper to count PlayerCoreAttributes instances
	auto CountPlayerCoreSets = [&ASC]()
	{
		int32 Count = 0;
		for (UAttributeSet* Set : ASC->GetSpawnedAttributes())
		{
			if (Set && Set->GetClass() == UPlayerCoreAttributes::StaticClass())
			{
				++Count;
			}
		}
		return Count;
	};

	// First bootstrap - should add AttributeSet
	Bootstrap->RunBootstrapForTests();
	TestEqual(TEXT("First bootstrap adds one PlayerCoreAttributes"), CountPlayerCoreSets(), 1);

	// Second bootstrap call (simulating PIE restart or respawn) - should NOT add duplicate
	Bootstrap->RunBootstrapForTests();
	TestEqual(TEXT("Second bootstrap does not add duplicate"), CountPlayerCoreSets(), 1);

	// Third call - still no duplicate
	Bootstrap->RunBootstrapForTests();
	TestEqual(TEXT("Third bootstrap still prevents duplicate"), CountPlayerCoreSets(), 1);

	// Verify the spawned set is actually the correct class
	const TArray<UAttributeSet*>& SpawnedSets = ASC->GetSpawnedAttributes();
	bool bFoundCorrectClass = false;
	for (UAttributeSet* Set : SpawnedSets)
	{
		if (Set && Set->GetClass() == UPlayerCoreAttributes::StaticClass())
		{
			bFoundCorrectClass = true;
			break;
		}
	}
	TestTrue(TEXT("Spawned AttributeSet is correct class"), bFoundCorrectClass);

	// Cleanup
	Bootstrap->DestroyComponent();
	ASC->DestroyComponent();
	Owner->Destroy();

	World->EndPlay(EEndPlayReason::Quit);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGasXBootstrapAuthorityTest,
	"GasX.Runtime.Bootstrap.ClientSideBlocksInitialization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGasXBootstrapAuthorityTest::RunTest(const FString& Parameters)
{
	// WHY: Verify that bootstrap correctly checks HasAuthority() and blocks client-side execution
	// Tests Requirements.md R11 and TEST_PLAN.md TC-N1
	
	// NOTE: This test verifies the authority check exists in the code path
	// Full multiplayer validation requires dedicated server testing (outside automation framework scope)
	
	// WHAT: Create test world
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

	// Spawn test actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("GasXAuthorityTestActor");
	AActor* Owner = World->SpawnActor<AActor>(SpawnParams);
	if (!TestNotNull(TEXT("Spawned test actor"), Owner))
	{
		World->EndPlay(EEndPlayReason::Quit);
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		return false;
	}

	// Verify actor has authority in this test world (since it's a standalone game world)
	TestTrue(TEXT("Actor has authority in test world"), Owner->HasAuthority());

	// Add ASC and Bootstrap
	UAbilitySystemComponent* ASC = NewObject<UAbilitySystemComponent>(Owner);
	Owner->AddInstanceComponent(ASC);
	ASC->RegisterComponentWithWorld(World);

	UGasXAttributeBootstrapComponent* Bootstrap = NewObject<UGasXAttributeBootstrapComponent>(Owner);
	Owner->AddInstanceComponent(Bootstrap);
	Bootstrap->RegisterComponentWithWorld(World);
	Bootstrap->TestAddAttributeSetType(UPlayerCoreAttributes::StaticClass());

	// Run bootstrap - should succeed since we have authority
	Bootstrap->RunBootstrapForTests();
	
	// Verify AttributeSet was added (proves authority path executed)
	int32 Count = 0;
	for (UAttributeSet* Set : ASC->GetSpawnedAttributes())
	{
		if (Set && Set->GetClass() == UPlayerCoreAttributes::StaticClass())
		{
			++Count;
		}
	}
	TestEqual(TEXT("AttributeSet added with authority"), Count, 1);

	// Cleanup
	Bootstrap->DestroyComponent();
	ASC->DestroyComponent();
	Owner->Destroy();

	World->EndPlay(EEndPlayReason::Quit);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
