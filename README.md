# FonixFlow Tracker Setup

**Unreal Engine plugin — one-click Live Link camera tracking auto-setup for FreeD and OpenTrack protocols.**

## Quick Start

1. **Copy** `FonixFlowTrackerSetup/` into your project's `Plugins/` folder
2. **Enable** — Edit > Plugins > "FonixFlow Tracker Setup" > Enable
3. **Click** the FF blue square button in the Level Editor toolbar
4. **Select camera** → choose Prime or Zoom → click **SETUP NOW**
5. **Calibrate** — rotate lens to min/max, capture Near/Far (and Wide/Tele for zoom), click **APPLY CALIBRATION**
6. Once calibrated, **APPLY LENS FILE** appears to re-apply the calibration to a new session

## What It Does

Automates the entire Live Link camera tracking pipeline in one compact dockable panel:

- **Camera Selection** — pick any CineCameraActor in the level
- **Lens Type** — Prime (fixed focal length) or Zoom (variable, Wide/Tele capture)
- **Auto-enables plugins** — ICVFX and LiveLinkLens enabled automatically on first use
- **FreeD Source** — creates LiveLink FreeD source on `0.0.0.0:40000`, skips if already active
- **LiveLink Controller** — adds component with Camera role, UseCameraRange enabled
- **Encoder Calibration** — live readout from FreeD, capture min/max for focus and zoom
- **Lens File** — generates ULensFile at `/Game/FonixFlowTrackerSetup/TrackedLens`
- **FreeD Settings** — sets UseManualRange on the FreeD source after calibration
- **Setup log** — writes timestamped output to `Saved/Logs/FonixFlowTracker.log`

## UI Layout

The panel uses a compact **2-tab layout** (~350 px wide, no scrolling needed):

- **Camera Setup tab** — Camera picker, Lens type, Protocol, Network IP, SETUP NOW button
- **Calibration tab** — Live Focus/Zoom readout, Near/Far/Wide/Tele capture rows, APPLY CALIBRATION button, APPLY LENS FILE button (appears after first calibration)

Active tab is highlighted. Log accordion is removed from the UI — check the log file instead.

## Features

- **One-click setup** — SETUP NOW configures everything
- **Live encoder readout** — real-time focus/zoom values from FreeD stream at 20Hz
- **Prime/Zoom selector** — inline spinbox inputs, zoom shows Wide/Tele capture rows
- **Focus distance range** — near/far in cm captured from live encoder
- **Calibration** — capture actual encoder values, auto-sets FreeD UseManualRange
- **Camera picker** — lists all CineCameraActors in the level, refresh button
- **Network display** — shows your IP:port for FreeD device configuration
- **FF toolbar icon** — blue square with white FF letters in the Level Editor toolbar
- **Active tab highlight** — darker background on the selected tab
- **Apply Lens File** — re-runs calibration on next session without re-capturing
- **Duplicate source skip** — SETUP NOW reuses an existing FreeD source if still active
- **External log file** — all steps written to `Saved/Logs/FonixFlowTracker.log`

## Supported Protocols

- **FreeD (3D Protocol)** — UDP, port 40000, industry standard
- **OpenTrack IO** — coming soon

## Requirements

- Unreal Engine 5.5 – 5.8
- Plugins (auto-enabled): LiveLink, LiveLinkCamera, LiveLinkFreeD, CameraCalibrationCore, VirtualCameraCore, LensComponent, ICVFX, LiveLinkLens

## Project Structure

```
FonixFlowTrackerSetup/
├── FonixFlowTrackerSetup.uplugin
├── Config/
├── Resources/Icons/
│   ├── FonixFlowFF.png     # Toolbar icon (20x20 blue FF square)
│   └── FonixFlowFF.svg     # Source SVG
├── Source/
│   ├── FonixFlowTrackerSetup/          # Runtime module
│   │   ├── Public/                     # Types, Settings, Subsystem
│   │   └── Private/                    # Implementation
│   └── FonixFlowTrackerSetupEditor/    # Editor module
│       ├── Public/Widgets/             # Panel, Wizard, AI Chat headers
│       └── Private/Widgets/            # Panel, Wizard, AI Chat implementation
```

## Known Compatibility

| Engine | Status |
|--------|--------|
| UE 5.5 | Builds and runs — all APIs verified against 5.5 documentation |
| UE 5.6 | Builds and runs |
| UE 5.7 | Builds and runs (`GetObjectsOfClass` replaces `TObjectIterator` for C++20) |
| UE 5.8 | Builds and runs — all APIs verified against 5.8 documentation |
| UE 5.4 and below | Not tested |

## License

MIT
