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
#include "Widgets/Input/SSpinBox.h"
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
#include "LiveLinkCameraController.h"
#include "Roles/LiveLinkCameraRole.h"
#include "Roles/LiveLinkCameraTypes.h"
#include "ILiveLinkClient.h"
#include "LiveLinkSourceFactory.h"
#include "LiveLinkSourceSettings.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Editor.h"
#include "Styling/AppStyle.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "TimerManager.h"
#include "LiveLinkFreeDSourceSettings.h"

#define LOCTEXT_NAMESPACE "SFonixFlowTrackerSetupPanel"

// ═════════════════════════════════════════════════════════════════════
// Lifecycle
// ═════════════════════════════════════════════════════════════════════

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

				// Lens type (Prime / Zoom)
				+ SVerticalBox::Slot().AutoHeight().Padding(8, 4)
				[ BuildLensTypeSection() ]

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

SFonixFlowTrackerSetupPanel::~SFonixFlowTrackerSetupPanel()
{
	StopLiveLinkPolling();
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
// Lens Type Selection (Prime / Zoom)
// ═════════════════════════════════════════════════════════════════════

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildLensTypeSection()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("LensTypeLabel", "Lens Type"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
	]

	// Prime lens option
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
	[
		SNew(SBorder)
		.BorderImage(bUsePrimeLens
			? FAppStyle::GetBrush("ToolPanel.GroupBorder") : FAppStyle::GetBrush("NoBorder"))
		.Padding(12, 6)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(bUsePrimeLens ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState) { bUsePrimeLens = true; UpdateLensTypeVisibility(); })
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8, 0, 0, 0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[ SNew(STextBlock).Text(LOCTEXT("PrimeName", "Prime Lens")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]
				+ SVerticalBox::Slot().AutoHeight()
				[ SNew(STextBlock).Text(LOCTEXT("PrimeDesc", "Fixed focal length — no zoom"))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f))) ]
			]
		]
	]

	// Prime lens focal length input
	+ SVerticalBox::Slot().AutoHeight().Padding(24, 0, 0, 8)
	[
		SAssignNew(PrimeLensInputBox, SBox)
		.Visibility(bUsePrimeLens ? EVisibility::Visible : EVisibility::Collapsed)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
			[ SNew(STextBlock).Text(LOCTEXT("PrimeFL", "Focal Length (mm):")).Font(FCoreStyle::GetDefaultFontStyle("Regular", 10)) ]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SSpinBox<float>)
				.MinValue(8.0f)
				.MaxValue(300.0f)
				.Value(PrimeLensFocalLengthMM)
				.OnValueChanged_Lambda([this](float Val) { PrimeLensFocalLengthMM = Val; })
				.MinDesiredWidth(80)
			]
		]
	]

	// Zoom lens option
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
	[
		SNew(SBorder)
		.BorderImage(!bUsePrimeLens
			? FAppStyle::GetBrush("ToolPanel.GroupBorder") : FAppStyle::GetBrush("NoBorder"))
		.Padding(12, 6)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(!bUsePrimeLens ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState) { bUsePrimeLens = false; UpdateLensTypeVisibility(); })
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8, 0, 0, 0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[ SNew(STextBlock).Text(LOCTEXT("ZoomName", "Zoom Lens")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]
				+ SVerticalBox::Slot().AutoHeight()
				[ SNew(STextBlock).Text(LOCTEXT("ZoomDesc", "Variable focal length — calibrate zoom encoder"))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f))) ]
			]
		]
	]

	// Zoom lens range inputs
	+ SVerticalBox::Slot().AutoHeight().Padding(24, 0, 0, 4)
	[
		SAssignNew(ZoomLensInputBox, SBox)
		.Visibility(!bUsePrimeLens ? EVisibility::Visible : EVisibility::Collapsed)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
			[ SNew(STextBlock).Text(LOCTEXT("ZoomMin", "Min Focal Length (mm):")).Font(FCoreStyle::GetDefaultFontStyle("Regular", 10)) ]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SSpinBox<float>)
				.MinValue(8.0f)
				.MaxValue(300.0f)
				.Value(FocalLengthMinMM)
				.OnValueChanged_Lambda([this](float Val) { FocalLengthMinMM = Val; })
				.MinDesiredWidth(80)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16, 0, 8, 0)
			[ SNew(STextBlock).Text(LOCTEXT("ZoomMax", "Max Focal Length (mm):")).Font(FCoreStyle::GetDefaultFontStyle("Regular", 10)) ]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SSpinBox<float>)
				.MinValue(8.0f)
				.MaxValue(300.0f)
				.Value(FocalLengthMaxMM)
				.OnValueChanged_Lambda([this](float Val) { FocalLengthMaxMM = Val; })
				.MinDesiredWidth(80)
			]
		]
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

	// LiveLink data readout
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 8)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(8)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[ SNew(STextBlock).Text(LOCTEXT("LLFocus", "LiveLink Focus (cm):")).Font(FCoreStyle::GetDefaultFontStyle("Regular", 10)) ]
			+ SVerticalBox::Slot().AutoHeight()
			[ SNew(STextBlock).Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetLiveLinkFocusText).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.8f, 0.4f))) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 0)
			[ SNew(STextBlock).Text(LOCTEXT("LLZoom", "LiveLink Zoom (mm):")).Font(FCoreStyle::GetDefaultFontStyle("Regular", 10)) ]
			+ SVerticalBox::Slot().AutoHeight()
			[ SNew(STextBlock).Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetLiveLinkZoomText).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.7f, 1.0f))) ]
		]
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
			[ SNew(STextBlock).Text(LOCTEXT("FocusDistLabel", "Focus Distance")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[ SNew(STextBlock).Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetFocusMinText) ]
				+ SHorizontalBox::Slot().AutoWidth()
				[ SNew(SButton).Text(LOCTEXT("CaptureFocusMin", "Capture Near"))
					.OnClicked_Lambda([this]() -> FReply { CaptureFocusMin(); return FReply::Handled(); })
					.ButtonStyle(FAppStyle::Get(), "FlatButton.Default") ]
			]

			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[ SNew(STextBlock).Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetFocusMaxText) ]
				+ SHorizontalBox::Slot().AutoWidth()
				[ SNew(SButton).Text(LOCTEXT("CaptureFocusMax", "Capture Far"))
					.OnClicked_Lambda([this]() -> FReply { CaptureFocusMax(); return FReply::Handled(); })
					.ButtonStyle(FAppStyle::Get(), "FlatButton.Default") ]
			]
		]
	]

	// Zoom calibration (zoom lenses only)
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 4)
	[
		SAssignNew(ZoomCalibBox, SBox)
		.Visibility(!bUsePrimeLens ? EVisibility::Visible : EVisibility::Collapsed)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			.Padding(8)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
				[ SNew(STextBlock).Text(LOCTEXT("ZoomCalibLabel", "Focal Length (Zoom)")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]

				+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
					[ SNew(STextBlock).Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetZoomWideText) ]
					+ SHorizontalBox::Slot().AutoWidth()
					[ SNew(SButton).Text(LOCTEXT("CaptureZoomWide", "Capture Wide"))
						.OnClicked_Lambda([this]() -> FReply { CaptureZoomWide(); return FReply::Handled(); })
						.ButtonStyle(FAppStyle::Get(), "FlatButton.Default") ]
				]

				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
					[ SNew(STextBlock).Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetZoomTeleText) ]
					+ SHorizontalBox::Slot().AutoWidth()
					[ SNew(SButton).Text(LOCTEXT("CaptureZoomTele", "Capture Tele"))
						.OnClicked_Lambda([this]() -> FReply { CaptureZoomTele(); return FReply::Handled(); })
						.ButtonStyle(FAppStyle::Get(), "FlatButton.Default") ]
				]
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
			SAssignNew(LogScrollBox, SScrollBox)
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
	if (LogScrollBox.IsValid())
	{
		LogScrollBox->ScrollToEnd();
	}
}

