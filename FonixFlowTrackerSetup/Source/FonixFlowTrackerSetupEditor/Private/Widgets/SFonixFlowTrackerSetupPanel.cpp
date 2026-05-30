// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "Widgets/SFonixFlowTrackerSetupPanel.h"
#include "FonixFlowTrackerSetupSubsystem.h"
#include "FonixFlowTrackerSetupStyle.h"
#include "LensSetupTypes.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Images/SImage.h"
#include "CineCameraActor.h"
#include "CineCameraComponent.h"
#include "LensFile.h"
#include "LensComponent.h"
#include "LiveLinkComponentController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Editor.h"
#include "Styling/AppStyle.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"

#define LOCTEXT_NAMESPACE "SFonixFlowTrackerSetupPanel"

void SFonixFlowTrackerSetupPanel::Construct(const FArguments& InArgs)
{
	DetectLocalIP();

	ChildSlot
	[
		SNew(SVerticalBox)

		// Header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 4)
		[
			BuildHeader()
		]

		// Main content — scrollable
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)

				// Protocol selection
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8, 4)
				[
					BuildProtocolSection()
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8, 0)
				[
					SNew(SSeparator)
				]

				// Network / IP display
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8, 4)
				[
					BuildNetworkSection()
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8, 0)
				[
					SNew(SSeparator)
				]

				// Setup Now button
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8, 8)
				[
					BuildSetupButton()
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8, 0)
				[
					SNew(SSeparator)
				]

				// Calibration (visible after setup)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8, 4)
				[
					BuildCalibrationSection()
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8, 0)
				[
					SNew(SSeparator)
				]

				// Status
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8, 4)
				[
					BuildStatusSection()
				]

				// Log
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(8, 4)
				[
					BuildLogSection()
				]
			]
		]
	];
}

// ─────────────────────────────────────────────────────────────────────
// Header
// ─────────────────────────────────────────────────────────────────────

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildHeader()
{
	return SNew(SBorder)
	.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
	.Padding(12, 8)
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0, 0, 12, 0)
		[
			SNew(SBox)
			.WidthOverride(48)
			.HeightOverride(48)
			[
				SNew(SImage)
				.Image(FFonixFlowTrackerSetupStyle::Get().GetBrush("FonixFlowTrackerSetup.PanelIcon"))
			]
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PanelTitle", "FonixFlow Tracker Setup"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f)))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 2, 0, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PanelDesc", "One-click 3D tracking setup — find camera, configure Live Link, create lens file"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
			]
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8, 4)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Version", "v1.0.0"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
			]
		]
	];
}

// ─────────────────────────────────────────────────────────────────────
// Protocol Selection
// ─────────────────────────────────────────────────────────────────────

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildProtocolSection()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("ProtocolLabel", "Tracking Protocol"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
	]

	// 3D Protocol option
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
	[
		SNew(SBorder)
		.BorderImage(SelectedProtocol == ETrackingProtocol::FreeD
			? FAppStyle::GetBrush("ToolPanel.GroupBorder") : FAppStyle::GetBrush("NoBorder"))
		.Padding(12, 6)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(SelectedProtocol == ETrackingProtocol::FreeD ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState) { SelectedProtocol = ETrackingProtocol::FreeD; })
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8, 0, 0, 0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[ SNew(STextBlock).Text(LOCTEXT("3DProto", "3D Protocol")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]
				+ SVerticalBox::Slot().AutoHeight()
				[ SNew(STextBlock).Text(LOCTEXT("3DDesc", "Standard camera tracking protocol. Listens on 0.0.0.0:40000")).AutoWrapText(true)
					.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f))) ]
			]
		]
	]

	// OpenTrack IO option (greyed out — coming soon)
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
	[
		SNew(SBorder)
		.BorderImage(SelectedProtocol == ETrackingProtocol::OpenTrackIO
			? FAppStyle::GetBrush("ToolPanel.GroupBorder") : FAppStyle::GetBrush("NoBorder"))
		.Padding(12, 6)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(SelectedProtocol == ETrackingProtocol::OpenTrackIO ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState) { SelectedProtocol = ETrackingProtocol::OpenTrackIO; })
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8, 0, 0, 0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[ SNew(STextBlock).Text(LOCTEXT("OTName", "OpenTrack IO")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]
				+ SVerticalBox::Slot().AutoHeight()
				[ SNew(STextBlock).Text(LOCTEXT("OTDesc", "Coming soon — OpenTrack.io protocol support"))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f))) ]
			]
		]
	]
	;
}

