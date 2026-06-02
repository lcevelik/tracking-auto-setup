#include "Widgets/SFonixFlowTrackerSetupPanel.h"
#include "FonixFlowTrackerSetupSubsystem.h"
#include "FonixFlowTrackerActions.h"
#include "FonixFlowTrackerSetupStyle.h"
#include "LensSetupTypes.h"
#include "SlateOptMacros.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
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
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Images/SImage.h"
#include "CineCameraActor.h"
#include "CineCameraComponent.h"
#include "LensFile.h"
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

		// Header
		+ SVerticalBox::Slot().AutoHeight()
		[ BuildHeader() ]

		// Tab bar
		+ SVerticalBox::Slot().AutoHeight()
		[ BuildTabBar() ]

		// Tab content
		+ SVerticalBox::Slot().FillHeight(1.0f)
		[
			SAssignNew(TabSwitcher, SWidgetSwitcher)
			.WidgetIndex(0)

			// Tab 0: Camera Setup
			+ SWidgetSwitcher::Slot()
			[ BuildCameraSetupTab() ]

			// Tab 1: Calibration
			+ SWidgetSwitcher::Slot()
			[ BuildCalibrationTab() ]

			// Tab 2: Chat
			+ SWidgetSwitcher::Slot()
			[
				SNew(SFonixFlowTrackerAIChatPanel)
				.Actions(this)
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
	return SNew(SBox).HeightOverride(44)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		.Padding(8, 0)
		[
			SNew(SHorizontalBox)

			// Blue "FF" logo box
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
			[
				SNew(SBox).WidthOverride(32).HeightOverride(32)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.1f, 0.27f, 0.45f))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("FF")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.ColorAndOpacity(FSlateColor(FLinearColor::White))
					]
				]
			]

			// Title
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PanelTitle", "FONIXFLOW TRACKER SETUP"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f)))
			]

			// Version
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Version", "v1.3.0"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.4f, 0.4f)))
			]
		]
	];
}

// ═════════════════════════════════════════════════════════════════════
// Tab Bar
// ═════════════════════════════════════════════════════════════════════

void SFonixFlowTrackerSetupPanel::SwitchTab(int32 TabIndex)
{
	ActiveTab = TabIndex;
	if (TabSwitcher.IsValid()) TabSwitcher->SetActiveWidgetIndex(ActiveTab);
}

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildTabBar()
{
	// Wrap each button in an SBorder whose background reacts to ActiveTab
	auto MakeTabBtn = [this](int32 TabIdx, FText Label) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
		.BorderBackgroundColor_Lambda([this, TabIdx]() -> FLinearColor
		{
			return ActiveTab == TabIdx
				? FLinearColor(0.22f, 0.22f, 0.22f) // active: lighter gray
				: FLinearColor(0.08f, 0.08f, 0.08f); // inactive: near-black
		})
		[
			SNew(SButton)
			.Text(Label)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.ButtonStyle(FAppStyle::Get(), "FlatButton")
			.TextStyle(FAppStyle::Get(), "SmallText")
			.OnClicked_Lambda([this, TabIdx]() -> FReply { SwitchTab(TabIdx); return FReply::Handled(); })
		];
	};

	return SNew(SBorder)
	.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
	.Padding(0)
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[ MakeTabBtn(0, LOCTEXT("TabCamera", "Camera Setup")) ]

		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[ MakeTabBtn(1, LOCTEXT("TabCalib", "Calibration")) ]

		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[ MakeTabBtn(2, LOCTEXT("TabChat", "Chat")) ]
	];
}

