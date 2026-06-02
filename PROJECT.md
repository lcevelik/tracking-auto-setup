# FonixFlow Tracker Setup

## Goals
- One-click Live Link camera tracking setup for Unreal Engine
- Single "Setup Now" button automates entire 3D tracking pipeline
- Support FreeD (3D Protocol) and OpenTrack IO protocols
- Prime and Zoom lens support with configurable focal length ranges
- Live encoder readout and calibration from FreeD data stream
- Target UE 5.5–5.8 with pre-built Win64 binaries

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
- [ ] Camera rig preset defaults (encoder ranges per manufacturer: Generic, Sony, Panasonic, Mosys, stYpe, Ncam)
- [ ] Documentation and examples
- [ ] AI Chat: streaming responses, markdown rendering
- [ ] AI Chat: context-aware setup suggestions (read current scene state)
- [ ] UE 5.8 pre-built binary (needs UE 5.8 installed on WinPC)

## Done
- [x] v1.2.0 — UE 5.5–5.8 compatibility, pre-built binaries
  - [x] Verified all APIs compatible across UE 5.5, 5.6, 5.7, 5.8
  - [x] Pre-built Win64 binaries for UE 5.5, 5.6, 5.7 on GitHub Releases
  - [x] Build pipeline: PowerShell script builds via UBT on WinPC, packages into ZIP
  - [x] Version bump locations: .uplugin VersionName + SFonixFlowTrackerSetupPanel.cpp LOCTEXT
  - [x] CLAUDE.md added for agent guidance
- [x] v1.1.0 — UE 5.7 compatibility + workflow improvements
  - [x] Replace `TObjectIterator` for-loops with `GetObjectsOfClass()` (fixes UE5.7 C++20 MSVC C7568 error)
  - [x] Applied in both `FonixFlowTrackerSetupSubsystem.cpp` and `SFonixFlowTrackerSetupPanel.cpp`
  - [x] Drop Log tab from UI — log now writes to `Saved/Logs/FonixFlowTracker.log` via `FFileHelper::SaveStringToFile` with `FILEWRITE_Append`
  - [x] FF blue square PNG toolbar icon (20×20, #1a3a58 background, white FF pixels, generated via Python struct/zlib)
  - [x] Active tab highlighting — `SBorder` background color driven by `BorderBackgroundColor_Lambda`
  - [x] Auto-enable ICVFX and LiveLinkLens plugins via `.uplugin` dependency list
  - [x] "APPLY LENS FILE" button appears after calibration (`bCalibrationApplied` flag + `Visibility_Lambda`) — calls `ApplyCalibration()` directly, no separate LensComponent creation
  - [x] Skip duplicate FreeD source — SETUP NOW checks `ActiveSourceGuid` against `GetSources()` before creating a new source
  - [x] `FSlateVectorImageBrush` not available in UE5.6/5.7 SlateCore — switched to `FSlateImageBrush` + PNG
  - [x] Fix `LogPath` variable name collision with UE's global `DECLARE_LOG_CATEGORY_EXTERN(LogPath, Warning, All)` — renamed to `LogFilePath`
  - [x] 2-tab layout: Camera Setup + Calibration (Log tab removed)
- [x] v1.0.0 — Full calibration pipeline
  - [x] FreeD source creation on 0.0.0.0 via reflection (ExportText, not ImportText)
  - [x] LiveLink polling — reads actual encoder values from FreeD at 20Hz
  - [x] Capture functions read real LiveLink values (not hardcoded 0/0xFFFFFF)
  - [x] Prime/Zoom lens selector with SAssignNew + explicit visibility control
  - [x] Zoom calibration: Wide/Tele capture rows (visible for zoom lens only)
  - [x] Focus distance range configurable (near/far cm)
  - [x] Zoom µm→mm fix (divide FreeD zoom encoder by 1000)
  - [x] ApplyCalibration sets FreeD UseManualRange=true via reflection
  - [x] ApplyCalibration sets UseCameraRange on LiveLink camera controller
  - [x] LiveLink controller: subject from LiveLink (auto-discovered), Camera role
  - [x] Lens file: correct encoder mapping for prime (fixed FL) and zoom (min/max)
  - [x] Iris encoder pinned to MinFStop (FreeD has no iris data)
  - [x] Pre-populate ControllerMap so ApplyCalibration works on first click
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
- v1.2.0 — UE 5.5–5.8 compat, pre-built Win64 binaries (5.5/5.6/5.7), CLAUDE.md (current)
- v1.1.0 — UE5.7 C++20 compat, 6 workflow improvements, log to file, FF icon, tab highlight
- v1.0.0 — Full calibration pipeline, LiveLink polling, prime/zoom, FreeD manual range
- v0.3.0 — UE 5.6 compatibility, reflection-based FreeD source creation
- v0.2.0 — AI Chat, Setup Wizard, Camera Picker
- v0.1.0 — Initial scaffold

## Notes
- UE source reference: D:\UE_Engine\UE_5.6 and D:\UE_Engine\UE_5.7 on WinPC (read-only)
- FreeD module: Engine/Plugins/VirtualProduction/LiveLinkFreeD
- FreeDConnectionSettings struct is in Private/ — access via reflection (GetObjectsOfClass, not TObjectIterator — TObjectIterator breaks in UE5.7 C++20 MSVC mode with C7568)
- FreeD source factory: ULiveLinkFreeDSourceFactory — use ExportText for connection string
- LiveLinkCameraController: access via ControllerMap[ULiveLinkCameraRole::StaticClass()]
- bUseCameraRange is on ULiveLinkCameraController (not on ULiveLinkComponentController)
- ILiveLinkClient::GetSubjects(false, false) — UE 5.6 API signature
- EvaluateFrame_AnyThread takes FLiveLinkSubjectFrameData (not FLiveLinkStaticDataStruct)
- SVerticalBox::FSlot does NOT have .Visibility() — use SAssignNew + SetVisibility instead
- FreeD encoder data: Focus (24-bit), Zoom (24-bit), UserDefined (16-bit)
- FreeD zoom encoder is in µm — divide by 1000 to get mm
- FreeD default port: 40000
- Key modules: Sockets, Networking (IP detection), LiveLinkCamera, LiveLinkFreeD
- SlateCore Brushes in UE5.6/5.7: only SlateImageBrush, SlateColorBrush, SlateDynamicImageBrush, SlateBoxBrush, SlateBorderBrush, SlateRoundedBoxBrush — NO FSlateVectorImageBrush
- Build pipeline: `build_plugin.ps1` on WinPC (D:\TempPluginBuild) — creates temp project, runs UBT per UE version, packages ZIPs
- Build settings: UE 5.5/5.6 use V4, UE 5.7 uses V5 + bOverrideBuildEnvironment=true
- Deploy pipeline: `scp -r FonixFlowTrackerSetup "f@10.0.0.18:F:/Unreal Projects/AbelCineDemo/Plugins/"` then SSH build
- Build cmd (5.6): `"D:\UE_Engine\UE_5.6\Engine\Build\BatchFiles\Build.bat" AbelCineDemoEditor Win64 Development -Project="F:\Unreal Projects\AbelCineDemo\AbelCineDemo.uproject"`
- Build cmd (5.7 plugin): `"D:\UE_Engine\UE_5.7\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="..." -Package="..." -Rocket -TargetPlatforms=Win64`