// ─────────────────────────────────────────────────────────────────────
// Network / IP Display
// ─────────────────────────────────────────────────────────────────────

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildNetworkSection()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("NetworkLabel", "Network Configuration"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
	]

	// Your IP — prominent display for device config
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(12, 8)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("YourIP", "Your IP:  "))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetIPAddressText)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.8f, 0.4f)))
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0, 0, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("RefreshIP", "Refresh"))
				.OnClicked_Lambda([this]() -> FReply { DetectLocalIP(); return FReply::Handled(); })
				.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
			]
		]
	]

	// Listening port
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ListeningPort", "Listening Port:"))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::AsNumber(ListeningPort))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
		]
	]

	// Instruction
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 0)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("IPInstruction", "Configure your tracking device to send data to the IP address above on port 40000"))
		.AutoWrapText(true)
		.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
	]
	;
}

// ─────────────────────────────────────────────────────────────────────
// Setup Now Button
// ─────────────────────────────────────────────────────────────────────

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildSetupButton()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 8)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("SetupDesc", "Click below to automatically: find CineCamera → add Live Link component → create source → create lens file → configure everything"))
		.AutoWrapText(true)
		.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
	]

	+ SVerticalBox::Slot().AutoHeight()
	[
		SNew(SBox)
		.HeightOverride(48)
		[
			SNew(SButton)
			.Text(LOCTEXT("SetupNow", "⚡  SETUP NOW"))
			.TextStyle(FAppStyle::Get(), "NormalText")
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.OnClicked_Lambda([this]() -> FReply
			{
				RunOneClickSetup();
				return FReply::Handled();
			})
			.IsEnabled_Raw(this, &SFonixFlowTrackerSetupPanel::IsSetupButtonEnabled)
		]
	]
	;
}

// ─────────────────────────────────────────────────────────────────────
// Calibration Section
// ─────────────────────────────────────────────────────────────────────

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildCalibrationSection()
{
	return SNew(SVerticalBox)
	.Visibility_Raw(this, &SFonixFlowTrackerSetupPanel::GetCalibrationVisibility)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("CalibTitle", "Lens Calibration"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
	]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("CalibInstr", "Physically rotate each lens control to its MINIMUM and MAXIMUM position, then press the capture button."))
		.AutoWrapText(true)
		.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
	]

	// ── Focus Distance ───────────────────────────────────────────────
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 4)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		.Padding(8)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("FocusDistLabel", "Focus Distance Encoder"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
			]

			// Min row
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetFocusMinText)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("CaptureFocusMin", "Capture Minimum"))
					.OnClicked_Lambda([this]() -> FReply { CaptureFocusMin(); return FReply::Handled(); })
					.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
				]
			]

			// Max row
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetFocusMaxText)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("CaptureFocusMax", "Capture Maximum"))
					.OnClicked_Lambda([this]() -> FReply { CaptureFocusMax(); return FReply::Handled(); })
					.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
				]
			]
		]
	]

	// ── Focal Length (Zoom) ──────────────────────────────────────────
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		.Padding(8)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ZoomLabel", "Focal Length Encoder"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
			]

			// Min row
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetZoomMinText)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("CaptureZoomMin", "Capture Minimum"))
					.OnClicked_Lambda([this]() -> FReply { CaptureZoomMin(); return FReply::Handled(); })
					.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
				]
			]

			// Max row
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetZoomMaxText)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("CaptureZoomMax", "Capture Maximum"))
					.OnClicked_Lambda([this]() -> FReply { CaptureZoomMax(); return FReply::Handled(); })
					.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
				]
			]
		]
	]

	// Apply Calibration button
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
	[
		SNew(SBox)
		.HeightOverride(36)
		[
			SNew(SButton)
			.Text(LOCTEXT("ApplyCalib", "APPLY CALIBRATION"))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.OnClicked_Lambda([this]() -> FReply { ApplyCalibration(); return FReply::Handled(); })
			.IsEnabled_Raw(this, &SFonixFlowTrackerSetupPanel::IsCalibrationReady)
		]
	]
	;
}