// ═════════════════════════════════════════════════════════════════════
// Tab: Camera Setup
// ═════════════════════════════════════════════════════════════════════

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildCameraSetupTab()
{
	return SNew(SScrollBox)
	+ SScrollBox::Slot().Padding(8, 4)
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight()
		[ BuildCameraSection() ]

		+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
		[ SNew(SSeparator) ]

		+ SVerticalBox::Slot().AutoHeight()
		[ BuildLensTypeSection() ]

		+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
		[ SNew(SSeparator) ]

		+ SVerticalBox::Slot().AutoHeight()
		[ BuildProtocolSection() ]

		+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
		[ SNew(SSeparator) ]

		+ SVerticalBox::Slot().AutoHeight()
		[ BuildNetworkSection() ]

		+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
		[ SNew(SSeparator) ]

		+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
		[ BuildSetupButton() ]
	];
}

// ═════════════════════════════════════════════════════════════════════
// Tab: Calibration
// ═════════════════════════════════════════════════════════════════════

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildCalibrationTab()
{
	return SNew(SScrollBox)
	+ SScrollBox::Slot().Padding(8, 4)
	[
		BuildCalibrationSection()
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
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
	];
}

void SFonixFlowTrackerSetupPanel::OnCameraSelected(
	TWeakObjectPtr<ACineCameraActor> InItem, ESelectInfo::Type SelectInfo)
{
	SelectedCamera = InItem.Get();
}

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildCameraSection()
{
	// Compact section-row: CAMERA label | list dropdown | Refresh button
	return SNew(SHorizontalBox)

	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
	[
		SNew(SBox).WidthOverride(60)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CameraLabel", "CAMERA"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
		]
	]

	+ SHorizontalBox::Slot().FillWidth(1.0f)
	[
		SNew(SBox)
		.HeightOverride_Lambda([this]() -> float
		{
			return FMath::Clamp(static_cast<float>(CameraWeakPtrs.Num()) * 24.0f, 24.0f, 96.0f);
		})
		[
			SAssignNew(CameraListView, SListView<TWeakObjectPtr<ACineCameraActor>>)
			.ListItemsSource(&CameraWeakPtrs)
			.OnGenerateRow(this, &SFonixFlowTrackerSetupPanel::OnGenerateCameraRow)
			.OnSelectionChanged(this, &SFonixFlowTrackerSetupPanel::OnCameraSelected)
			.SelectionMode(ESelectionMode::Single)
		]
	]

	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0, 0, 0)
	[
		SNew(SButton)
		.Text(LOCTEXT("Refresh", "Refresh"))
		.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
		.TextStyle(FAppStyle::Get(), "SmallText")
		.OnClicked_Lambda([this]() -> FReply { RefreshCameraList(); return FReply::Handled(); })
	];
}

// ═════════════════════════════════════════════════════════════════════
// Lens Type Selection (Prime / Zoom)
// ═════════════════════════════════════════════════════════════════════

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildLensTypeSection()
{
	// Compact row: LENS label | Prime/Zoom checkboxes | spinbox(es)
	return SNew(SHorizontalBox)

	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
	[
		SNew(SBox).WidthOverride(60)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("LensLabel", "LENS"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
		]
	]

	// Prime chip
	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 4, 0)
	[
		SNew(SCheckBox)
		.Style(FAppStyle::Get(), "ToggleButtonCheckbox")
		.IsChecked_Lambda([this]() { return bUsePrimeLens ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
		.OnCheckStateChanged_Lambda([this](ECheckBoxState) { bUsePrimeLens = true; UpdateLensTypeVisibility(); })
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PrimeChip", "PRIME"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
		]
	]

	// Zoom chip
	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
	[
		SNew(SCheckBox)
		.Style(FAppStyle::Get(), "ToggleButtonCheckbox")
		.IsChecked_Lambda([this]() { return !bUsePrimeLens ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
		.OnCheckStateChanged_Lambda([this](ECheckBoxState) { bUsePrimeLens = false; UpdateLensTypeVisibility(); })
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ZoomChip", "ZOOM"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
		]
	]

	// Prime spinbox (focal length)
	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
	[
		SAssignNew(PrimeLensInputBox, SBox)
		.Visibility(bUsePrimeLens ? EVisibility::Visible : EVisibility::Collapsed)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 4, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PrimeFLLabel", "f"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SSpinBox<float>)
				.MinValue(8.0f).MaxValue(300.0f)
				.Value(PrimeLensFocalLengthMM)
				.OnValueChanged_Lambda([this](float Val) { PrimeLensFocalLengthMM = Val; })
				.MinDesiredWidth(60)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0, 0, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PrimeFLUnit", "mm"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
			]
		]
	]

	// Zoom spinboxes (min – max)
	+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
	[
		SAssignNew(ZoomLensInputBox, SBox)
		.Visibility(!bUsePrimeLens ? EVisibility::Visible : EVisibility::Collapsed)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SSpinBox<float>)
				.MinValue(8.0f).MaxValue(300.0f)
				.Value(FocalLengthMinMM)
				.OnValueChanged_Lambda([this](float Val) { FocalLengthMinMM = Val; })
				.MinDesiredWidth(55)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("–")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.4f, 0.4f)))
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SSpinBox<float>)
				.MinValue(8.0f).MaxValue(300.0f)
				.Value(FocalLengthMaxMM)
				.OnValueChanged_Lambda([this](float Val) { FocalLengthMaxMM = Val; })
				.MinDesiredWidth(55)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0, 0, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ZoomUnit", "mm"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
			]
		]
	];
}

