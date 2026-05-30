# Copyright (c) 2026 Libor Cevelik. All Rights Reserved.
#
# Tracking Auto Setup - Auto-register toolbar button
#
# This script can be placed in your project's Content/Python/startup.py
# to automatically register the toolbar button when the editor starts.
#
# Or run manually from Output Log:
#   exec <path>/auto_register.py

import unreal
import os

# Find the widget blueprint automatically
WIDGET_SEARCH_PATHS = [
    "/Game/TrackingAutoSetup/EUW_TrackingAutoSetup",
    "/Game/TrackingAutoSetup/EditorUtilities/EUW_TrackingAutoSetup",
    "/TrackingAutoSetup/Content/EditorUtilities/EUW_TrackingAutoSetup",
]

def find_widget_blueprint():
    """Search for the Tracking Auto Setup widget blueprint."""
    for path in WIDGET_SEARCH_PATHS:
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if asset is not None:
            return path
    return None


@unreal.uclass()
class TrackingAutoSetup_AutoToolbar(unreal.ToolMenuEntryScript):

    @unreal.ufunction(override=True)
    def execute(self, context):
        widget_path = find_widget_blueprint()

        if widget_path is None:
            # Fallback: open the wizard tab directly
            unreal.log_warning("Tracking Auto Setup: Widget blueprint not found. Use C++ tabs instead.")
            return

        asset = unreal.EditorAssetLibrary.load_asset(widget_path)
        subsystem = unreal.get_editor_subsystem(unreal.EditorUtilitySubsystem)

        widget = subsystem.find_utility_widget_from_blueprint(asset)
        if widget is None:
            widget = subsystem.spawn_and_register_tab(asset)

    def init_as_toolbar_button(self):
        self.data.menu = "LevelEditor.LevelEditorToolBar.PlayToolBar"
        self.data.advanced.entry_type = unreal.MultiBlockType.TOOL_BAR_BUTTON
        self.data.icon = unreal.ScriptSlateIcon(
            "TrackingAutoSetupStyle",
            "TrackingAutoSetup.Icon"
        )


def auto_register():
    """Register toolbar button if plugin is loaded."""
    # Check if the plugin is available
    try:
        tool_menus = unreal.ToolMenus.get()
        menu = tool_menus.find_menu("LevelEditor.LevelEditorToolBar.PlayToolBar")

        if menu is None:
            return

        entry = TrackingAutoSetup_AutoToolbar()
        entry.init_as_toolbar_button()
        entry.init_entry(
            "trackingAutoSetup",
            "trackingAutoSetupAutoEntry",
            "",
            "",
            "Tracking Auto Setup",
            "Opens the Tracking Auto Setup panel"
        )

        toolbar = tool_menus.extend_menu("LevelEditor.LevelEditorToolBar.PlayToolBar")
        toolbar.add_menu_entry_object(entry)
        tool_menus.refresh_all_widgets()

        unreal.log("Tracking Auto Setup: Auto-registered toolbar button")
    except Exception as e:
        unreal.log_warning(f"Tracking Auto Setup: Auto-register failed: {e}")


# Run on import
auto_register()
