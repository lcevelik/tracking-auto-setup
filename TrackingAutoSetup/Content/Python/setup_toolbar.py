# Copyright (c) 2026 Libor Cevelik. All Rights Reserved.
#
# Tracking Auto Setup - Toolbar Button Setup Script
# Based on Epic Games Editor Utilities GDC 2024 pattern
#
# This script creates a toolbar button in the UE editor that opens
# the Tracking Auto Setup Editor Utility Widget.
#
# Usage:
#   1. Run this script from the Output Log: exec <path>/setup_toolbar.py
#   2. Or place in your project's Content/Python/ folder
#   3. The button appears in the Level Editor toolbar
#
# The widget blueprint must exist at the path specified below.
# Create it via: Content Browser > Right Click > Editor Utilities > Tracking Auto Setup Widget

import unreal

# Configuration
MENU_OWNER = "trackingAutoSetup"
TOOLBAR_MENU = "LevelEditor.LevelEditorToolBar.PlayToolBar"
WIDGET_BLUEPRINT_PATH = "/Game/TrackingAutoSetup/EUW_TrackingAutoSetup"
ENTRY_NAME = "trackingAutoSetupEntry"

@unreal.uclass()
class TrackingAutoSetup_ToolbarButton(unreal.ToolMenuEntryScript):
    """Toolbar button that opens the Tracking Auto Setup Editor Utility Widget."""

    @unreal.ufunction(override=True)
    def execute(self, context):
        """Called when the toolbar button is clicked."""
        registry = unreal.AssetRegistryHelpers.get_asset_registry()
        asset = unreal.EditorAssetLibrary.load_asset(WIDGET_BLUEPRINT_PATH)

        if asset is None:
            unreal.log_warning(
                f"Tracking Auto Setup: Widget not found at {WIDGET_BLUEPRINT_PATH}. "
                f"Create it via Content Browser > Right Click > Editor Utilities > Tracking Auto Setup Widget"
            )
            return

        subsystem = unreal.get_editor_subsystem(unreal.EditorUtilitySubsystem)

        # Find existing tab or spawn new one (same pattern as Epic example)
        widget = subsystem.find_utility_widget_from_blueprint(asset)
        if widget is None:
            widget = subsystem.spawn_and_register_tab(asset)

        unreal.log("Tracking Auto Setup: Panel opened")

    def init_as_toolbar_button(self):
        """Configure as a toolbar button with custom icon."""
        self.data.menu = TOOLBAR_MENU
        self.data.advanced.entry_type = unreal.MultiBlockType.TOOL_BAR_BUTTON

        # Custom icon using ScriptSlateIcon
        # Format: ScriptSlateIcon(StyleSetName, BrushName)
        # Our custom style: "TrackingAutoSetupStyle", "TrackingAutoSetup.Icon"
        self.data.icon = unreal.ScriptSlateIcon(
            "TrackingAutoSetupStyle",
            "TrackingAutoSetup.Icon"
        )


def setup_toolbar():
    """Register the toolbar button."""
    tool_menus = unreal.ToolMenus.get()

    entry = TrackingAutoSetup_ToolbarButton()
    entry.init_as_toolbar_button()
    entry.init_entry(
        MENU_OWNER,
        ENTRY_NAME,
        "",  # No label (icon only)
        "",  # No tooltip (uses icon hover)
        "Tracking Auto Setup",
        "Opens the Tracking Auto Setup panel with wizard, AI chat, and quick setup"
    )

    toolbar = tool_menus.extend_menu(TOOLBAR_MENU)
    toolbar.add_menu_entry_object(entry)
    tool_menus.refresh_all_widgets()

    unreal.log("Tracking Auto Setup: Toolbar button registered")


def remove_toolbar():
    """Remove the toolbar button."""
    tool_menus = unreal.ToolMenus.get()
    menu = tool_menus.find_menu(TOOLBAR_MENU)

    if menu:
        menu.remove_entry(MENU_OWNER, ENTRY_NAME)
        tool_menus.refresh_all_widgets()
        unreal.log("Tracking Auto Setup: Toolbar button removed")


# Auto-run when executed
setup_toolbar()
