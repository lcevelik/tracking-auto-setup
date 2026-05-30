// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

using UnrealBuildTool;

public class TrackingAutoSetupEditor : ModuleRules
{
	public TrackingAutoSetupEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"TrackingAutoSetup",
			"LiveLinkInterface",
			"LiveLink",
			"LiveLinkComponents",
			"LiveLinkCamera",
			"CameraCalibrationCore",
			"Slate",
			"SlateCore",
			"UnrealEd",
			"LevelEditor",
			"ToolMenus",
			"EditorStyle",
			"PropertyEditor",
			"WorkspaceMenuStructure",
			"InputCore",
			"Http",
			"Json",
			"JsonUtilities"
		});
	}
}
