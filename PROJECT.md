# FonixFlow Tracker Setup

## Goals
- One-click Live Link camera tracking setup for Unreal Engine
- Single "Setup Now" button automates entire 3D tracking pipeline
- Support 3D Protocol (FreeD) and OpenTrack IO protocols
- Automate: find camera, add Live Link component, create source, create lens file, configure everything
- Show user IP address for device configuration
- Lens calibration with min/max capture buttons
- Target UE 5.6+ with backward compatibility to UE 5.0+
- Distribution: Unreal Marketplace first, GitHub later

## In Progress
- [ ] Lens encoder range capture from Live Link data stream (live read values)
- [ ] Runtime testing in UE Editor — verify full flow end-to-end
- [ ] OpenTrack.io protocol support (UI ready, implementation pending)

## To Do
- [ ] Multi-camera setup support (multiple cameras in one scene)
- [ ] Lens file auto-generation from tracking data
- [ ] Virtual camera integration (VCamCore wiring)
- [ ] Auto-detect FreeD packets on network
- [ ] Blueprint function library for procedural setups
- [ ] Save/load tracking presets
- [ ] UE 5.0-5.5 version compatibility layer
- [ ] Marketplace listing preparation
- [ ] Documentation and examples
- [ ] AI Chat: streaming responses, markdown rendering
- [ ] AI Chat: context-aware setup suggestions (read current scene state)
- [ ] Live Link source creation via programmatic API (currently needs manual Live Link panel)
- [ ] Camera rig preset defaults (encoder ranges per manufacturer)

## Done
- [x] v1.0.0 — Single-button setup, wizard removed
  - [x] Replaced 9-step wizard with single "Setup Now" button
  - [x] Protocol selector: 3D Protocol / OpenTrack.io
  - [x] IP address display for device configuration
  - [x] Calibration section: Capture Min/Max buttons for focus + zoom
  - [x] APPLY CALIBRATION button creates calibrated lens file
  - [x] Setup log with timestamped output
  - [x] Status indicator (idle → running → complete)
- [x] UE 5.6 compilation — all 18 compile units pass, both DLLs link clean
- [x] UE 5.6 API compatibility fixes (FEditorStyle, CineCamera includes, VCamCore, etc.)
- [x] Plugin scaffold created (Runtime + Editor modules)
- [x] Core types defined (FTrackingConnectionSettings, FCameraSetupConfig, FFonixFlowTrackerResult)
- [x] Subsystem architecture (UFonixFlowTrackerSetupSubsystem)
- [x] GitHub repo created
- [x] AI Chat Panel (STrackingAIChatPanel)
- [x] Editor toolbar integration (single button, Tools menu)

## Blocked
- (none)

## Releases
- v1.0.0 — Single-button setup, wizard removed, IP display, calibration (current)
- v0.3.0 — UE 5.6 compatibility, all API fixes
- v0.2.0 — AI Chat, Setup Wizard, Camera Picker
- v0.1.0 — Initial scaffold

## Notes
- UE source reference: D:\UE_Engine\UE_5.6 on WinPC (read-only)
- FreeD module: Engine/Plugins/VirtualProduction/LiveLinkFreeD
- OpenTrackIO module: Engine/Plugins/VirtualProduction/LiveLinkOpenTrackIO
- LiveLink Camera: Engine/Plugins/VirtualProduction/LiveLinkCamera
- Camera Calibration: Engine/Plugins/VirtualProduction/CameraCalibrationCore
- Virtual Camera: Engine/Plugins/VirtualProduction/VirtualCameraCore
- FreeD default port: 40000, OpenTrack multicast port: 55555
- FreeD encoder data: Focus (24-bit), Zoom (24-bit), UserDefined (16-bit)
- Camera rig presets: Generic, Panasonic, Sony, stYpe, Mosys, Ncam
- Key: VCamCore module (not VirtualCameraCore), FAppStyle (not FEditorStyle), CinematicCamera dep needed
- Lens table AddPoint: 5 args (focus, zoom, info, tolerance, bIsCalibrationPoint)
- Sockets/Networking modules needed for IP detection via ISocketSubsystem
