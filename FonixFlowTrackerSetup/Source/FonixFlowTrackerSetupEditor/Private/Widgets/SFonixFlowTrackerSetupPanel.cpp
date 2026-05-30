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
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Images/SImage.h"
#include "CineCameraActor.h"
#include "CineCameraComponent.h"
#include "LensFile.h"
#include "LensComponent.h"
#include "LiveLinkComponentController.h"
#include "Roles/LiveLinkCameraRole.h"
#include "ILiveLinkClient.h"
#include "LiveLinkSourceFactory.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Editor.h"
#include "Styling/AppStyle.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"

#define LOCTEXT_NAMESPACE "SFonixFlowTrackerSetupPanel"

void SFonixFlowTrackerSetupPanel::Construct(const FArguments& InArgs)
{
	DetectLocalIP();
	RefreshCameraList();

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ BuildHeader() ]

		+ SVerticalBox::Slot().FillHeight(1.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)

				// Camera selection
				+ SVerticalBox::Slot().AutoHeight().Padding(8, 4)
				[ BuildCameraSection() ]

				+ SVerticalBox::Slot().AutoHeight().Padding(8, 0)
				[ SNew(SSeparator) ]

				// Protocol
				+ SVerticalBox::Slot().AutoHeight().Padding(8, 4)
				[ BuildProtocolSection() ]

				+ SVerticalBox::Slot().AutoHeight().Padding(8, 0)
				[ SNew(SSeparator) ]

				// Network / IP
				+ SVerticalBox::Slot().AutoHeight().Padding(8, 4)
				[ BuildNetworkSection() ]

				+ SVerticalBox::Slot().AutoHeight().Padding(8, 0)
				[ SNew(SSeparator) ]

				// Setup Now
				+ SVerticalBox::Slot().AutoHeight().Padding(8, 8)
				[ BuildSetupButton() ]

				+ SVerticalBox::Slot().AutoHeight().Padding(8, 0)
				[ SNew(SSeparator) ]

				// Calibration
				+ SVerticalBox::Slot().AutoHeight().Padding(8, 4)
				[ BuildCalibrationSection() ]

				+ SVerticalBox::Slot().AutoHeight().Padding(8, 0)
				[ SNew(SSeparator) ]

				// Status
				+ SVerticalBox::Slot().AutoHeight().Padding(8, 4)
				[ BuildStatusSection() ]

				// Log
				+ SVerticalBox::Slot().FillHeight(1.0f).Padding(8, 4)
				[ BuildLogSection() ]
			]
		]
	];
}

// ═════════════════════════════════════════════════════════════════════
// Header
// ═════════════════════════════════════════════════════════════════════

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildHeader()
{
	return SNew(SBorder)
	.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
	.Padding(12, 8)
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 12, 0)
		[
			SNew(SBox).WidthOverride(48).HeightOverride(48)
			[
				SNew(SImage)
				.Image(FFonixFlowTrackerSetupStyle::Get().GetBrush("FonixFlowTrackerSetup.PanelIcon"))
			]
		]

		+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
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
				.Text(LOCTEXT("PanelDesc", "Select camera, configure Live Link, create lens file — all in one click"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
			]
		]

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
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

// ═════════════════════════════════════════════════════════════════════
// Camera Selection
// ═════════════════════════════════════════════════════════════════════

void SFonixFlowTrackerSetupPanel::RefreshCameraList()
{
	AvailableCameras.Empty();
	CameraWeakPtrs.Empty();

	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World) return;

	for (TActorIterator<ACineCameraActor> It(World); It; ++It)
	{
		AvailableCameras.Add(*It);
		CameraWeakPtrs.Add(MakeWeakObjectPtr(*It));
	}

	if (CameraListView.IsValid())
	{
		CameraListView->RequestListRefresh();
	}

	// Auto-select first camera if none selected
	if (!SelectedCamera && AvailableCameras.Num() > 0)
	{
		SelectedCamera = AvailableCameras[0];
	}
}

TSharedRef<ITableRow> SFonixFlowTrackerSetupPanel::OnGenerateCameraRow(
	TWeakObjectPtr<ACineCameraActor> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	ACineCameraActor* Camera = InItem.Get();
	FString Label = Camera ? Camera->GetActorLabel() : TEXT("Invalid");

	return SNew(STableRow<TWeakObjectPtr<ACineCameraActor>>, OwnerTable)
	.Padding(4)
	[
		SNew(STextBlock)
		.Text(FText::FromString(Label))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
	];
}