// ── LiveLink Polling ────────────────────────────────────────────────

void SFonixFlowTrackerSetupPanel::PollLiveLinkData()
{
	if (!SelectedCamera || !SelectedCamera->IsValidLowLevel())
	{
		return;
	}

	ILiveLinkClient* LiveLinkClient = nullptr;
	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	if (ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		LiveLinkClient = &ModularFeatures.GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
	}

	if (!LiveLinkClient) return;

	// Get all subjects — don't filter by source GUID (subject may not be linked yet)
	TArray<FLiveLinkSubjectKey> AllSubjects = LiveLinkClient->GetSubjects(false, false);

	for (const FLiveLinkSubjectKey& SubjectKey : AllSubjects)
	{
		// Check if subject supports camera role
		if (!LiveLinkClient->DoesSubjectSupportsRole_AnyThread(SubjectKey, ULiveLinkCameraRole::StaticClass()))
			continue;

		// Evaluate frame data
		FLiveLinkSubjectFrameData FrameData;
		if (LiveLinkClient->EvaluateFrame_AnyThread(SubjectKey.SubjectName, ULiveLinkCameraRole::StaticClass(), FrameData))
		{
			FLiveLinkCameraFrameData* CameraFrame = FrameData.FrameData.Cast<FLiveLinkCameraFrameData>();
			if (CameraFrame)
			{
				// FreeD normalizes the raw encoder to 0.0–1.0 in FocusDistance.
				// Reconstruct physical cm: physical = min + normalized * (max - min)
				// where min/max come from the FreeD source encoder auto-range.
				float PhysicalMin = 0.0f, PhysicalMax = 0.0f;
				ULiveLinkFreeDSourceSettings* FreeDSettings = nullptr;
				if (ActiveSourceGuid.IsValid())
				{
					FreeDSettings = Cast<ULiveLinkFreeDSourceSettings>(LiveLinkClient->GetSourceSettings(ActiveSourceGuid));
				}
				if (!FreeDSettings)
				{
					for (const FGuid& SrcGuid : LiveLinkClient->GetSources())
					{
						FreeDSettings = Cast<ULiveLinkFreeDSourceSettings>(LiveLinkClient->GetSourceSettings(SrcGuid));
						if (FreeDSettings) break;
					}
				}
				if (FreeDSettings)
				{
					PhysicalMin = (float)FreeDSettings->FocusDistanceEncoderData.Min;
					PhysicalMax = (float)FreeDSettings->FocusDistanceEncoderData.Max;
				}
				const float Delta = PhysicalMax - PhysicalMin;
				LiveLinkFocusValue = (Delta > 0.0f)
					? PhysicalMin + CameraFrame->FocusDistance * Delta
					: CameraFrame->FocusDistance;

				// Reconstruct zoom physical mm from FreeD FocalLength (also 0.0–1.0 normalized)
				float ZoomPhysMin = 0.0f, ZoomPhysMax = 0.0f;
				if (FreeDSettings)
				{
					ZoomPhysMin = (float)FreeDSettings->FocalLengthEncoderData.Min;
					ZoomPhysMax = (float)FreeDSettings->FocalLengthEncoderData.Max;
				}
				const float ZoomDelta = ZoomPhysMax - ZoomPhysMin;
				const float RawZoomMicrons = (ZoomDelta > 0.0f)
					? ZoomPhysMin + CameraFrame->FocalLength * ZoomDelta
					: CameraFrame->FocalLength;
				// FreeD zoom encoder unit is micrometres (µm); convert to mm
				LiveLinkZoomValue = RawZoomMicrons / 1000.0f;
			}
		}

		// Auto-assign subject to the LiveLink controller on the selected camera
		if (!bSubjectAutoAssigned && SelectedCamera && SelectedCamera->IsValidLowLevel())
		{
			ULiveLinkComponentController* LLController =
				SelectedCamera->FindComponentByClass<ULiveLinkComponentController>();
			if (LLController)
			{
				LLController->SubjectRepresentation.Subject = SubjectKey.SubjectName;
				bSubjectAutoAssigned = true;
				AddLog(FString::Printf(TEXT("LiveLink subject auto-assigned: '%s'"),
					*SubjectKey.SubjectName.ToString()));
			}
		}
		break; // Use first camera subject found
	}
}

