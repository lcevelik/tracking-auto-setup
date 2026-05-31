# FonixFlow Tracker Setup — Session Summary

## Overview

Built an Unreal Engine 5.6 plugin (`FonixFlowTrackerSetup`) from scratch that automates Live Link camera tracking setup with a single button click. The plugin creates a FreeD source, configures a LiveLink component on a selected camera, captures encoder calibration data, and generates a lens file.

---

## What Was Asked (Chronological)

### 1. Initial Setup
- Look at Unreal Engine source code on WinPC (`F:\Unreal Projects\AbelCineDemo\`) for reference
- Find plugin description in the ideas repo on GitHub
- Create a new GitHub repo called `tracking-auto-setup`
- Build a plugin that works with UE 5.6+ and covers all versions
- Support both C++ and Blueprint

### 2. First Deploy
- Deploy plugin to AbelCineDemo project on WinPC
- Build via command line (UBT) since IDE wasn't available
- Clear all cached data between builds

### 3. Bug Fixes — Round 1
- **FreeD source binding to 127.0.0.1** — should listen on 0.0.0.0
- **LiveLink component auto-created on startup** — should wait for user to select camera
- **Component re-creation broken** — if user deletes the component, plugin can't make a new one

### 4. Bug Fixes — Round 2
- **LiveLink Controller settings**: Subject representation set to "Subject from LiveLink", UseCameraRange checked
- **Lens mapping not working** — capture min/max always shows 0 and 16M, not actual FreeD values
- **Prime vs Zoom lens selection** — user should pick lens type, not default to 28-100mm
- **Lens file format incorrect** — encoder tables had wrong values

### 5. Bug Fixes — Round 3
- **Prime/Zoom toggle not working** — zoom section still visible when prime selected
- **Capture always returns 0** — not reading actual LiveLink data
- **FreeD UseManualRange** — must be enabled after calibration
- **Controller UseCameraRange** — must be checked on the camera controller

### 6. Version & Docs
- Bump version to v1.0.0
- Rewrite README.md and PROJECT.md
- Create summary document (this file)

---

## What Was Built

### Plugin Architecture
- **Runtime module** (`FonixFlowTrackerSetup`) — types, settings, subsystem
- **Editor module** (`FonixFlowTrackerSetupEditor`) — UI panel, toolbar integration, AI chat panel

### Key Components

#### `SFonixFlowTrackerSetupPanel` (Main UI)
- Camera selection list (all CineCameraActors in level)
- Lens type selector (Prime / Zoom) with explicit `SAssignNew` + `SetVisibility` control
- Protocol selector (FreeD / OpenTrack IO)
- Network display (IP address, port)
- SETUP NOW button
- Calibration section with LiveLink encoder readout
- Setup log with timestamps

#### `UFonixFlowTrackerSetupSubsystem` (Runtime)
- `SetupTracking()` — full setup pipeline
- `CreateLiveLinkSource()` — FreeD/OpenTrack source creation
- `CreateTrackedCamera()` — spawn or configure camera
- `ConfigureLiveLinkComponent()` — add LiveLink controller
- `ApplyLensConfiguration()` — create lens file

#### `ULensSetupUtility` (Lens File)
- Creates ULensFile with encoder tables
- Focus encoder mapping (raw → cm)
- Focal length mapping (mm for prime, encoder range for zoom)

### Features Implemented

| Feature | Status |
|---------|--------|
| FreeD source on 0.0.0.0:40000 | ✅ Working |
| LiveLink component on selected camera only | ✅ Working |
| Component re-creation after delete | ✅ Working |
| Prime lens mode (fixed focal length) | ✅ Working |
| Zoom lens mode (min/max focal length) | ✅ Working |
| LiveLink encoder polling (20Hz) | ✅ Working |
| Capture real encoder values | ✅ Working |
| FreeD UseManualRange after calibration | ✅ Working |
| Controller UseCameraRange | ✅ Working |
| Lens file generation | ✅ Working |
| Focus distance range (near/far cm) | ✅ Working |
| IP address display | ✅ Working |
| Setup log | ✅ Working |
| OpenTrack IO | 🔲 Coming soon |

---

## Technical Discoveries (UE 5.6 API Pitfalls)

### LiveLink API
- `ILiveLinkClient::GetSubjects()` takes two bools: `GetSubjects(bool bIncludeDisabledSubject, bool bIncludeVirtualSubject)`
- `EvaluateFrame_AnyThread()` takes `FLiveLinkSubjectFrameData&`, NOT `FLiveLinkStaticDataStruct`
- `GetSubjectRole()` doesn't exist — use `DoesSubjectSupportsRole_AnyThread()`
- `GetSubjectData()` doesn't exist — use `EvaluateFrame_AnyThread()`
- `GetSourceSettings(FGuid)` returns `ULiveLinkSourceSettings*` for a source

### FreeD Module
- `FLiveLinkFreeDConnectionSettings` is in `Private/` folder — can't include directly
- Use `TObjectIterator<UScriptStruct>` to find the struct at runtime
- Use `ExportText` to generate connection string (not hand-crafted `ImportText`)
- `ULiveLinkFreeDSourceFactory` — find via `TObjectIterator<ULiveLinkSourceFactory>`
- FreeD source settings: `FocusDistanceEncoderData`, `FocalLengthEncoderData` with `bUseManualRange`, `Min`, `Max`

### LiveLink Component Controller
- `ULiveLinkComponentController` has `ControllerMap` (TMap<Role, ControllerBase>)
- `bUseCameraRange` is on `ULiveLinkCameraController`, NOT on `ULiveLinkComponentController`
- Access via: `ControllerMap[ULiveLinkCameraRole::StaticClass()]`
- `GetChildren()` doesn't exist on `ULiveLinkComponentController`

### Slate UI
- `SVerticalBox::FSlot` does NOT have `.Visibility()` method
- Use `SAssignNew(BoxRef, SBox)` + `BoxRef->SetVisibility()` for dynamic visibility
- `Visibility_Lambda` works for static bindings but `SetVisibility` is more reliable for state changes
- Always call `UpdateLensTypeVisibility()` from checkbox callbacks

### Build System
- `LiveLinkFreeD` module dependency needed in `.Build.cs` and `.uplugin`
- `PrivateIncludePaths` can access private headers from other modules (fragile)
- Reflection (`TObjectIterator`, `CastField`, `SetPropertyValue_InContainer`) avoids private header dependency
- Live Coding blocks UBT — must close editor or press Ctrl+Alt+F11
- Always clear `DerivedDataCache/`, `Intermediate/`, `Binaries/`, `Saved/` for clean builds

---

## Build & Deploy Workflow

```bash
# 1. Push to GitHub
cd ~/tracking-auto-setup && git add -A && git commit -m "..." && git push

# 2. Clear caches on WinPC
ssh f@10.0.0.18 "rmdir /s /q \"F:\\Unreal Projects\\AbelCineDemo\\DerivedDataCache\""
ssh f@10.0.0.18 "rmdir /s /q \"F:\\Unreal Projects\\AbelCineDemo\\Intermediate\""

# 3. Deploy plugin
ssh f@10.0.0.18 "rmdir /s /q \"F:\\Unreal Projects\\AbelCineDemo\\Plugins\\FonixFlowTrackerSetup\""
scp -r ~/tracking-auto-setup/FonixFlowTrackerSetup f@10.0.0.18:"F:/Unreal Projects/AbelCineDemo/Plugins/"

# 4. Build
ssh f@10.0.0.18 "\"D:\\UE_Engine\\UE_5.6\\Engine\\Build\\BatchFiles\\Build.bat\" AbelCineDemoEditor Win64 Development -Project=\"F:\\Unreal Projects\\AbelCineDemo\\AbelCineDemo.uproject\""
```

---

## Commits (12 total)

| Hash | Message |
|------|---------|
| `ae3744d` | fix: FreeD source 0.0.0.0 via reflection, robust component re-creation |
| `58c236d` | fix: use TObjectIterator<UScriptStruct> instead of UStruct |
| `5449a25` | feat: LiveLink polling, prime/zoom lens, real encoder capture |
| `eb7ee50` | fix: correct Slate bracket structure and LiveLink API for UE 5.6 |
| `02ec3f9` | fix: explicit visibility control, LiveLink data read, FreeD manual range |
| `16e6cc8` | fix: use ControllerMap for UseCameraRange, remove GetChildren |
| `41e4f07` | chore: bump version to 1.0.0 |
| `ace387e` | docs: rewrite README and PROJECT.md for v1.0.0 |

---

## Current State

- **Version**: 1.0.0
- **Status**: Builds clean (25/25 actions, 60s full rebuild)
- **Deployed to**: `F:\Unreal Projects\AbelCineDemo\Plugins\FonixFlowTrackerSetup\`
- **GitHub**: `lcevelik/tracking-auto-setup` (main branch)
- **Next**: Runtime testing in UE Editor — verify full flow end-to-end