void SFonixFlowTrackerSetupPanel::OnCameraSelected(
	TWeakObjectPtr<ACineCameraActor> InItem, ESelectInfo::Type SelectInfo)
{
	SelectedCamera = InItem.Get();
}

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildCameraSection()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CameraLabel", "Select Camera"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("Refresh", "Refresh"))
			.OnClicked_Lambda([this]() -> FReply { RefreshCameraList(); return FReply::Handled(); })
			.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
		]
	]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("CameraDesc", "Choose which CineCameraActor to configure with tracking"))
		.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
	]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		.Padding(4)
		[
			SNew(SBox)
			.HeightOverride_Lambda([this]() -> float
			{
				return FMath::Clamp(static_cast<float>(CameraWeakPtrs.Num()) * 28.0f, 28.0f, 150.0f);
			})
			[
				SAssignNew(CameraListView, SListView<TWeakObjectPtr<ACineCameraActor>>)
				.ListItemsSource(&CameraWeakPtrs)
				.OnGenerateRow(this, &SFonixFlowTrackerSetupPanel::OnGenerateCameraRow)
				.OnSelectionChanged(this, &SFonixFlowTrackerSetupPanel::OnCameraSelected)
				.SelectionMode(ESelectionMode::Single)
			]
		]
	]

	+ SVerticalBox::Slot().AutoHeight()
	[
		SNew(STextBlock)
		.Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetSelectedCameraText)
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
		.ColorAndOpacity_Lambda([this]() -> FSlateColor
		{
			return SelectedCamera ? FSlateColor(FLinearColor(0.2f, 0.8f, 0.4f)) : FSlateColor(FLinearColor(0.9f, 0.3f, 0.3f));
		})
	];
}

// ═════════════════════════════════════════════════════════════════════
// Protocol Selection
// ═════════════════════════════════════════════════════════════════════

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildProtocolSection()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("ProtocolLabel", "Tracking Protocol"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
	]

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
				[ SNew(STextBlock).Text(LOCTEXT("3DDesc", "Standard camera tracking. Listens on 0.0.0.0:40000")).AutoWrapText(true)
					.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f))) ]
			]
		]
	]

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
				[ SNew(STextBlock).Text(LOCTEXT("OTDesc", "Coming soon"))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f))) ]
			]
		]
	];
}

// ═════════════════════════════════════════════════════════════════════
// Network / IP Display
// ═════════════════════════════════════════════════════════════════════

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildNetworkSection()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("NetworkLabel", "Network Configuration"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
	]

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

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 0)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("IPInstruction", "Configure your tracking device to send data to the IP above on port 40000"))
		.AutoWrapText(true)
		.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
	];
}

// ═════════════════════════════════════════════════════════════════════
// Setup Now Button
// ═════════════════════════════════════════════════════════════════════

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildSetupButton()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 8)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("SetupDesc", "Adds Live Link component to selected camera, creates FreeD source on 0.0.0.0:40000"))
		.AutoWrapText(true)
		.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
	]

	+ SVerticalBox::Slot().AutoHeight()
	[
		SNew(SBox).HeightOverride(48)
		[
			SNew(SButton)
			.Text(LOCTEXT("SetupNow", "SETUP NOW"))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.OnClicked_Lambda([this]() -> FReply
			{
				RunOneClickSetup();
				return FReply::Handled();
			})
			.IsEnabled_Raw(this, &SFonixFlowTrackerSetupPanel::IsSetupButtonEnabled)
		]
	];
}

