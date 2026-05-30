# Tracking Auto Setup

**Unreal Engine plugin for one-click Live Link camera tracking setup.**

## Features

- **One-Click Setup** — Configure FreeD or OpenTrack camera tracking instantly
- **Setup Wizard** — 9-step guided configuration
- **AI Chat Assistant** — In-editor AI for tracking/VP questions
- **Camera Picker** — Select existing CineCameraActors from your scene
- **Lens File Creation** — Proper ULensFile with encoder tables, focal length, image center
- **Lens Calibration** — Guided encoder range capture for focus/zoom
- **Sensor Presets** — Super 35, Alexa 35, RED, Full Frame, etc.
- **Custom Toolbar Icon** — SVG icon with custom Slate style
- **Python API** — Toolbar button setup via Python (Epic GDC 2024 pattern)

## Quick Start

### 1. Enable the Plugin
Edit > Plugins > Search "Tracking Auto Setup" > Enable

### 2. Open the Panel
- Click the **camera icon** in the Level Editor toolbar
- Or: Tools > Tracking Auto Setup

### 3. Follow the Wizard
1. Select protocol (FreeD/OpenTrack)
2. Configure network
3. Pick camera (new or existing)
4. Select/create lens file
5. Calibrate lens encoders
6. Set sensor dimensions
7. Configure anchor point
8. Review & apply

## Toolbar Button (Python)

The plugin includes a Python script following Epic's GDC 2024 pattern:

```python
# In UE Output Log:
exec <PluginPath>/Content/Python/setup_toolbar.py
```

This creates a toolbar button with a custom SVG icon that opens the Editor Utility Widget.

### Auto-Register on Startup

Copy `Content/Python/auto_register.py` to your project's `Content/Python/startup.py` to auto-register the button.

## Supported Protocols

| Protocol | Default Port | Notes |
|----------|-------------|-------|
| FreeD | 40000 | UDP, industry standard for camera tracking |
| OpenTrack IO | 55555 (multicast) | Open protocol, multicast/unicast |

## Camera Rig Presets

Generic, Panasonic, Sony, stYpe, Mosys, Ncam

## Lens Configuration

The plugin creates proper `ULensFile` assets with:
- **Encoder tables** — Maps raw encoder values (0-16777215 for 24-bit) to physical values
- **Focal length table** — Normalized FxFy values at calibration points
- **Image center table** — Default center point
- **CineCameraComponent** — Filmback, lens ranges, focus settings
- **LensComponent** — FIZ evaluation mode (UseLiveLink), distortion source, filmback override

### Sensor Presets

| Preset | Dimensions (mm) |
|--------|----------------|
| Super 35mm | 23.76 x 13.365 |
| ARRI Alexa 35 | 24.576 x 13.824 |
| RED V-Raptor | 27.03 x 14.26 |
| Full Frame 35mm | 36.0 x 24.0 |
| ARRI Alexa 65 | 54.12 x 25.59 |
| Super 16mm | 21.0 x 11.8 |
| MFT | 17.8 x 10.0 |

## Requirements

- Unreal Engine 5.0+ (5.6 recommended)
- Live Link plugin enabled
- LiveLinkCamera plugin enabled
- CameraCalibrationCore plugin enabled
- LensComponent plugin enabled

## Installation

1. Clone or download into your project's `Plugins/` folder
2. Enable "Tracking Auto Setup" in Edit > Plugins
3. Use toolbar buttons or Python scripts

## Project Structure

```
TrackingAutoSetup/
├── Content/
│   └── Python/
│       ├── setup_toolbar.py      # Manual toolbar setup
│       ├── auto_register.py      # Auto-register on startup
│       └── README.md             # Python setup guide
├── Resources/
│   └── Icons/
│       └── TrackingAutoSetup.svg # Custom SVG icon
├── Source/
│   ├── TrackingAutoSetup/        # Runtime module
│   │   ├── Public/
│   │   │   ├── TrackingAutoSetupModule.h
│   │   │   ├── TrackingAutoSetupSubsystem.h
│   │   │   ├── TrackingSetupTypes.h
│   │   │   └── LensSetupTypes.h
│   │   └── Private/
│   │       ├── TrackingAutoSetupModule.cpp
│   │       ├── TrackingAutoSetupSubsystem.cpp
│   │       └── LensSetupTypes.cpp
│   └── TrackingAutoSetupEditor/  # Editor module
│       ├── Public/
│       │   ├── TrackingAutoSetupEditorModule.h
│       │   ├── TrackingAutoSetupStyle.h
│       │   ├── Widgets/
│       │   │   ├── STrackingAutoSetupPanel.h
│       │   │   ├── STrackingSetupWizard.h
│       │   │   └── STrackingAIChatPanel.h
│       │   └── EditorUtility/
│       │       ├── TrackingAutoSetupEditorUtilityWidget.h
│       │       └── TrackingAutoSetupEditorUtilityWidgetFactory.h
│       └── Private/
│           ├── TrackingAutoSetupEditorModule.cpp
│           ├── TrackingAutoSetupStyle.cpp
│           ├── Widgets/
│           │   ├── STrackingAutoSetupPanel.cpp
│           │   ├── STrackingSetupWizard.cpp
│           │   └── STrackingAIChatPanel.cpp
│           └── EditorUtility/
│               ├── TrackingAutoSetupEditorUtilityWidget.cpp
│               └── TrackingAutoSetupEditorUtilityWidgetFactory.cpp
├── TrackingAutoSetup.uplugin
└── README.md
```

## License

MIT