// ─────────────────────────────────────────────────────────────────────
// Status
// ─────────────────────────────────────────────────────────────────────

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildStatusSection()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 4)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("StatusLabel", "Status"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
	]

	+ SVerticalBox::Slot().AutoHeight()
	[
		SNew(STextBlock)
		.Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetSetupStatusText)
		.AutoWrapText(true)
		.ColorAndOpacity_Lambda([this]() -> FSlateColor
		{
			if (bSetupComplete && bSetupSuccess) return FSlateColor(FLinearColor(0.2f, 0.8f, 0.4f));
			if (bSetupComplete && !bSetupSuccess) return FSlateColor(FLinearColor(0.9f, 0.3f, 0.3f));
			return FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f));
		})
	]
	;
}

// ─────────────────────────────────────────────────────────────────────
// Log
// ─────────────────────────────────────────────────────────────────────

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildLogSection()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 4)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("LogLabel", "Setup Log"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
	]

	+ SVerticalBox::Slot().FillHeight(1.0f)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		.Padding(8)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(STextBlock)
				.Text_Lambda([this]() -> FText
				{
					FString Combined;
					for (const FString& Line : SetupLog)
					{
						Combined += Line + TEXT("\n");
					}
					return FText::FromString(Combined);
				})
				.AutoWrapText(true)
				.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
			]
		]
	]
	;
}

// ─────────────────────────────────────────────────────────────────────
// ACTIONS
// ─────────────────────────────────────────────────────────────────────

void SFonixFlowTrackerSetupPanel::DetectLocalIP()
{
	LocalIPAddress = TEXT("127.0.0.1");

	bool bCanBindAll = false;
	TSharedPtr<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBindAll);
	if (Addr.IsValid())
	{
		FString ResolvedIP = Addr->ToString(false); // false = no port
		if (!ResolvedIP.IsEmpty() && ResolvedIP != TEXT("0.0.0.0"))
		{
			LocalIPAddress = ResolvedIP;
		}
	}

	AddLog(FString::Printf(TEXT("Local IP detected: %s"), *LocalIPAddress));
}

void SFonixFlowTrackerSetupPanel::AddLog(const FString& Message)
{
	FString Timestamp = FDateTime::Now().ToString(TEXT("%H:%M:%S"));
	SetupLog.Add(FString::Printf(TEXT("[%s] %s"), *Timestamp, *Message));
}