// ═════════════════════════════════════════════════════════════════════
// Calibration Section
// ═════════════════════════════════════════════════════════════════════

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
		.Text(LOCTEXT("CalibInstr", "Rotate each lens control to MIN and MAX, then capture. Apply creates the lens file."))
		.AutoWrapText(true)
		.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
	]

	// Focus Distance
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 4)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		.Padding(8)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[ SNew(STextBlock).Text(LOCTEXT("FocusDistLabel", "Focus Distance Encoder")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[ SNew(STextBlock).Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetFocusMinText) ]
				+ SHorizontalBox::Slot().AutoWidth()
				[ SNew(SButton).Text(LOCTEXT("CaptureFocusMin", "Capture Minimum"))
					.OnClicked_Lambda([this]() -> FReply { CaptureFocusMin(); return FReply::Handled(); })
					.ButtonStyle(FAppStyle::Get(), "FlatButton.Default") ]
			]

			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[ SNew(STextBlock).Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetFocusMaxText) ]
				+ SHorizontalBox::Slot().AutoWidth()
				[ SNew(SButton).Text(LOCTEXT("CaptureFocusMax", "Capture Maximum"))
					.OnClicked_Lambda([this]() -> FReply { CaptureFocusMax(); return FReply::Handled(); })
					.ButtonStyle(FAppStyle::Get(), "FlatButton.Default") ]
			]
		]
	]

	// Focal Length
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		.Padding(8)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[ SNew(STextBlock).Text(LOCTEXT("ZoomLabel", "Focal Length Encoder")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[ SNew(STextBlock).Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetZoomMinText) ]
				+ SHorizontalBox::Slot().AutoWidth()
				[ SNew(SButton).Text(LOCTEXT("CaptureZoomMin", "Capture Minimum"))
					.OnClicked_Lambda([this]() -> FReply { CaptureZoomMin(); return FReply::Handled(); })
					.ButtonStyle(FAppStyle::Get(), "FlatButton.Default") ]
			]

			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[ SNew(STextBlock).Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetZoomMaxText) ]
				+ SHorizontalBox::Slot().AutoWidth()
				[ SNew(SButton).Text(LOCTEXT("CaptureZoomMax", "Capture Maximum"))
					.OnClicked_Lambda([this]() -> FReply { CaptureZoomMax(); return FReply::Handled(); })
					.ButtonStyle(FAppStyle::Get(), "FlatButton.Default") ]
			]
		]
	]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
	[
		SNew(SBox).HeightOverride(36)
		[
			SNew(SButton)
			.Text(LOCTEXT("ApplyCalib", "APPLY CALIBRATION"))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.OnClicked_Lambda([this]() -> FReply { ApplyCalibration(); return FReply::Handled(); })
			.IsEnabled_Raw(this, &SFonixFlowTrackerSetupPanel::IsCalibrationReady)
		]
	];
}

// ═════════════════════════════════════════════════════════════════════
// Status & Log
// ═════════════════════════════════════════════════════════════════════

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildStatusSection()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 4)
	[ SNew(STextBlock).Text(LOCTEXT("StatusLabel", "Status")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]

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
	];
}

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildLogSection()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 4)
	[ SNew(STextBlock).Text(LOCTEXT("LogLabel", "Setup Log")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]

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
					for (const FString& Line : SetupLog) Combined += Line + TEXT("\n");
					return FText::FromString(Combined);
				})
				.AutoWrapText(true)
				.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
			]
		]
	];
}

// ═════════════════════════════════════════════════════════════════════
// ACTIONS
// ═════════════════════════════════════════════════════════════════════

