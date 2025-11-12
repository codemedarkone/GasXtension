using UnrealBuildTool;

public class GasXEditor : ModuleRules
{
    public GasXEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine",
            "GameplayAbilities", "GameplayTags",
            "GameFeatures", "ModularGameplay",
            "UnrealEd", "AssetTools", "Slate", "SlateCore"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "EditorFramework", "Kismet", "InputCore"
        });
    }
}
