// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PP_Capture_Sandbox : ModuleRules
{
	public PP_Capture_Sandbox(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "HeadMountedDisplay",
            "ShaderCore",
            "RHI",
            "RenderCore",
            "Renderer",
            "GameplayTasks",
            "UMG",
            "Slate",
            "SlateCore",
        });

        PrivateDependencyModuleNames.AddRange(new string[]{
            "ImageWrapper",
        });

    }
}
