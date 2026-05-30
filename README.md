# Tracking Auto Setup

**Unreal Engine plugin for one-click Live Link camera tracking setup.**

## What It Does

Automates the tedious multi-panel Live Link camera tracking configuration into a single click. Supports FreeD and OpenTrack IO protocols.

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
- **Setup FreeD Camera** — Instant FreeD setup with defaults
- **Setup OpenTrack Camera** — Instant OpenTrack IO setup

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
```

### Full Configuration

```cpp
FTrackingConnectionSettings ConnSettings;
ConnSettings.Protocol = ETrackingProtocol::FreeD;
ConnSettings.IPAddress = "192.168.1.100";
ConnSettings.FreeDPort = 40000;
ConnSettings.SubjectName = "Camera1";

FCameraSetupConfig CamConfig;
CamConfig.CameraName = "TrackedCamera";
CamConfig.bCreateNewCamera = true;
CamConfig.bCreateAnchorPoint = true;
CamConfig.bAutoGenerateLensFile = true;
CamConfig.bEnableVirtualCamera = false;

FTrackingSetupResult Result = UTrackingAutoSetupSubsystem::SetupTracking(
    WorldContextObject, ConnSettings, CamConfig
);
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
