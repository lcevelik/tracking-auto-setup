# Tracking Auto Setup

## Goals
- One-click Live Link camera tracking setup for Unreal Engine
- Support FreeD and OpenTrack IO protocols
- Automate: Live Link source, camera component, anchor point, lens file, virtual camera
- Multi-camera support with presets
- Target UE 5.6+ with backward compatibility to UE 5.0+
- Distribution: Unreal Marketplace first, GitHub later

## In Progress
- [ ] Core plugin scaffold and module structure
- [ ] FreeD protocol auto-setup implementation
- [ ] OpenTrack IO protocol auto-setup implementation
- [ ] Editor toolbar integration (one-click buttons)

## To Do
- [ ] Camera rig presets (Panasonic, Sony, stYpe, Mosys, Ncam)
- [ ] Multi-camera setup support
- [ ] Lens file auto-generation from tracking data
- [ ] Virtual camera integration
- [ ] Auto-detect FreeD packets on network
- [ ] Blueprint function library for procedural setups
- [ ] Save/load tracking presets
- [ ] Settings panel UI (full wizard option)
- [ ] UE 5.0-5.5 version compatibility layer
- [ ] Marketplace listing preparation
- [ ] Documentation and examples

## Done
- [x] Plugin scaffold created (Runtime + Editor modules)
- [x] Core types defined (FTrackingConnectionSettings, FCameraSetupConfig, FTrackingSetupResult)
- [x] Subsystem architecture (UTrackingAutoSetupSubsystem)
- [x] GitHub repo created

## Blocked
- (none)

## Releases
- v0.1.0 — Initial scaffold (current)

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