void SFonixFlowTrackerSetupPanel::DetectLocalIP()
{
	LocalIPAddress = TEXT("127.0.0.1");
	bool bCanBindAll = false;
	TSharedPtr<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBindAll);
	if (Addr.IsValid())
	{
		FString ResolvedIP = Addr->ToString(false);
		if (!ResolvedIP.IsEmpty() && ResolvedIP != TEXT("0.0.0.0")) LocalIPAddress = ResolvedIP;
	}
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
	bSetupSuccess = false;
	SetupLog.Empty();

	AddLog(TEXT("=== Starting Setup ==="));

	// Verify camera selection
	if (!SelectedCamera || !SelectedCamera->IsValidLowLevel())
	{
		AddLog(TEXT("ERROR: No camera selected. Select a CineCameraActor from the list above."));
		bSetupRunning = false;
		bSetupComplete = true;
		return;
	}

	ACineCameraActor* CineCamera = SelectedCamera;
	AddLog(FString::Printf(TEXT("Camera: %s"), *CineCamera->GetActorLabel()));

	// ── Step 1: Add LiveLinkComponentController ──────────────────────
	AddLog(TEXT("Step 1: Adding LiveLinkComponentController..."));

	// Remove existing if present
	ULiveLinkComponentController* LLController = CineCamera->FindComponentByClass<ULiveLinkComponentController>();
	if (LLController)
	{
		AddLog(TEXT("  Removing existing controller"));
		LLController->DestroyComponent();
		LLController = nullptr;
	}

	LLController = NewObject<ULiveLinkComponentController>(CineCamera, TEXT("FonixFlowLiveLinkController"));
	LLController->RegisterComponent();
	CineCamera->AddInstanceComponent(LLController);

	LLController = CineCamera->FindComponentByClass<ULiveLinkComponentController>();
	if (LLController)
	{
		AddLog(TEXT("  LiveLinkComponentController added"));
		LLController->SubjectRepresentation.Role = ULiveLinkCameraRole::StaticClass();
		AddLog(TEXT("  Subject role: Camera"));
	}
	else
	{
		AddLog(TEXT("  ERROR: Failed to add LiveLinkComponentController"));
	}

	// ── Step 2: Create Live Link source (0.0.0.0:40000) ─────────────
	AddLog(TEXT("Step 2: Creating Live Link source..."));

	FGuid SourceGuid;
	ILiveLinkClient* LiveLinkClient = nullptr;
	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	if (ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		LiveLinkClient = &ModularFeatures.GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
	}

	if (LiveLinkClient)
	{
		ULiveLinkSourceFactory* Factory = nullptr;
		for (TObjectIterator<ULiveLinkSourceFactory> It; It; ++It)
		{
			FText Name = It->GetSourceDisplayName();
			if (Name.ToString().Contains(TEXT("FreeD")) || Name.ToString().Contains(TEXT("3D")))
			{
				Factory = *It;
				AddLog(FString::Printf(TEXT("  Factory: %s"), *Name.ToString()));
				break;
			}
		}

		if (Factory)
		{
			FString ConnectionString = TEXT("IPAddress=\"0.0.0.0\" UDPPortNumber=40000");
			TSharedPtr<ILiveLinkSource> Source = Factory->CreateSource(ConnectionString);
			if (Source.IsValid())
			{
				SourceGuid = LiveLinkClient->AddSource(Source);
				AddLog(FString::Printf(TEXT("  Source GUID: %s"), *SourceGuid.ToString()));
				AddLog(TEXT("  Listening on 0.0.0.0:40000"));
			}
			else
			{
				AddLog(TEXT("  WARNING: Factory returned null source"));
			}
		}
		else
		{
			AddLog(TEXT("  WARNING: No FreeD factory found — enable LiveLinkFreeD plugin"));
		}
	}

	// ── Step 3: Configure CineCamera ─────────────────────────────────
	AddLog(TEXT("Step 3: Configuring CineCamera..."));
	UCineCameraComponent* CineComp = CineCamera->GetCineCameraComponent();
	if (CineComp)
	{
		CineComp->LensSettings.MinFocalLength = FocalLengthMinMM;
		CineComp->LensSettings.MaxFocalLength = FocalLengthMaxMM;
		AddLog(FString::Printf(TEXT("  Focal length: %.0f - %.0f mm"), FocalLengthMinMM, FocalLengthMaxMM));
	}

	// ── Done ─────────────────────────────────────────────────────────
	AddLog(TEXT(""));
	AddLog(TEXT("=== Setup Complete ==="));
	AddLog(FString::Printf(TEXT("Camera: %s"), *CineCamera->GetActorLabel()));
	AddLog(FString::Printf(TEXT("LiveLink: %s"),
		CineCamera->FindComponentByClass<ULiveLinkComponentController>() ? TEXT("OK") : TEXT("FAILED")));
	AddLog(FString::Printf(TEXT("Source: %s"),
		SourceGuid.IsValid() ? *SourceGuid.ToString() : TEXT("Check Live Link panel")));
	AddLog(TEXT(""));
	AddLog(TEXT("Next: rotate lens to min/max, capture, then APPLY CALIBRATION"));

	bSetupRunning = false;
	bSetupComplete = true;
	bSetupSuccess = true;
}

void SFonixFlowTrackerSetupPanel::CaptureFocusMin() { FocusEncoderMin = 0; bFocusMinCaptured = true; AddLog(FString::Printf(TEXT("Focus MIN: %d"), FocusEncoderMin)); }
void SFonixFlowTrackerSetupPanel::CaptureFocusMax() { FocusEncoderMax = 0x00FFFFFF; bFocusMaxCaptured = true; AddLog(FString::Printf(TEXT("Focus MAX: %d"), FocusEncoderMax)); }
void SFonixFlowTrackerSetupPanel::CaptureZoomMin() { ZoomEncoderMin = 0; bZoomMinCaptured = true; AddLog(FString::Printf(TEXT("Zoom MIN: %d"), ZoomEncoderMin)); }
void SFonixFlowTrackerSetupPanel::CaptureZoomMax() { ZoomEncoderMax = 0x00FFFFFF; bZoomMaxCaptured = true; AddLog(FString::Printf(TEXT("Zoom MAX: %d"), ZoomEncoderMax)); }

