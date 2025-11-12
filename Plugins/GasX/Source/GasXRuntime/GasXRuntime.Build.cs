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
    }
}
