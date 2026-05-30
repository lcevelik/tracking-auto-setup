# Tracking Auto Setup - Python Setup

## Toolbar Button Setup (Epic Games GDC 2024 Pattern)

This plugin includes Python scripts to create toolbar buttons in the UE editor, following the pattern from Epic's "Accelerating Your In-Editor Workflows with Editor Utilities" GDC 2024 talk.

### Quick Setup

1. **Create the Widget Blueprint:**
   - Content Browser → Right Click → Editor Utilities → Widget Blueprint
   - Select "Tracking Auto Setup Widget" as the parent class
   - Save as `Content/TrackingAutoSetup/EUW_TrackingAutoSetup`

2. **Run the setup script:**
   ```
   # In UE Output Log:
   exec <YourProject>/Plugins/TrackingAutoSetup/Content/Python/setup_toolbar.py
   ```

3. **The toolbar button appears** in the Level Editor toolbar with the custom icon.

### Auto-Register (Startup)

To automatically register the button when the editor starts:

1. Copy `auto_register.py` to your project's `Content/Python/startup.py`
2. Or add `exec <path>/auto_register.py` to your startup script

### Custom Icon

The button uses a custom SVG icon registered via `FTrackingAutoSetupStyle`:
- Style name: `TrackingAutoSetupStyle`
- Brush name: `TrackingAutoSetup.Icon`
- Source: `Resources/Icons/TrackingAutoSetup.svg`

In Python:
```python
self.data.icon = unreal.ScriptSlateIcon(
    "TrackingAutoSetupStyle",
    "TrackingAutoSetup.Icon"
)
```

### Widget Blueprint Structure

The `UTrackingAutoSetupEditorUtilityWidget` C++ class provides:
- `OpenWizard()` - Opens the wizard tab
- `OpenAIChat()` - Opens the AI chat tab
- `QuickSetupFreeD()` - One-click FreeD setup
- `QuickSetupOpenTrack()` - One-click OpenTrack setup

You can override these in Blueprint to customize behavior.

### Script Reference

| Script | Description |
|--------|-------------|
| `setup_toolbar.py` | Manual toolbar button setup |
| `auto_register.py` | Auto-register on editor startup |

### Troubleshooting

**Button doesn't appear:**
- Check Output Log for errors
- Verify the widget blueprint exists at the specified path
- Run `setup_toolbar.py` again

**Icon not showing:**
- Verify `Resources/Icons/TrackingAutoSetup.svg` exists
- Check that the plugin is enabled in Edit > Plugins

**Widget not opening:**
- Check that the widget blueprint parent class is `UTrackingAutoSetupEditorUtilityWidget`
- Verify the blueprint compiles without errors
