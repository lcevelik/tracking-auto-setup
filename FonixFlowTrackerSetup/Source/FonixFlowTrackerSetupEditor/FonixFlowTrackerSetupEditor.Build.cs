// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

using UnrealBuildTool;

public class FonixFlowTrackerSetupEditor : ModuleRules
{
	public FonixFlowTrackerSetupEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"FonixFlowTrackerSetup",
			"LiveLinkInterface",
			"LiveLink",
			"LiveLinkComponents",
			"LiveLinkCamera",
			"CameraCalibrationCore",
			"LensComponent",
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
			"JsonUtilities",
			"CinematicCamera",
			"Projects",
			"Sockets",
			"Networking"
		});
	}
}