void SFonixFlowTrackerSetupPanel::ApplyCalibration()
{
	AddLog(TEXT("=== Applying Calibration ==="));

	if (!SelectedCamera || !SelectedCamera->IsValidLowLevel())
	{
		AddLog(TEXT("ERROR: No camera selected"));
		return;
	}

	ACineCameraActor* CineCamera = SelectedCamera;

	int32 FocusEncMin = (FocusEncoderMin / 10) - 1;
	int32 FocusEncMax = (FocusEncoderMax / 10) - 1;
	int32 ZoomEncMin = (ZoomEncoderMin / 10) - 1;
	int32 ZoomEncMax = (ZoomEncoderMax / 10) - 1;

	AddLog(FString::Printf(TEXT("Focus mapping: 0->%d, 1->%d"), FocusEncMin, FocusEncMax));
	AddLog(FString::Printf(TEXT("Zoom mapping:  0->%d, 1->%d"), ZoomEncMin, ZoomEncMax));

	// Create lens file
	FLensConfiguration LensConfig;
	LensConfig.bCreateNewLensFile = true;
	LensConfig.LensFileName = TEXT("TrackedLens");
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
		AddLog(FString::Printf(TEXT("Lens file: %s"), *LensFile->GetName()));
		AddLog(TEXT("Saved at: /Game/FonixFlowTrackerSetup/TrackedLens"));

		// Apply to LiveLink
		ULiveLinkComponentController* LLController = CineCamera->FindComponentByClass<ULiveLinkComponentController>();
		if (LLController)
		{
			LLController->SubjectRepresentation.Role = ULiveLinkCameraRole::StaticClass();
			AddLog(TEXT("Applied to LiveLink controller"));
		}
		else
		{
			AddLog(TEXT("WARNING: No LiveLink controller — run Setup Now first"));
		}
	}
	else
	{
		AddLog(TEXT("ERROR: Failed to create lens file"));
	}

	UCineCameraComponent* CineComp = CineCamera->GetCineCameraComponent();
	if (CineComp)
	{
		CineComp->LensSettings.MinFocalLength = FocalLengthMinMM;
		CineComp->LensSettings.MaxFocalLength = FocalLengthMaxMM;
		AddLog(FString::Printf(TEXT("Camera lens: %.0f-%.0f mm"), FocalLengthMinMM, FocalLengthMaxMM));
	}

	AddLog(TEXT("=== Calibration Applied ==="));
}

// ═════════════════════════════════════════════════════════════════════
// QUERIES
// ═════════════════════════════════════════════════════════════════════

FText SFonixFlowTrackerSetupPanel::GetIPAddressText() const { return FText::FromString(LocalIPAddress); }

FText SFonixFlowTrackerSetupPanel::GetFocusMinText() const
{
	return bFocusMinCaptured ? FText::Format(LOCTEXT("FocusMinFmt", "Min: {0}"), FText::AsNumber(FocusEncoderMin)) : LOCTEXT("FocusMinUncaptured", "Min: not captured");
}

FText SFonixFlowTrackerSetupPanel::GetFocusMaxText() const
{
	return bFocusMaxCaptured ? FText::Format(LOCTEXT("FocusMaxFmt", "Max: {0}"), FText::AsNumber(FocusEncoderMax)) : LOCTEXT("FocusMaxUncaptured", "Max: not captured");
}

FText SFonixFlowTrackerSetupPanel::GetZoomMinText() const
{
	return bZoomMinCaptured ? FText::Format(LOCTEXT("ZoomMinFmt", "Min: {0}"), FText::AsNumber(ZoomEncoderMin)) : LOCTEXT("ZoomMinUncaptured", "Min: not captured");
}

FText SFonixFlowTrackerSetupPanel::GetZoomMaxText() const
{
	return bZoomMaxCaptured ? FText::Format(LOCTEXT("ZoomMaxFmt", "Max: {0}"), FText::AsNumber(ZoomEncoderMax)) : LOCTEXT("ZoomMaxUncaptured", "Max: not captured");
}

FText SFonixFlowTrackerSetupPanel::GetSelectedCameraText() const
{
	if (SelectedCamera) return FText::Format(LOCTEXT("CamSelected", "Selected: {0}"), FText::FromString(SelectedCamera->GetActorLabel()));
	return LOCTEXT("CamNone", "No camera selected — add a CineCameraActor to the level");
}

FText SFonixFlowTrackerSetupPanel::GetSetupStatusText() const
{
	if (!bSetupComplete) return LOCTEXT("StatusIdle", "Select a camera, then click SETUP NOW");
	if (bSetupSuccess) return LOCTEXT("StatusOK", "Setup complete — calibrate lens below");
	return LOCTEXT("StatusFail", "Setup error — check log");
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
	return !bSetupRunning && SelectedCamera != nullptr;
}

#undef LOCTEXT_NAMESPACE
