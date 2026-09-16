// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WcwMemoryMonitorModule : ModuleRules
{
	public WcwMemoryMonitorModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bAllowConfidentialPlatformDefines = true;

        PublicIncludePaths.AddRange(
			new string[] {
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
            }
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Engine",
                "Slate",
		        "SlateCore",
				"RHI",
			  }
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
	            "RenderCore",
	            "RHI",
				"MeshDescription",
				"StaticMeshDescription",
                "DeveloperSettings",
            }
            );
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        if (Target.bBuildEditor)
        {
            PublicDependencyModuleNames.Add("UnrealEd");
        }
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicSystemLibraries.AddRange(new string[] { "DXGI.lib", "Pdh.lib" });
        }
    }
}
