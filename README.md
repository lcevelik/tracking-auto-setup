# FonixFlow Tracker Setup

**Unreal Engine plugin — one-click Live Link camera tracking auto-setup for FreeD and OpenTrack protocols.**

## Quick Start

1. **Copy** `FonixFlowTrackerSetup/` into your project's `Plugins/` folder
2. **Enable** — Edit > Plugins > "FonixFlow Tracker Setup" > Enable
3. **Click** the FonixFlow Tracker button in the Level Editor toolbar
4. **Select camera** → choose Prime or Zoom → click **SETUP NOW**
5. **Calibrate** — rotate lens to min/max, capture, click APPLY CALIBRATION

## What It Does

Automates the entire Live Link camera tracking pipeline in one panel:

- **Camera Selection** — pick any CineCameraActor in the level
- **Lens Type** — Prime (fixed focal length) or Zoom (variable)
- **FreeD Source** — creates LiveLink FreeD source on `0.0.0.0:40000`
- **LiveLink Controller** — adds component with Camera role, UseCameraRange enabled
- **Encoder Calibration** — live readout from FreeD, capture min/max for focus and zoom
- **Lens File** — generates ULensFile with correct encoder mapping and focal length tables
- **FreeD Settings** — sets UseManualRange on the FreeD source after calibration

## Features

- **One-click setup** — SETUP NOW configures everything
- **Live encoder readout** — real-time focus/zoom values from FreeD stream
- **Prime/Zoom selector** — spinbox inputs for focal length, configurable ranges
- **Focus distance range** — near/far in cm (default 60–4095cm)
- **Calibration** — capture actual encoder values, auto-sets FreeD UseManualRange
- **Camera picker** — lists all CineCameraActors in the level, refresh button
- **Network display** — shows your IP address for device configuration
- **Setup log** — timestamped output of every step

## Supported Protocols

- **FreeD (3D Protocol)** — UDP, port 40000, industry standard
- **OpenTrack IO** — coming soon

## Requirements

- Unreal Engine 5.6+ (tested on 5.6, designed for 5.0+ compatibility)
- Plugins (auto-enabled via dependency): LiveLink, LiveLinkCamera, LiveLinkFreeD, CameraCalibrationCore, LensComponent

## Project Structure

```
FonixFlowTrackerSetup/
├── FonixFlowTrackerSetup.uplugin
├── Config/
├── Resources/Icons/
├── Source/
│   ├── FonixFlowTrackerSetup/          # Runtime module
│   │   ├── Public/                     # Types, Settings, Subsystem
│   │   └── Private/                    # Implementation
│   └── FonixFlowTrackerSetupEditor/    # Editor module
│       ├── Public/Widgets/             # Panel, Wizard, AI Chat headers
│       └── Private/Widgets/            # Panel, Wizard, AI Chat implementation
```

## License

MIT