void SFonixFlowTrackerSetupPanel::StopLiveLinkPolling()
{
	bLiveLinkPollingActive = false;
	if (GEditor)
	{
		UWorld* World = GEditor->GetEditorWorldContext().World();
		if (World)
		{
			World->GetTimerManager().ClearTimer(LiveLinkPollTimerHandle);
		}
	}
}

void SFonixFlowTrackerSetupPanel::UpdateLensTypeVisibility()
{
	EVisibility PrimeVis = bUsePrimeLens ? EVisibility::Visible : EVisibility::Collapsed;
	EVisibility ZoomVis = !bUsePrimeLens ? EVisibility::Visible : EVisibility::Collapsed;

	if (PrimeLensInputBox.IsValid()) PrimeLensInputBox->SetVisibility(PrimeVis);
	if (ZoomLensInputBox.IsValid()) ZoomLensInputBox->SetVisibility(ZoomVis);
	if (ZoomCalibBox.IsValid()) ZoomCalibBox->SetVisibility(ZoomVis);
}

// ── Setup ───────────────────────────────────────────────────────────

void SFonixFlowTrackerSetupPanel::RunOneClickSetup()
{
	bSetupRunning = true;
	bSetupComplete = false;
	bSetupSuccess = false;
	SetupLog.Empty();
	StopLiveLinkPolling();

	AddLog(TEXT("=== Starting Setup ==="));

	// Verify protocol
	if (SelectedProtocol != ETrackingProtocol::FreeD)
	{
		AddLog(TEXT("ERROR: Only FreeD protocol is currently supported. OpenTrack IO is coming soon."));
		bSetupRunning = false;
		bSetupComplete = true;
		return;
	}

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

	// Mark actor as modified so editor tracks the change
	CineCamera->Modify();

	// Remove ALL existing LiveLinkComponentControllers (handles stale/cached ones)
	TArray<ULiveLinkComponentController*> ExistingControllers;
	for (UActorComponent* Comp : CineCamera->GetComponents())
	{
		ULiveLinkComponentController* Existing = Cast<ULiveLinkComponentController>(Comp);
		if (Existing)
		{
			ExistingControllers.Add(Existing);
		}
	}
	if (ExistingControllers.Num() > 0)
	{
		AddLog(FString::Printf(TEXT("  Removing %d existing controller(s)"), ExistingControllers.Num()));
		for (ULiveLinkComponentController* Old : ExistingControllers)
		{
			Old->DestroyComponent();
		}
	}

	ULiveLinkComponentController* LLController = NewObject<ULiveLinkComponentController>(CineCamera, TEXT("FonixFlowLiveLinkController"));
	LLController->RegisterComponent();
	CineCamera->AddInstanceComponent(LLController);

	LLController = CineCamera->FindComponentByClass<ULiveLinkComponentController>();
	if (LLController)
	{
		AddLog(TEXT("  LiveLinkComponentController added"));

		// Subject representation: use subject from LiveLink (auto-discovered)
		LLController->SubjectRepresentation.Role = ULiveLinkCameraRole::StaticClass();
		// Don't set SubjectName — let it discover from the source

		// Pre-populate ControllerMap now: direct Role assignment bypasses
		// ULiveLinkComponentController::UpdateControllerMap(), so the map
		// would otherwise stay empty until the next editor tick — causing
		// ApplyCalibration to silently fail on the first click.
		if (!LLController->ControllerMap.Contains(ULiveLinkCameraRole::StaticClass()))
		{
			ULiveLinkCameraController* CamCtrl = NewObject<ULiveLinkCameraController>(LLController);
			CamCtrl->bUseCameraRange = true;
			LLController->ControllerMap.Add(ULiveLinkCameraRole::StaticClass(), CamCtrl);
		}

		AddLog(TEXT("  Subject role: Camera (from LiveLink)"));
	}
	else
	{
		AddLog(TEXT("  ERROR: Failed to add LiveLinkComponentController"));
	}

	// ── Step 2: Create Live Link FreeD source (0.0.0.0:40000) ──────
	AddLog(TEXT("Step 2: Creating Live Link FreeD source..."));

	FGuid SourceGuid;
	ILiveLinkClient* LiveLinkClient = nullptr;
	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	if (ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		LiveLinkClient = &ModularFeatures.GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
	}

	if (LiveLinkClient)
	{
		// Find the FreeD connection settings struct via reflection
		UScriptStruct* FreeDSettingsStruct = nullptr;
		for (TObjectIterator<UScriptStruct> It; It; ++It)
		{
			if (It->GetName() == TEXT("LiveLinkFreeDConnectionSettings"))
			{
				FreeDSettingsStruct = *It;
				break;
			}
		}

		// Find the FreeD source factory
		ULiveLinkSourceFactory* FreeDFactory = nullptr;
		for (TObjectIterator<ULiveLinkSourceFactory> It; It; ++It)
		{
			FText Name = It->GetSourceDisplayName();
			if (Name.ToString().Contains(TEXT("FreeD")))
			{
				FreeDFactory = *It;
				AddLog(FString::Printf(TEXT("  Factory: %s"), *Name.ToString()));
				break;
			}
		}

		if (FreeDFactory && FreeDSettingsStruct)
		{
			// Construct settings and set 0.0.0.0 via reflection
			TArray<uint8> SettingsMemory;
			SettingsMemory.SetNumZeroed(FreeDSettingsStruct->GetStructureSize());
			FreeDSettingsStruct->InitializeDefaultValue(SettingsMemory.GetData());

			FStrProperty* IPProp = CastField<FStrProperty>(FreeDSettingsStruct->FindPropertyByName(FName("IPAddress")));
			FUInt16Property* PortProp = CastField<FUInt16Property>(FreeDSettingsStruct->FindPropertyByName(FName("UDPPortNumber")));

			if (IPProp) IPProp->SetPropertyValue_InContainer(SettingsMemory.GetData(), TEXT("0.0.0.0"));
			if (PortProp) PortProp->SetPropertyValue_InContainer(SettingsMemory.GetData(), static_cast<uint16>(ListeningPort));

			// Generate connection string via ExportText
			FString ConnectionString;
			FreeDSettingsStruct->ExportText(ConnectionString, SettingsMemory.GetData(), nullptr, nullptr, PPF_None, nullptr);

			AddLog(FString::Printf(TEXT("  Connection: %s"), *ConnectionString));

			TSharedPtr<ILiveLinkSource> Source = FreeDFactory->CreateSource(ConnectionString);
			if (Source.IsValid())
			{
				SourceGuid = LiveLinkClient->AddSource(Source);
				ActiveSourceGuid = SourceGuid;
				bSubjectAutoAssigned = false; // Reset so PollLiveLinkData will auto-assign once subjects appear
				AddLog(FString::Printf(TEXT("  Source created — GUID: %s"), *SourceGuid.ToString()));
				AddLog(FString::Printf(TEXT("  Listening on 0.0.0.0:%d"), ListeningPort));

				// Start polling LiveLink data
				bLiveLinkPollingActive = true;
				UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
				if (World)
				{
					World->GetTimerManager().SetTimer(
						LiveLinkPollTimerHandle,
						FTimerDelegate::CreateSP(this, &SFonixFlowTrackerSetupPanel::PollLiveLinkData),
						0.05f, // 20Hz polling
						true);
					AddLog(TEXT("  LiveLink polling started (20Hz)"));
				}
			}
			else
			{
				AddLog(TEXT("  ERROR: FreeD factory returned null source"));
			}

			FreeDSettingsStruct->DestroyStruct(SettingsMemory.GetData());
		}
		else
		{
			if (!FreeDFactory) AddLog(TEXT("  ERROR: LiveLinkFreeD factory not found — enable LiveLinkFreeD plugin"));
			if (!FreeDSettingsStruct) AddLog(TEXT("  ERROR: FLiveLinkFreeDConnectionSettings struct not found"));
		}
	}
	else
	{
		AddLog(TEXT("  ERROR: LiveLink client not available"));
	}

	// ── Step 3: Configure CineCamera ─────────────────────────────────
	AddLog(TEXT("Step 3: Configuring CineCamera..."));
	UCineCameraComponent* CineComp = CineCamera->GetCineCameraComponent();
	if (CineComp)
	{
		if (bUsePrimeLens)
		{
			CineComp->LensSettings.MinFocalLength = PrimeLensFocalLengthMM;
			CineComp->LensSettings.MaxFocalLength = PrimeLensFocalLengthMM;
			AddLog(FString::Printf(TEXT("  Prime lens: %.0f mm"), PrimeLensFocalLengthMM));
		}
		else
		{
			CineComp->LensSettings.MinFocalLength = FocalLengthMinMM;
			CineComp->LensSettings.MaxFocalLength = FocalLengthMaxMM;
			AddLog(FString::Printf(TEXT("  Zoom lens: %.0f\u2013%.0f mm"), FocalLengthMinMM, FocalLengthMaxMM));
		}
		CineComp->MarkPackageDirty();
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

// ── Capture Functions (read actual LiveLink values) ─────────────────

void SFonixFlowTrackerSetupPanel::CaptureFocusMin()
{
	// LiveLink focus value is treated as physical cm from the 3D system
	FocusEncoderMin = LiveLinkFocusValue;
	bFocusMinCaptured = true;
	AddLog(FString::Printf(TEXT("Focus NEAR captured: %.2f cm"), FocusEncoderMin));
}

void SFonixFlowTrackerSetupPanel::CaptureFocusMax()
{
	FocusEncoderMax = LiveLinkFocusValue;
	bFocusMaxCaptured = true;
	AddLog(FString::Printf(TEXT("Focus FAR captured: %.2f cm"), FocusEncoderMax));
}

void SFonixFlowTrackerSetupPanel::CaptureZoomWide()
{
	ZoomEncoderWide = LiveLinkZoomValue;
	bZoomWideCaptured = true;
	// Also update the spinbox so the user sees the measured value
	FocalLengthMinMM = ZoomEncoderWide;
	AddLog(FString::Printf(TEXT("Zoom WIDE captured: %.2f mm"), ZoomEncoderWide));
}

void SFonixFlowTrackerSetupPanel::CaptureZoomTele()
{
	ZoomEncoderTele = LiveLinkZoomValue;
	bZoomTeleCaptured = true;
	// Also update the spinbox so the user sees the measured value
	FocalLengthMaxMM = ZoomEncoderTele;
	AddLog(FString::Printf(TEXT("Zoom TELE captured: %.2f mm"), ZoomEncoderTele));
}

// ── Apply Calibration ───────────────────────────────────────────────

void SFonixFlowTrackerSetupPanel::ApplyCalibration()
{
	AddLog(TEXT("=== Applying Calibration ==="));

	if (!SelectedCamera || !SelectedCamera->IsValidLowLevel())
	{
		AddLog(TEXT("ERROR: No camera selected"));
		return;
	}

	ACineCameraActor* CineCamera = SelectedCamera;

	// Build lens config
	// Captured LiveLink values are treated as physical cm from the 3D system.
	// Encoder table maps normalized encoder 0.0 -> Near cm, 1.0 -> Far cm.
	FLensConfiguration LensConfig;
	LensConfig.bCreateNewLensFile = true;
	LensConfig.LensFileName = TEXT("TrackedLens");
	LensConfig.FocusDistanceMinCM = FocusEncoderMin; // physical near (cm) at encoder=0
	LensConfig.FocusDistanceMaxCM = FocusEncoderMax; // physical far  (cm) at encoder=1

	if (bUsePrimeLens)
	{
		LensConfig.FocalLengthMinMM = PrimeLensFocalLengthMM;
		LensConfig.FocalLengthMaxMM = PrimeLensFocalLengthMM;
	}
	else
	{
		LensConfig.FocalLengthMinMM = bZoomWideCaptured ? ZoomEncoderWide : FocalLengthMinMM;
		LensConfig.FocalLengthMaxMM = bZoomTeleCaptured ? ZoomEncoderTele : FocalLengthMaxMM;
	}

	AddLog(FString::Printf(TEXT("Focus: near=%.2f cm, far=%.2f cm"), FocusEncoderMin, FocusEncoderMax));
	AddLog(FString::Printf(TEXT("Zoom: wide=%.2f mm, tele=%.2f mm"), LensConfig.FocalLengthMinMM, LensConfig.FocalLengthMaxMM));

	ULensFile* LensFile = ULensSetupUtility::ApplyLensConfiguration(CineCamera, LensConfig);
	if (LensFile)
	{
		AddLog(FString::Printf(TEXT("Lens file: %s"), *LensFile->GetName()));
		AddLog(TEXT("Saved at: /Game/FonixFlowTrackerSetup/TrackedLens"));

		// Refresh editor so the new LensComponent appears in the Details panel
		if (GEditor)
		{
			GEditor->NoteSelectionChange();
		}
	}

	// Apply to LiveLink controller
	ULiveLinkComponentController* LLController = CineCamera->FindComponentByClass<ULiveLinkComponentController>();
	if (LLController)
	{
		LLController->SubjectRepresentation.Role = ULiveLinkCameraRole::StaticClass();

		if (LLController->ControllerMap.Contains(ULiveLinkCameraRole::StaticClass()))
		{
			ULiveLinkCameraController* CamController = Cast<ULiveLinkCameraController>(
				LLController->ControllerMap[ULiveLinkCameraRole::StaticClass()]);
			if (CamController)
			{
				CamController->bUseCameraRange = true;
				AddLog(TEXT("  UseCameraRange enabled on camera controller"));
			}
		}
		AddLog(TEXT("Applied to LiveLink controller"));
	}
	else
	{
		AddLog(TEXT("WARNING: No LiveLink controller — run Setup Now first"));
	}

	AddLog(TEXT("=== Calibration Applied ==="));
}

// ═════════════════════════════════════════════════════════════════════
// QUERIES
// ═════════════════════════════════════════════════════════════════════

FText SFonixFlowTrackerSetupPanel::GetIPAddressText() const { return FText::FromString(LocalIPAddress); }

FText SFonixFlowTrackerSetupPanel::GetFocusMinText() const
{
	if (!bFocusMinCaptured) return LOCTEXT("FocusMinUncaptured", "Near: not captured");
	return FText::FromString(FString::Printf(TEXT("Near: %.2f cm"), FocusEncoderMin));
}

FText SFonixFlowTrackerSetupPanel::GetFocusMaxText() const
{
	if (!bFocusMaxCaptured) return LOCTEXT("FocusMaxUncaptured", "Far: not captured");
	return FText::FromString(FString::Printf(TEXT("Far: %.2f cm"), FocusEncoderMax));
}

FText SFonixFlowTrackerSetupPanel::GetZoomWideText() const
{
	if (!bZoomWideCaptured) return LOCTEXT("ZoomWideUncaptured", "Wide: not captured");
	return FText::FromString(FString::Printf(TEXT("Wide: %.2f mm"), ZoomEncoderWide));
}

FText SFonixFlowTrackerSetupPanel::GetZoomTeleText() const
{
	if (!bZoomTeleCaptured) return LOCTEXT("ZoomTeleUncaptured", "Tele: not captured");
	return FText::FromString(FString::Printf(TEXT("Tele: %.2f mm"), ZoomEncoderTele));
}

FText SFonixFlowTrackerSetupPanel::GetLiveLinkZoomText() const
{
	return FText::FromString(FString::Printf(TEXT("%.2f mm"), LiveLinkZoomValue));
}

FText SFonixFlowTrackerSetupPanel::GetSelectedCameraText() const
{
	if (SelectedCamera) return FText::Format(LOCTEXT("CamSelected", "Selected: {0}"), FText::FromString(SelectedCamera->GetActorLabel()));
	return LOCTEXT("CamNone", "No camera selected — add a CineCameraActor to the level");
}

FText SFonixFlowTrackerSetupPanel::GetLiveLinkFocusText() const
{
	return FText::FromString(FString::Printf(TEXT("%.2f cm"), LiveLinkFocusValue));
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
	if (!bFocusMinCaptured || !bFocusMaxCaptured) return false;
	if (!bUsePrimeLens && (!bZoomWideCaptured || !bZoomTeleCaptured)) return false;
	return true;
}

bool SFonixFlowTrackerSetupPanel::IsSetupButtonEnabled() const
{
	return !bSetupRunning && SelectedCamera != nullptr && SelectedCamera->IsValidLowLevel();
}

#undef LOCTEXT_NAMESPACE