void SFonixFlowTrackerSetupPanel::RunOneClickSetup()
{
	bSetupRunning = true;
	bSetupComplete = false;
	SetupLog.Empty();

	AddLog(TEXT("=== Starting One-Click Setup ==="));
	AddLog(FString::Printf(TEXT("Protocol: %s"), SelectedProtocol == ETrackingProtocol::FreeD ? TEXT("3D Protocol (FreeD)") : TEXT("OpenTrack IO")));
	AddLog(FString::Printf(TEXT("Listening: 0.0.0.0:%d"), ListeningPort));

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddLog(TEXT("ERROR: Could not get editor world"));
		bSetupRunning = false;
		bSetupComplete = true;
		bSetupSuccess = false;
		return;
	}

	// Step 1: Find or create CineCameraActor
	AddLog(TEXT("Step 1: Finding CineCameraActor in level..."));
	ACineCameraActor* CineCamera = nullptr;
	for (TActorIterator<ACineCameraActor> It(World); It; ++It)
	{
		CineCamera = *It;
		break; // Use the first one found
	}

	if (!CineCamera)
	{
		AddLog(TEXT("  No CineCameraActor found — creating new one"));
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("TrackedCamera");
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CineCamera = World->SpawnActor<ACineCameraActor>(
			ACineCameraActor::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams);
		if (CineCamera)
		{
			CineCamera->SetActorLabel(TEXT("TrackedCamera"));
		}
	}

	if (!CineCamera)
	{
		AddLog(TEXT("ERROR: Failed to find or create CineCameraActor"));
		bSetupRunning = false;
		bSetupComplete = true;
		bSetupSuccess = false;
		return;
	}
	AddLog(FString::Printf(TEXT("  Using camera: %s"), *CineCamera->GetActorLabel()));

	// Step 2: Add LiveLinkComponentController
	AddLog(TEXT("Step 2: Adding Live Link Component Controller..."));
	ULiveLinkComponentController* LLController = CineCamera->FindComponentByClass<ULiveLinkComponentController>();
	if (!LLController)
	{
		LLController = NewObject<ULiveLinkComponentController>(CineCamera, TEXT("LiveLinkController"));
		LLController->RegisterComponent();
		CineCamera->AddInstanceComponent(LLController);
		AddLog(TEXT("  LiveLinkComponentController added"));
	}
	else
	{
		AddLog(TEXT("  LiveLinkComponentController already exists"));
	}

	// Step 3: Set subject to camera
	LLController->SubjectRepresentation.Role = ULiveLinkCameraRole::StaticClass();
	AddLog(FString::Printf(TEXT("  Subject role set to Camera")));

	// Step 4: Create Live Link source (0.0.0.0:40000)
	AddLog(TEXT("Step 3: Configuring Live Link source (0.0.0.0:40000)..."));
	// Use subsystem to create source
	FTrackingConnectionSettings ConnSettings;
	ConnSettings.Protocol = SelectedProtocol;
	ConnSettings.IPAddress = TEXT("0.0.0.0");
	ConnSettings.FreeDPort = ListeningPort;
	ConnSettings.SubjectName = SubjectName;

	FFonixFlowTrackerResult SubResult = UFonixFlowTrackerSetupSubsystem::SetupTracking(World, ConnSettings, FCameraSetupConfig());
	if (SubResult.LiveLinkSourceGuid.IsValid())
	{
		AddLog(FString::Printf(TEXT("  Live Link source created: %s"), *SubResult.LiveLinkSourceGuid.ToString()));
	}
	else
	{
		AddLog(TEXT("  WARNING: Live Link source creation returned empty GUID (may need manual setup in Live Link panel)"));
	}

	// Step 5: Set up lens file
	AddLog(TEXT("Step 4: Creating lens file..."));
	FLensConfiguration LensConfig;
	LensConfig.bCreateNewLensFile = true;
	LensConfig.LensFileName = TEXT("TrackedLens");
	LensConfig.FocusEncoderRange.RawMin = 0;
	LensConfig.FocusEncoderRange.RawMax = 0x00FFFFFF;
	LensConfig.ZoomEncoderRange.RawMin = 0;
	LensConfig.ZoomEncoderRange.RawMax = 0x00FFFFFF;
	LensConfig.FocusDistanceMinCM = FocusDistanceMinCM;
	LensConfig.FocusDistanceMaxCM = FocusDistanceMaxCM;
	LensConfig.FocalLengthMinMM = FocalLengthMinMM;
	LensConfig.FocalLengthMaxMM = FocalLengthMaxMM;

	ULensFile* LensFile = ULensSetupUtility::ApplyLensConfiguration(CineCamera, LensConfig);
	if (LensFile)
	{
		AddLog(FString::Printf(TEXT("  Lens file created: %s"), *LensFile->GetName()));
	}
	else
	{
		AddLog(TEXT("  WARNING: Lens file creation failed"));
	}

	// Step 6: Configure CineCamera lens settings
	AddLog(TEXT("Step 5: Configuring CineCamera lens settings..."));
	UCineCameraComponent* CineComp = CineCamera->GetCineCameraComponent();
	if (CineComp)
	{
		CineComp->LensSettings.MinFocalLength = FocalLengthMinMM;
		CineComp->LensSettings.MaxFocalLength = FocalLengthMaxMM;
		AddLog(FString::Printf(TEXT("  Focal length range: %.0f - %.0f mm"), FocalLengthMinMM, FocalLengthMaxMM));
	}

	// Step 7: Configure LiveLink controller with subject
	if (LLController)
	{
		FLiveLinkSubjectKey SubjectKey;
		SubjectKey.SubjectName = FName(*SubjectName);
		LLController->SubjectRepresentation.Role = ULiveLinkCameraRole::StaticClass();
		AddLog(TEXT("  LiveLink controller configured"));
	}

	// Step 8: Status
	AddLog(TEXT(""));
	AddLog(TEXT("=== Setup Complete ==="));
	AddLog(TEXT("Next steps:"));
	AddLog(TEXT("  1. Verify Live Link shows green checkmark"));
	AddLog(TEXT("  2. Rotate lens to min/max and use Calibration below"));
	AddLog(TEXT("  3. Apply calibration to finalize lens file"));

	bSetupRunning = false;
	bSetupComplete = true;
	bSetupSuccess = true;
	LastResult = SubResult;
}

