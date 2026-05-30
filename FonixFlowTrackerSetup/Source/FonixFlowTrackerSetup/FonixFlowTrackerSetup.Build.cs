// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

using UnrealBuildTool;

public class FonixFlowTrackerSetup : ModuleRules
{
	public FonixFlowTrackerSetup(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"LiveLinkInterface",
			"LiveLink",
			"LiveLinkComponents",
			"LiveLinkCamera",
			"CameraCalibrationCore",
			"LensComponent",
			"VirtualCameraCore",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"InputCore",
			"EditorStyle",
			"ToolMenus",
			"UnrealEd",
			"LevelEditor"
		});
	}
}
