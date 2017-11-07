// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class PP_Capture_SandboxTarget : TargetRules
{
	public PP_Capture_SandboxTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.Add("PP_Capture_Sandbox");
	}
}