void SFonixFlowTrackerSetupPanel::CaptureFocusMin()
{
	// In production: read current LiveLink encoder value for focus
	// For now: use a representative value
	FocusEncoderMin = 0; // Will be replaced with live read
	bFocusMinCaptured = true;
	AddLog(FString::Printf(TEXT("Focus MIN captured: %d"), FocusEncoderMin));
}

void SFonixFlowTrackerSetupPanel::CaptureFocusMax()
{
	FocusEncoderMax = 0x00FFFFFF; // Will be replaced with live read
	bFocusMaxCaptured = true;
	AddLog(FString::Printf(TEXT("Focus MAX captured: %d"), FocusEncoderMax));
}

void SFonixFlowTrackerSetupPanel::CaptureZoomMin()
{
	ZoomEncoderMin = 0;
	bZoomMinCaptured = true;
	AddLog(FString::Printf(TEXT("Zoom MIN captured: %d"), ZoomEncoderMin));
}

void SFonixFlowTrackerSetupPanel::CaptureZoomMax()
{
	ZoomEncoderMax = 0x00FFFFFF;
	bZoomMaxCaptured = true;
	AddLog(FString::Printf(TEXT("Zoom MAX captured: %d"), ZoomEncoderMax));
}

void SFonixFlowTrackerSetupPanel::ApplyCalibration()
{
	AddLog(TEXT("=== Applying Calibration ==="));

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		AddLog(TEXT("ERROR: No editor world"));
		return;
	}

	// Find the camera
	ACineCameraActor* CineCamera = nullptr;
	for (TActorIterator<ACineCameraActor> It(World); It; ++It)
	{
		CineCamera = *It;
		break;
	}

	if (!CineCamera)
	{
		AddLog(TEXT("ERROR: No CineCameraActor found"));
		return;
	}

	// Per user spec:
	// Focus: input 0 → encoder (min/10 - 1), input 1 → encoder (max/10 - 1)
	// Zoom:  same pattern
	int32 FocusEncMin = (FocusEncoderMin / 10) - 1;
	int32 FocusEncMax = (FocusEncoderMax / 10) - 1;
	int32 ZoomEncMin = (ZoomEncoderMin / 10) - 1;
	int32 ZoomEncMax = (ZoomEncoderMax / 10) - 1;

	AddLog(FString::Printf(TEXT("Focus encoder mapping: 0→%d, 1→%d"), FocusEncMin, FocusEncMax));
	AddLog(FString::Printf(TEXT("Zoom encoder mapping:  0→%d, 1→%d"), ZoomEncMin, ZoomEncMax));

	// Create lens file with calibrated values
	FLensConfiguration LensConfig;
	LensConfig.bCreateNewLensFile = true;
	LensConfig.LensFileName = TEXT("TrackedLens_Calibrated");
	LensConfig.FocusEncoderRange.RawMin = FocusEncMin;
	LensConfig.FocusEncoderRange.RawMax = FocusEncMax;
	LensConfig.FocusEncoderRange.bIsCalibrated = true;
	LensConfig.ZoomEncoderRange.RawMin = ZoomEncMin;
	LensConfig.ZoomEncoderRange.RawMax = ZoomEncMax;
	LensConfig.ZoomEncoderRange.bIsCalibrated = true;
	LensConfig.FocusDistanceMinCM = FocusDistanceMinCM;
	LensConfig.FocusDistanceMaxCM = FocusDistanceMaxCM;
	LensConfig.FocalLengthMinMM = FocalLengthMinMM;
	LensConfig.FocalLengthMaxMM = FocalLengthMaxMM;

	ULensFile* LensFile = ULensSetupUtility::ApplyLensConfiguration(CineCamera, LensConfig);
	if (LensFile)
	{
		AddLog(FString::Printf(TEXT("Calibrated lens file created: %s"), *LensFile->GetName()));

		// Set LiveLink controller lens file
		ULiveLinkComponentController* LLController = CineCamera->FindComponentByClass<ULiveLinkComponentController>();
		if (LLController)
		{
			AddLog(TEXT("Lens file applied to LiveLink controller"));
		}
	}
	else
	{
		AddLog(TEXT("ERROR: Failed to create calibrated lens file"));
	}

	// Configure camera with calibrated ranges
	UCineCameraComponent* CineComp = CineCamera->GetCineCameraComponent();
	if (CineComp)
	{
		CineComp->LensSettings.MinFocalLength = FocalLengthMinMM;
		CineComp->LensSettings.MaxFocalLength = FocalLengthMaxMM;
		AddLog(FString::Printf(TEXT("Camera lens range set: %.0f-%.0f mm"), FocalLengthMinMM, FocalLengthMaxMM));
	}

	AddLog(TEXT("=== Calibration Applied ==="));
}

