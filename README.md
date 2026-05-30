# Tracking Auto Setup

**Unreal Engine plugin for one-click Live Link camera tracking setup.**

## What It Does

Automates the tedious multi-panel Live Link camera tracking configuration into a single click. Supports FreeD and OpenTrack IO protocols.

### Features

- **One-Click Setup** — Configure FreeD or OpenTrack camera tracking instantly
- **Setup Wizard** — Step-by-step guided configuration
- **AI Chat Assistant** — In-editor AI for tracking/VP questions
- **Camera Picker** — Select existing CineCameraActors from your scene
- **Lens Calibration** — Guided encoder range capture for focus/zoom
- **Multi-Camera Support** — Configure multiple tracked cameras
- **Rig Presets** — Panasonic, Sony, stYpe, Mosys, Ncam defaults

### Auto-configures:
1. **Live Link Source** — FreeD/OpenTrack IO with correct port and settings
2. **Up Vector** — Z-up orientation for FreeD
3. **Camera Component** — Assigns tracking to CineCameraActor
4. **Anchor Point** — Creates and attaches tracking origin
5. **Lens File** — Generates or configures lens calibration
6. **Virtual Camera** — Wires lens and tracking into Virtual Camera system

## Quick Start

### One-Click (Editor Toolbar)

After enabling the plugin, use the toolbar buttons:
- **Tracking Wizard** — Full guided setup
- **Tracking AI** — AI chat assistant
- **Quick Setup** → FreeD/OpenTrack — Instant setup with defaults

### Setup Wizard

The wizard walks you through:
1. **Protocol Selection** — FreeD or OpenTrack IO + rig preset
2. **Network Configuration** — IP address, port, multicast settings
3. **Camera Selection** — Create new or pick existing CineCameraActor
4. **Lens Calibration** — Rotate lenses to min/max positions to capture encoder ranges
5. **Anchor Point** — Configure tracking origin position
6. **Review & Apply** — Confirm all settings

### AI Chat Assistant

Ask questions about:
- FreeD protocol configuration
- OpenTrack IO setup
- Lens calibration best practices
- Camera tracking workflows
- Virtual Production tips

Configure API key in project config:
```ini
[/Script/TrackingAutoSetup.TrackingAutoSetupSubsystem]
AIAPIKey=your-api-key-here
AIEndpoint=https://openrouter.ai/api/v1/chat/completions
AIModel=anthropic/claude-sonnet-4
```

Or set environment variable: `TRACKING_AI_API_KEY`

### Blueprint

```cpp
// FreeD quick setup
FTrackingSetupResult Result = UTrackingAutoSetupSubsystem::SetupFreeDCamera(
    WorldContextObject,
    "192.168.1.100",  // IP
    40000,            // Port
    "MainCamera"      // Name
);

// OpenTrack quick setup
FTrackingSetupResult Result = UTrackingAutoSetupSubsystem::SetupOpenTrackCamera(
    WorldContextObject,
    1,               // Source number
    "MainCamera"     // Name
);

// Use existing camera
FCameraSetupConfig CamConfig;
CamConfig.bCreateNewCamera = false;
CamConfig.ExistingCamera = MyExistingCameraActor;
```

## Supported Protocols

| Protocol | Default Port | Notes |
|----------|-------------|-------|
| FreeD | 40000 | UDP, industry standard for camera tracking |
| OpenTrack IO | 55555 (multicast) | Open protocol, multicast/unicast |

## Camera Rig Presets

- **Generic** — Default settings
- **Panasonic** — Panasonic camera encoder defaults
- **Sony** — Sony camera encoder defaults
- **stYpe** — stYpe RedSpy/FreeD defaults
- **Mosys** — Mosys camera tracking defaults
- **Ncam** — Ncam AR tracking defaults

## Lens Calibration

The wizard guides you through encoder range calibration:

1. **Focus Distance** — Rotate focus ring to MIN, capture. Rotate to MAX, capture.
2. **Focal Length** — Rotate zoom ring to MIN, capture. Rotate to MAX, capture.

This maps raw encoder values (0-16777215 for 24-bit) to physical lens values.

## Requirements

- Unreal Engine 5.0+ (5.6 recommended)
- Live Link plugin enabled
- LiveLinkCamera plugin enabled
- CameraCalibrationCore plugin enabled

## Installation

1. Clone or download into your project's `Plugins/` folder
2. Enable "Tracking Auto Setup" in Edit > Plugins
3. Use toolbar buttons or Blueprint functions

## License

MIT
