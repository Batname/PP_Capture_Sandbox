// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class PP_Capture_SandboxEditorTarget : TargetRules
{
	public PP_Capture_SandboxEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		ExtraModuleNames.Add("PP_Capture_Sandbox");
	}
}
