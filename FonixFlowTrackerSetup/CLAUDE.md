# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

An Unreal Engine 5 editor plugin (`FonixFlowTrackerSetup`) for virtual production. It automates Live Link camera tracking setup — the user selects a `CineCameraActor`, clicks "SETUP NOW", and the plugin wires up a FreeD or OpenTrack IO Live Link source, configures the actor's `ULiveLinkComponentController`, and generates a `ULensFile` asset.

## Building

This plugin has no standalone build. It must live in a UE5 project's `Plugins/` folder and is compiled by UnrealBuildTool when the project is built. On Linux:

```bash
<UE_ROOT>/Engine/Build/BatchFiles/Linux/Build.sh <ProjectName>Editor Linux Development "<ProjectPath>/<ProjectName>.uproject" -plugin="<PluginPath>/FonixFlowTrackerSetup.uplugin"
```

Alternatively, open the `.uproject` in the Unreal Editor — it will offer to compile the plugin automatically.

There are no automated tests in this codebase.

## Module layout

Two modules, strict separation:

| Module | Type | Depends on |
|--------|------|------------|
| `FonixFlowTrackerSetup` | Runtime | LiveLink, CameraCalibrationCore, LensComponent, CinematicCamera |
| `FonixFlowTrackerSetupEditor` | Editor | Runtime module + LiveLinkFreeD, Slate, UnrealEd, HTTP/Json |

The editor module depends on the runtime module, never the reverse. The runtime module's subsystem (`UFonixFlowTrackerSetupSubsystem`) is a `UWorldSubsystem` and is also callable from Blueprint.

## Architecture: two-step user flow

**Step 1 — Camera Setup tab (`SFonixFlowTrackerSetupPanel`):**
- User picks a `CineCameraActor` from the level, selects protocol (FreeD/OpenTrack), sets port/lens type, clicks **SETUP NOW**.
- `RunOneClickSetup()` does everything in one synchronous call:
  1. Removes any stale `ULiveLinkComponentController` on the actor, creates a fresh one.
  2. Pre-populates `ControllerMap` with a `ULiveLinkCameraController` immediately (avoids the first-click silent failure that happens if you wait for the editor tick to do it).
  3. Creates a FreeD Live Link source via reflection (see below) and stores the GUID in `ActiveSourceGuid`.
  4. Starts a 20 Hz timer (`PollLiveLinkData`) that reads live encoder values and auto-assigns the first camera subject it finds to the controller.
  5. Sets `UCineCameraComponent::LensSettings` min/max focal length.
  6. Switches the panel to the Calibration tab.

**Step 2 — Calibration tab:**
- Live readout (focus cm, zoom mm) comes from `PollLiveLinkData()` reconstructing physical values from the normalized 0–1 FreeD values using `ULiveLinkFreeDSourceSettings::FocusDistanceEncoderData` / `FocalLengthEncoderData`.
- User rotates the lens and clicks CAPTURE for NEAR/FAR (focus) and WIDE/TELE (zoom).
- **APPLY CALIBRATION** calls `ULensSetupUtility::ApplyLensConfiguration()`, which:
  - Creates a `ULensFile` asset at `/Game/FonixFlowTrackerSetup/TrackedLens`.
  - Populates the focus encoder table (normalized 0→near cm, 1→far cm).
  - Pins the iris encoder curve flat at `MinFStop` (FreeD sends no iris data; without this the camera controller interpolates a random high f-stop).
  - Sets `CamCtrl->LensFilePicker.LensFile` on the `ULiveLinkCameraController` inside the controller map.
- The **APPLY LENS FILE** button (visible after first apply) also calls `ApplyCalibration()` — it's the same operation, not a separate one.

## Key implementation patterns

**FreeD source creation via reflection** — The plugin cannot `#include` `FLiveLinkFreeDConnectionSettings` directly without a private module dependency. Instead, `CreateLiveLinkSource()` and `RunOneClickSetup()` both use `GetObjectsOfClass(UScriptStruct::StaticClass(), ...)` to find `LiveLinkFreeDConnectionSettings` by name, then set `IPAddress`/`UDPPortNumber` via `CastField<FStrProperty>` / `CastField<FUInt16Property>`. This is intentional — maintain this pattern when changing connection settings.

**FreeD listens on `0.0.0.0`** — The IP is always set to `0.0.0.0` (listen on all interfaces), not the auto-detected local IP. The displayed local IP in the Network section is informational only.

**FreeD encoder units** — FreeD focus arrives normalized (0–1); physical cm is reconstructed using `FreeDSettings->FocusDistanceEncoderData.{Min,Max}`. FreeD zoom arrives normalized and represents micrometres (µm); divide by 1000 to get mm. These conversions live in `PollLiveLinkData()`.

**UE5.7 / C++20 compatibility** — `TObjectIterator<UScriptStruct>` is replaced with `GetObjectsOfClass(UScriptStruct::StaticClass(), ...)` everywhere. Do not reintroduce `TObjectIterator` for UE-managed types.

**OpenTrack IO is not implemented** — `CreateLiveLinkSource()` logs a warning and returns an invalid GUID. `RunOneClickSetup()` bails early with an error log if the user selects OpenTrack. Do not assume it works.

## Settings

`UFonixFlowTrackerSettings` (`UDeveloperSettings`, config=Game) exposes defaults and the AI Chat API key. Accessible at **Edit → Project Settings → Plugins → FonixFlow Tracker Setup**. The AI chat panel (`SFonixFlowTrackerAIChatPanel`) calls an OpenAI-compatible endpoint (default: OpenRouter with `anthropic/claude-sonnet-4`) and is gated on `IsAIChatConfigured()` (non-empty API key).

## Logs

All setup activity is appended to `<ProjectDir>/Saved/Logs/FonixFlowTracker.log` via `AddLog()`. Use this for debugging without opening the UE Output Log.
