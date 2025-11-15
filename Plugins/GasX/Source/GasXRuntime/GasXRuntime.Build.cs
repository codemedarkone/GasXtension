using UnrealBuildTool;

public class GasXRuntime : ModuleRules
{
    public GasXRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine",
            "GameplayAbilities", "GameplayTags",
            "GameFeatures", "ModularGameplay"
        });

        // WHY: Enable automation tests in Development builds for editor testing
        if (Target.Configuration != UnrealTargetConfiguration.Shipping)
        {
            PublicDefinitions.Add("WITH_AUTOMATION_TESTS=1");
        }
    }
}
