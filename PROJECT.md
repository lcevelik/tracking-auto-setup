# FonixFlow Tracker Setup

## Goals
- One-click Live Link camera tracking setup for Unreal Engine
- Support FreeD and OpenTrack IO protocols
- Automate: Live Link source, camera component, anchor point, lens file, virtual camera
- Multi-camera support with presets
- Target UE 5.6+ with backward compatibility to UE 5.0+
- Distribution: Unreal Marketplace first, GitHub later

## In Progress
- [ ] Test compilation on WinPC with UE 5.6
- [ ] Lens encoder range capture from Live Link data stream
- [ ] Camera rig preset defaults (encoder ranges per manufacturer)

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

## Done
- [x] Plugin scaffold created (Runtime + Editor modules)
- [x] Core types defined (FTrackingConnectionSettings, FCameraSetupConfig, FFonixFlowTrackerResult)
- [x] Subsystem architecture (UFonixFlowTrackerSetupSubsystem)
- [x] GitHub repo created
- [x] AI Chat Panel (STrackingAIChatPanel)
  - [x] OpenRouter/OpenAI-compatible API integration
  - [x] VP-focused system prompt
  - [x] Chat history, error handling
  - [x] Configurable API key/endpoint/model
- [x] Setup Wizard (SFonixFlowTrackerWizard)
  - [x] 7-step guided flow: Protocol → Network → Camera → Lens → Anchor → Review → Done
  - [x] Protocol selection (FreeD/OpenTrack IO)
  - [x] Camera rig presets (Generic, Panasonic, Sony, stYpe, Mosys, Ncam)
  - [x] Network config (IP/port, multicast/unicast)
  - [x] Camera picker (create new or select existing CineCameraActor)
  - [x] Lens calibration step (capture min/max encoder values)
  - [x] Anchor point configuration
  - [x] Virtual camera toggle
  - [x] Review step with full summary
- [x] Editor toolbar integration
  - [x] Wizard tab button
  - [x] AI Chat tab button
  - [x] Quick Setup submenu (FreeD/OpenTrack)
  - [x] Tools menu entries

## Blocked
- (none)

## Releases
- v0.2.0 — AI Chat, Setup Wizard, Camera Picker (current)
- v0.1.0 — Initial scaffold

## Notes
- UE source reference: D:\UE_Engine\UE_5.6 on WinPC (read-only)
- FreeD module: Engine/Plugins/VirtualProduction/LiveLinkFreeD
- OpenTrackIO module: Engine/Plugins/VirtualProduction/LiveLinkOpenTrackIO
- LiveLink Camera: Engine/Plugins/VirtualProduction/LiveLinkCamera
- Camera Calibration: Engine/Plugins/VirtualProduction/CameraCalibrationCore
- Virtual Camera: Engine/Plugins/VirtualProduction/VirtualCameraCore
- FreeD default port: 40000, OpenTrack multicast port: 55555
- FreeD connection settings: IP + UDP port
- OpenTrack connection settings: Source number (1-200) for multicast 235.135.1.[N]
- FreeD encoder data: Focus (24-bit), Zoom (24-bit), UserDefined (16-bit)
- Camera rig presets: Generic, Panasonic, Sony, stYpe, Mosys, Ncam