// ═════════════════════════════════════════════════════════════════════
// Protocol Selection
// ═════════════════════════════════════════════════════════════════════

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildProtocolSection()
{
	return SNew(SHorizontalBox)

	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
	[
		SNew(SBox).WidthOverride(60)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ProtocolLabel", "PROTOCOL"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
		]
	]

	// FreeD chip
	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 4, 0)
	[
		SNew(SCheckBox)
		.Style(FAppStyle::Get(), "ToggleButtonCheckbox")
		.IsChecked_Lambda([this]() { return SelectedProtocol == ETrackingProtocol::FreeD ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
		.OnCheckStateChanged_Lambda([this](ECheckBoxState) { SelectedProtocol = ETrackingProtocol::FreeD; })
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FreeDChip", "FreeD 3D"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
		]
	]

	// OpenTrack chip
	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
	[
		SNew(SCheckBox)
		.Style(FAppStyle::Get(), "ToggleButtonCheckbox")
		.IsChecked_Lambda([this]() { return SelectedProtocol == ETrackingProtocol::OpenTrackIO ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
		.OnCheckStateChanged_Lambda([this](ECheckBoxState) { SelectedProtocol = ETrackingProtocol::OpenTrackIO; })
		[
			SNew(STextBlock)
			.Text(LOCTEXT("OpenTrackChip", "OpenTrack"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
		]
	];
}

// ═════════════════════════════════════════════════════════════════════
// Network / IP Display
// ═════════════════════════════════════════════════════════════════════

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildNetworkSection()
{
	return SNew(SHorizontalBox)

	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
	[
		SNew(SBox).WidthOverride(60)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NetworkLabel", "NETWORK"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
		]
	]

	+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
	[
		SNew(STextBlock)
		.Text_Lambda([this]() -> FText
		{
			return FText::FromString(FString::Printf(TEXT("%s : %d"), *LocalIPAddress, ListeningPort));
		})
		.Font(FCoreStyle::GetDefaultFontStyle("Mono", 10))
		.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
	]

	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0, 0, 0)
	[
		SNew(SButton)
		.Text(LOCTEXT("RefreshIP", "Refresh"))
		.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
		.TextStyle(FAppStyle::Get(), "SmallText")
		.OnClicked_Lambda([this]() -> FReply { DetectLocalIP(); return FReply::Handled(); })
	];
}

// ═════════════════════════════════════════════════════════════════════
// Setup Now Button
// ═════════════════════════════════════════════════════════════════════

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildSetupButton()
{
	return SNew(SBox).HeightOverride(36)
	[
		SNew(SButton)
		.Text(LOCTEXT("SetupNow", "SETUP NOW"))
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.OnClicked_Lambda([this]() -> FReply
		{
			RunOneClickSetup();
			SwitchTab(1); // Switch to calibration tab after setup
			return FReply::Handled();
		})
		.IsEnabled_Raw(this, &SFonixFlowTrackerSetupPanel::IsSetupButtonEnabled)
	];
}

// ═════════════════════════════════════════════════════════════════════
// Calibration Section (now a full tab)
// ═════════════════════════════════════════════════════════════════════

// Helper: compact capture row
static TSharedRef<SWidget> MakeCaptureRow(
	const FText& Label,
	TAttribute<FText> ValueAttr,
	TAttribute<FSlateColor> ValueColor,
	FOnClicked OnCapture,
	const FText& BtnLabel)
{
	return SNew(SHorizontalBox)

	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 4, 0)
	[
		SNew(SBox).WidthOverride(36)
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
		]
	]

	+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
	[
		SNew(STextBlock)
		.Text(ValueAttr)
		.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
		.ColorAndOpacity(ValueColor)
		.Justification(ETextJustify::Right)
	]

	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0, 0, 0)
	[
		SNew(SBox).MinDesiredWidth(90)
		[
			SNew(SButton)
			.Text(BtnLabel)
			.HAlign(HAlign_Center)
			.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
			.TextStyle(FAppStyle::Get(), "SmallText")
			.OnClicked(OnCapture)
		]
	];
}

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildCalibrationSection()
{
	return SNew(SVerticalBox)

	// Live readout row
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 8)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 20, 0)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(0, 0, 4, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LiveFocusLabel", "FOCUS"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetLiveLinkFocusText)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.8f, 0.4f)))
			]
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(0, 0, 4, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LiveZoomLabel", "ZOOM"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Raw(this, &SFonixFlowTrackerSetupPanel::GetLiveLinkZoomText)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.67f, 1.0f)))
			]
		]
	]

	// NEAR capture row
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
	[
		MakeCaptureRow(
			LOCTEXT("NearLabel", "NEAR"),
			TAttribute<FText>::CreateRaw(this, &SFonixFlowTrackerSetupPanel::GetFocusMinText),
			TAttribute<FSlateColor>::CreateLambda([this]() -> FSlateColor
			{
				return bFocusMinCaptured ? FSlateColor(FLinearColor::White) : FSlateColor(FLinearColor(0.8f, 0.53f, 0.2f));
			}),
			FOnClicked::CreateLambda([this]() -> FReply { CaptureFocusMin(); return FReply::Handled(); }),
			LOCTEXT("CaptureNear", "CAPTURE")
		)
	]

	// FAR capture row
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
	[
		MakeCaptureRow(
			LOCTEXT("FarLabel", "FAR"),
			TAttribute<FText>::CreateRaw(this, &SFonixFlowTrackerSetupPanel::GetFocusMaxText),
			TAttribute<FSlateColor>::CreateLambda([this]() -> FSlateColor
			{
				return bFocusMaxCaptured ? FSlateColor(FLinearColor::White) : FSlateColor(FLinearColor(0.8f, 0.53f, 0.2f));
			}),
			FOnClicked::CreateLambda([this]() -> FReply { CaptureFocusMax(); return FReply::Handled(); }),
			LOCTEXT("CaptureFar", "CAPTURE")
		)
	]

	// WIDE / TELE rows (zoom only)
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
	[
		SAssignNew(ZoomCalibBox, SBox)
		.Visibility(!bUsePrimeLens ? EVisibility::Visible : EVisibility::Collapsed)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
			[
				MakeCaptureRow(
					LOCTEXT("WideLabel", "WIDE"),
					TAttribute<FText>::CreateRaw(this, &SFonixFlowTrackerSetupPanel::GetZoomWideText),
					TAttribute<FSlateColor>::CreateLambda([this]() -> FSlateColor
					{
						return bZoomWideCaptured ? FSlateColor(FLinearColor::White) : FSlateColor(FLinearColor(0.8f, 0.53f, 0.2f));
					}),
					FOnClicked::CreateLambda([this]() -> FReply { CaptureZoomWide(); return FReply::Handled(); }),
					LOCTEXT("CaptureWide", "CAPTURE")
				)
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
			[
				MakeCaptureRow(
					LOCTEXT("TeleLabel", "TELE"),
					TAttribute<FText>::CreateRaw(this, &SFonixFlowTrackerSetupPanel::GetZoomTeleText),
					TAttribute<FSlateColor>::CreateLambda([this]() -> FSlateColor
					{
						return bZoomTeleCaptured ? FSlateColor(FLinearColor::White) : FSlateColor(FLinearColor(0.8f, 0.53f, 0.2f));
					}),
					FOnClicked::CreateLambda([this]() -> FReply { CaptureZoomTele(); return FReply::Handled(); }),
					LOCTEXT("CaptureTele", "CAPTURE")
				)
			]
		]
	]

	// Apply Calibration button
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
	]

	// Apply Lens File button — appears after calibration is applied
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 0)
	[
		SAssignNew(ApplyLensFileBox, SBox)
		.Visibility_Lambda([this]() -> EVisibility
		{
			return bCalibrationApplied ? EVisibility::Visible : EVisibility::Collapsed;
		})
		[
			SNew(SBox).HeightOverride(36)
			[
				SNew(SButton)
				.Text(LOCTEXT("ApplyLensFile", "APPLY LENS FILE"))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.OnClicked_Lambda([this]() -> FReply { ApplyCalibration(); return FReply::Handled(); })
			]
		]
	];
}

