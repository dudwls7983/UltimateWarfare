// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class Ultimate_WarfareEditorTarget : TargetRules
{
	public Ultimate_WarfareEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		ExtraModuleNames.Add("Ultimate_Warfare");
	}
}
