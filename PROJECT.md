# FonixFlow Tracker Setup

## Goals
- One-click Live Link camera tracking setup for Unreal Engine
- Single "Setup Now" button automates entire 3D tracking pipeline
- Support FreeD (3D Protocol) and OpenTrack IO protocols
- Prime and Zoom lens support with configurable focal length ranges
- Live encoder readout and calibration from FreeD data stream
- Target UE 5.6+ with backward compatibility goal

## In Progress
- [ ] Runtime testing — verify full flow end-to-end in UE Editor
- [ ] OpenTrack IO protocol support (UI ready, implementation pending)
- [ ] Verify lens file quality — encoder mapping, focal length tables, image center

## To Do
- [ ] Multi-camera setup support (multiple cameras in one scene)
- [ ] Lens file auto-generation from tracking data over time
- [ ] Virtual camera integration (VCamCore wiring)
- [ ] Auto-detect FreeD packets on network
- [ ] Blueprint function library for procedural setups
- [ ] Save/load tracking presets
- [ ] UE 5.0–5.5 version compatibility layer
- [ ] Camera rig preset defaults (encoder ranges per manufacturer: Generic, Sony, Panasonic, Mosys, stYpe, Ncam)
- [ ] Documentation and examples
- [ ] AI Chat: streaming responses, markdown rendering
- [ ] AI Chat: context-aware setup suggestions (read current scene state)

## Done
- [x] v1.0.0 — Full calibration pipeline
  - [x] FreeD source creation on 0.0.0.0 via reflection (ExportText, not ImportText)
  - [x] LiveLink polling — reads actual encoder values from FreeD at 20Hz
  - [x] Capture functions read real LiveLink values (not hardcoded 0/0xFFFFFF)
  - [x] Prime/Zoom lens selector with SAssignNew + explicit visibility control
  - [x] Focus distance range configurable (near/far cm)
  - [x] ApplyCalibration sets FreeD UseManualRange=true via reflection
  - [x] ApplyCalibration sets UseCameraRange on LiveLink camera controller
  - [x] LiveLink controller: subject from LiveLink (auto-discovered), Camera role
  - [x] Lens file: correct encoder mapping for prime (fixed FL) and zoom (min/max)
  - [x] Version bumped to 1.0.0
- [x] v0.3.0 — UE 5.6 compatibility
  - [x] UE 5.6 API fixes (FEditorStyle→FAppStyle, CineCamera includes, VCamCore)
  - [x] LiveLinkFreeD module dependency added
  - [x] Runtime reflection for FLiveLinkFreeDConnectionSettings (private header)
  - [x] All 25 compile units pass, both DLLs link clean
- [x] v0.2.0 — AI Chat, Setup Wizard, Camera Picker
  - [x] AI Chat Panel (SFonixFlowTrackerAIChatPanel)
  - [x] Setup Wizard (deprecated, moved to panel)
  - [x] Camera picker — lists CineCameraActors in level
  - [x] Editor toolbar integration (button + Tools menu)
- [x] v0.1.0 — Initial scaffold
  - [x] Plugin scaffold (Runtime + Editor modules)
  - [x] Core types (FTrackingConnectionSettings, FCameraSetupConfig, FFonixFlowTrackerResult)
  - [x] Subsystem architecture (UFonixFlowTrackerSetupSubsystem)
  - [x] GitHub repo created

## Blocked
- (none)

## Releases
- v1.0.0 — Full calibration pipeline, LiveLink polling, prime/zoom, FreeD manual range (current)
- v0.3.0 — UE 5.6 compatibility, reflection-based FreeD source creation
- v0.2.0 — AI Chat, Setup Wizard, Camera Picker
- v0.1.0 — Initial scaffold

## Notes
- UE source reference: D:\UE_Engine\UE_5.6 on WinPC (read-only, do not modify)
- FreeD module: Engine/Plugins/VirtualProduction/LiveLinkFreeD
- FreeDConnectionSettings struct is in Private/ — access via reflection (TObjectIterator<UScriptStruct>)
- FreeD source factory: ULiveLinkFreeDSourceFactory — use ExportText for connection string
- LiveLinkCameraController: access via ControllerMap[ULiveLinkCameraRole::StaticClass()]
- bUseCameraRange is on ULiveLinkCameraController (not on ULiveLinkComponentController)
- ILiveLinkClient::GetSubjects(false, false) — UE 5.6 API signature
- EvaluateFrame_AnyThread takes FLiveLinkSubjectFrameData (not FLiveLinkStaticDataStruct)
- SVerticalBox::FSlot does NOT have .Visibility() — use SAssignNew + SetVisibility instead
- FreeD encoder data: Focus (24-bit), Zoom (24-bit), UserDefined (16-bit)
- FreeD default port: 40000
- Key modules: Sockets, Networking (IP detection), LiveLinkCamera, LiveLinkFreeD