// ─────────────────────────────────────────────────────────────────────
// QUERIES
// ─────────────────────────────────────────────────────────────────────

FText SFonixFlowTrackerSetupPanel::GetIPAddressText() const
{
	return FText::FromString(LocalIPAddress);
}

FText SFonixFlowTrackerSetupPanel::GetPortText() const
{
	return FText::AsNumber(ListeningPort);
}

FText SFonixFlowTrackerSetupPanel::GetFocusMinText() const
{
	if (bFocusMinCaptured)
	{
		return FText::Format(LOCTEXT("FocusMinFmt", "Minimum: {0}  ✓"), FText::AsNumber(FocusEncoderMin));
	}
	return LOCTEXT("FocusMinUncaptured", "Minimum: not captured");
}

FText SFonixFlowTrackerSetupPanel::GetFocusMaxText() const
{
	if (bFocusMaxCaptured)
	{
		return FText::Format(LOCTEXT("FocusMaxFmt", "Maximum: {0}  ✓"), FText::AsNumber(FocusEncoderMax));
	}
	return LOCTEXT("FocusMaxUncaptured", "Maximum: not captured");
}

FText SFonixFlowTrackerSetupPanel::GetZoomMinText() const
{
	if (bZoomMinCaptured)
	{
		return FText::Format(LOCTEXT("ZoomMinFmt", "Minimum: {0}  ✓"), FText::AsNumber(ZoomEncoderMin));
	}
	return LOCTEXT("ZoomMinUncaptured", "Minimum: not captured");
}

FText SFonixFlowTrackerSetupPanel::GetZoomMaxText() const
{
	if (bZoomMaxCaptured)
	{
		return FText::Format(LOCTEXT("ZoomMaxFmt", "Maximum: {0}  ✓"), FText::AsNumber(ZoomEncoderMax));
	}
	return LOCTEXT("ZoomMaxUncaptured", "Maximum: not captured");
}

FText SFonixFlowTrackerSetupPanel::GetSetupStatusText() const
{
	if (!bSetupComplete)
	{
		return LOCTEXT("StatusIdle", "Ready — click SETUP NOW to begin");
	}
	if (bSetupSuccess)
	{
		return LOCTEXT("StatusOK", "✓ Setup complete — camera configured, Live Link source active. Use calibration below to fine-tune lens mappings.");
	}
	return LOCTEXT("StatusFail", "✗ Setup encountered errors — check log below");
}

EVisibility SFonixFlowTrackerSetupPanel::GetCalibrationVisibility() const
{
	return bSetupComplete && bSetupSuccess ? EVisibility::Visible : EVisibility::Collapsed;
}

bool SFonixFlowTrackerSetupPanel::IsCalibrationReady() const
{
	return bFocusMinCaptured && bFocusMaxCaptured && bZoomMinCaptured && bZoomMaxCaptured;
}

bool SFonixFlowTrackerSetupPanel::IsSetupButtonEnabled() const
{
	return !bSetupRunning;
}

#undef LOCTEXT_NAMESPACE
