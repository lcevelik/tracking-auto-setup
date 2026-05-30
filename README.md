# Tracking Auto Setup

**Unreal Engine plugin for one-click Live Link camera tracking setup.**

## Quick Start

1. **Enable plugin** — Edit > Plugins > "Tracking Auto Setup" > Enable
2. **Click the camera icon** in the Level Editor toolbar
3. **Follow the wizard** — done

That's it. No extra steps, no scripts to run.

## What It Does

Automates the entire Live Link camera tracking setup:

1. **Live Link Source** — FreeD or OpenTrack IO with correct port
2. **Camera** — Create new or pick existing CineCameraActor
3. **Lens File** — Create with proper encoder tables, focal length, image center
4. **Encoder Calibration** — Capture min/max encoder ranges
5. **Sensor/Filmback** — Presets (Super 35, Alexa 35, RED, Full Frame, etc.)
6. **Anchor Point** — Tracking origin
7. **Lens Component** — FIZ evaluation, distortion, filmback override
8. **Virtual Camera** — Optional VCam integration

## Features

- **Setup Wizard** — 9-step guided configuration
- **AI Chat Assistant** — Ask tracking/VP questions in-editor
- **Camera Picker** — Select existing CineCameraActors from scene
- **Lens File Creation** — Proper ULensFile with encoder/focal length tables
- **Sensor Presets** — Super 35, Alexa 35, RED V-Raptor, Full Frame, etc.
- **Quick Setup** — One-click FreeD or OpenTrack with defaults
- **Custom Icon** — SVG camera+tracking icon in toolbar

## Supported Protocols

| Protocol | Default Port | Notes |
|----------|-------------|-------|
| FreeD | 40000 | UDP, industry standard |
| OpenTrack IO | 55555 (multicast) | Open protocol |

## Requirements

- Unreal Engine 5.0+ (5.6 recommended)
- Live Link, LiveLinkCamera, CameraCalibrationCore, LensComponent plugins

## Installation

1. Clone into `Plugins/TrackingAutoSetup/`
2. Enable in Edit > Plugins
3. Click toolbar icon

## License

MIT