// ═════════════════════════════════════════════════════════════════════
// ACTIONS
// ═════════════════════════════════════════════════════════════════════

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
	FString Line = FString::Printf(TEXT("[%s] %s\n"), *Timestamp, *Message);
	FString LogFilePath = FPaths::ProjectLogDir() / TEXT("FonixFlowTracker.log");
	IFileManager::Get().MakeDirectory(*FPaths::ProjectLogDir(), true);
	FFileHelper::SaveStringToFile(Line, *LogFilePath,
		FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
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

	// ── Step 2: Create or reuse Live Link FreeD source ──────────────
	AddLog(TEXT("Step 2: Setting up FreeD source..."));

	ILiveLinkClient* LiveLinkClient = nullptr;
	{
		IModularFeatures& ModularFeatures = IModularFeatures::Get();
		if (ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
		{
			LiveLinkClient = &ModularFeatures.GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
		}
	}

	FGuid SourceGuid;

	// Reuse existing source if it is still registered in the LiveLink client
	if (ActiveSourceGuid.IsValid() && LiveLinkClient)
	{
		TArray<FGuid> ActiveSources = LiveLinkClient->GetSources();
		if (ActiveSources.Contains(ActiveSourceGuid))
		{
			SourceGuid = ActiveSourceGuid;
			AddLog(TEXT("  Existing FreeD source still active — skipping creation"));

			// Restart polling (was stopped at the top of RunOneClickSetup)
			bSubjectAutoAssigned = false;
			bLiveLinkPollingActive = true;
			UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
			if (World)
			{
				World->GetTimerManager().SetTimer(
					LiveLinkPollTimerHandle,
					FTimerDelegate::CreateSP(this, &SFonixFlowTrackerSetupPanel::PollLiveLinkData),
					0.05f, true);
				AddLog(TEXT("  LiveLink polling restarted (20Hz)"));
			}
		}
	}

	if (!SourceGuid.IsValid() && LiveLinkClient)
	{
		// Find the FreeD connection settings struct via reflection
		UScriptStruct* FreeDSettingsStruct = nullptr;
		{
			TArray<UObject*> Structs;
			GetObjectsOfClass(UScriptStruct::StaticClass(), Structs);
			for (UObject* Obj : Structs)
			{
				if (Obj && Obj->GetName() == TEXT("LiveLinkFreeDConnectionSettings"))
				{
					FreeDSettingsStruct = Cast<UScriptStruct>(Obj);
					break;
				}
			}
		}

		// Find the FreeD source factory
		ULiveLinkSourceFactory* FreeDFactory = nullptr;
		{
			TArray<UObject*> Factories;
			GetObjectsOfClass(ULiveLinkSourceFactory::StaticClass(), Factories);
			for (UObject* Obj : Factories)
			{
				ULiveLinkSourceFactory* Factory = Cast<ULiveLinkSourceFactory>(Obj);
				if (Factory)
				{
					FText Name = Factory->GetSourceDisplayName();
					if (Name.ToString().Contains(TEXT("FreeD")))
					{
						FreeDFactory = Factory;
						AddLog(FString::Printf(TEXT("  Factory: %s"), *Name.ToString()));
						break;
					}
				}
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
	else if (!LiveLinkClient)
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
	bCalibrationApplied = true;
}



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

// ═════════════════════════════════════════════════════════════════════
// IFonixFlowTrackerActions — AI Chat interface
// ═════════════════════════════════════════════════════════════════════

FFonixFlowTrackerState SFonixFlowTrackerSetupPanel::GetState() const
{
	FFonixFlowTrackerState State;

	// Camera
	State.bHasCamera = (SelectedCamera != nullptr && SelectedCamera->IsValidLowLevel());
	State.SelectedCamera = State.bHasCamera ? SelectedCamera->GetActorLabel() : TEXT("None");

	// Protocol
	State.Protocol = (SelectedProtocol == ETrackingProtocol::FreeD) ? TEXT("FreeD") : TEXT("OpenTrackIO");
	State.IPAddress = LocalIPAddress;
	State.Port = ListeningPort;

	// Lens
	State.LensType = bUsePrimeLens ? TEXT("Prime") : TEXT("Zoom");
	State.PrimeFocalLengthMM = PrimeLensFocalLengthMM;
	State.ZoomMinMM = FocalLengthMinMM;
	State.ZoomMaxMM = FocalLengthMaxMM;

	// Status
	State.bSetupComplete = bSetupComplete;
	State.bLiveLinkActive = bLiveLinkPollingActive;

	// Calibration
	State.bCalibrationApplied = bCalibrationApplied;
	State.bNearCaptured = bFocusMinCaptured;
	State.bFarCaptured = bFocusMaxCaptured;
	State.bWideCaptured = bZoomWideCaptured;
	State.bTeleCaptured = bZoomTeleCaptured;

	// Live values
	State.LiveFocusCM = LiveLinkFocusValue;
	State.LiveZoomMM = LiveLinkZoomValue;

	return State;
}

bool SFonixFlowTrackerSetupPanel::SelectCamera(const FString& CameraName)
{
	for (int32 i = 0; i < CameraWeakPtrs.Num(); i++)
	{
		ACineCameraActor* Cam = CameraWeakPtrs[i].Get();
		if (Cam && Cam->GetActorLabel() == CameraName)
		{
			SelectedCamera = Cam;
			if (CameraListView.IsValid())
			{
				CameraListView->SetSelection(CameraWeakPtrs[i]);
			}
			return true;
		}
	}
	return false;
}

TArray<FString> SFonixFlowTrackerSetupPanel::GetAvailableCameraNames() const
{
	TArray<FString> Names;
	for (const auto& WeakCam : CameraWeakPtrs)
	{
		ACineCameraActor* Cam = WeakCam.Get();
		if (Cam) Names.Add(Cam->GetActorLabel());
	}
	return Names;
}

bool SFonixFlowTrackerSetupPanel::SetProtocol(const FString& Protocol)
{
	if (Protocol == TEXT("FreeD"))
	{
		SelectedProtocol = ETrackingProtocol::FreeD;
		return true;
	}
	if (Protocol == TEXT("OpenTrackIO"))
	{
		SelectedProtocol = ETrackingProtocol::OpenTrackIO;
		return true;
	}
	return false;
}

bool SFonixFlowTrackerSetupPanel::SetLensType(const FString& LensType)
{
	if (LensType == TEXT("Prime"))
	{
		bUsePrimeLens = true;
		UpdateLensTypeVisibility();
		return true;
	}
	if (LensType == TEXT("Zoom"))
	{
		bUsePrimeLens = false;
		UpdateLensTypeVisibility();
		return true;
	}
	return false;
}

bool SFonixFlowTrackerSetupPanel::SetPrimeFocalLength(float MM)
{
	if (MM >= 8.0f && MM <= 300.0f)
	{
		PrimeLensFocalLengthMM = MM;
		return true;
	}
	return false;
}

bool SFonixFlowTrackerSetupPanel::SetZoomRange(float MinMM, float MaxMM)
{
	if (MinMM >= 8.0f && MaxMM <= 300.0f && MinMM < MaxMM)
	{
		FocalLengthMinMM = MinMM;
		FocalLengthMaxMM = MaxMM;
		return true;
	}
	return false;
}

FString SFonixFlowTrackerSetupPanel::RunSetup()
{
	if (!SelectedCamera || !SelectedCamera->IsValidLowLevel())
	{
		return TEXT("ERROR: No camera selected. Select a CineCameraActor first.");
	}
	RunOneClickSetup();
	if (bSetupSuccess)
	{
		return FString::Printf(TEXT("Setup complete for '%s'. Protocol: %s, Port: %d. Switch to Calibration tab to continue."),
			*SelectedCamera->GetActorLabel(), *GetState().Protocol, ListeningPort);
	}
	return TEXT("Setup failed. Check the log for details.");
}

FString SFonixFlowTrackerSetupPanel::CaptureCalibration(const FString& Target)
{
	if (!bSetupComplete || !bSetupSuccess)
	{
		return TEXT("ERROR: Run setup first before calibrating.");
	}

	if (Target == TEXT("Near"))
	{
		CaptureFocusMin();
		return FString::Printf(TEXT("Near captured: %.2f cm"), FocusEncoderMin);
	}
	if (Target == TEXT("Far"))
	{
		CaptureFocusMax();
		return FString::Printf(TEXT("Far captured: %.2f cm"), FocusEncoderMax);
	}
	if (Target == TEXT("Wide"))
	{
		if (bUsePrimeLens) return TEXT("ERROR: Wide capture is only for zoom lenses.");
		CaptureZoomWide();
		return FString::Printf(TEXT("Wide captured: %.2f mm"), ZoomEncoderWide);
	}
	if (Target == TEXT("Tele"))
	{
		if (bUsePrimeLens) return TEXT("ERROR: Tele capture is only for zoom lenses.");
		CaptureZoomTele();
		return FString::Printf(TEXT("Tele captured: %.2f mm"), ZoomEncoderTele);
	}

	return TEXT("ERROR: Invalid target. Use: Near, Far, Wide, or Tele.");
}

FString SFonixFlowTrackerSetupPanel::ApplyCalibrationAction()
{
	if (!IsCalibrationReady())
	{
		return TEXT("ERROR: Calibration not ready. Capture all required values first.");
	}
	ApplyCalibration();
	if (bCalibrationApplied)
	{
		return TEXT("Calibration applied successfully. Lens file created at /Game/FonixFlowTrackerSetup/TrackedLens.");
	}
	return TEXT("Calibration failed. Check the log for details.");
}
