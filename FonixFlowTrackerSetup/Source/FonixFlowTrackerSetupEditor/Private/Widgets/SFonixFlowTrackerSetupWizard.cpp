// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "Widgets/SFonixFlowTrackerSetupWizard.h"
#include "FonixFlowTrackerSetupSubsystem.h"
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
#include "CineCameraActor.h"
#include "CineCameraComponent.h"
#include "LensFile.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Editor.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "SFonixFlowTrackerSetupWizard"

void SFonixFlowTrackerSetupWizard::Construct(const FArguments& InArgs)
{
	OnSetupComplete = InArgs._OnSetupComplete;

	// Initialize sensor presets
	SensorPresets = ULensSetupUtility::GetSensorPresets();

	// Build cached sensor options for combo box
	for (const FVector2D& Size : SensorPresets)
	{
		FString Name = ULensSetupUtility::GetSensorPresetName(Size);
		CachedSensorOptions.Add(MakeShareable(new FString(FString::Printf(TEXT("%s (%.1fx%.1f mm)"), *Name, Size.X, Size.Y))));
	}

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(16)
		[
			SNew(SVerticalBox)

			// Header
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("WizardTitle", "FonixFlow Tracker Setup Wizard"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 4)
			[
				SNew(STextBlock)
				.Text_Raw(this, &SFonixFlowTrackerSetupWizard::GetStepNumberText)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 4)
			[
				SNew(STextBlock)
				.Text_Raw(this, &SFonixFlowTrackerSetupWizard::GetStepTitle)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 12)
			[
				SNew(STextBlock)
				.Text_Raw(this, &SFonixFlowTrackerSetupWizard::GetStepDescription)
				.AutoWrapText(true)
				.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(SSeparator)
			]

			// Step content
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("NoBorder"))
					.Padding(0, 8)
					[
						SNew(SBox)
						.MinDesiredHeight(300)
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
							.Padding(16)
							[
								GetCurrentStepContent()
							]
						]
					]
				]
			]

			// Navigation buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 8, 0, 0)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("Back", "← Back"))
					.OnClicked_Lambda([this]() -> FReply
					{
						GoToPreviousStep();
						return FReply::Handled();
					})
					.IsEnabled_Raw(this, &SFonixFlowTrackerSetupWizard::CanGoPrevious)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("StepDots", "● ● ● ● ● ● ● ● ●"))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.3f, 0.3f, 0.3f)))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text_Lambda([this]() -> FText
					{
						return CurrentStep == EWizardStep::Review
							? LOCTEXT("Apply", "Apply Setup")
							: LOCTEXT("Next", "Next →");
					})
					.OnClicked_Lambda([this]() -> FReply
					{
						if (CurrentStep == EWizardStep::Review)
						{
							ApplySetup();
						}
						else
						{
							GoToNextStep();
						}
						return FReply::Handled();
					})
					.IsEnabled_Raw(this, &SFonixFlowTrackerSetupWizard::CanGoNext)
				]
			]
		]
	];
}

void SFonixFlowTrackerSetupWizard::GoToNextStep()
{
	if (CurrentStep < EWizardStep::Complete)
	{
		CurrentStep = static_cast<EWizardStep>(static_cast<uint8>(CurrentStep) + 1);

		if (CurrentStep == EWizardStep::Camera)
		{
			RefreshCameraList();
		}
		else if (CurrentStep == EWizardStep::LensSelection)
		{
			RefreshLensFileList();
		}
	}
}

void SFonixFlowTrackerSetupWizard::GoToPreviousStep()
{
	if (CurrentStep > EWizardStep::Protocol)
	{
		CurrentStep = static_cast<EWizardStep>(static_cast<uint8>(CurrentStep) - 1);
	}
}

bool SFonixFlowTrackerSetupWizard::CanGoNext() const
{
	return CurrentStep < EWizardStep::Complete;
}

bool SFonixFlowTrackerSetupWizard::CanGoPrevious() const
{
	return CurrentStep > EWizardStep::Protocol;
}

TSharedRef<SWidget> SFonixFlowTrackerSetupWizard::GetCurrentStepContent()
{
	switch (CurrentStep)
	{
	case EWizardStep::Protocol:        return BuildProtocolStep();
	case EWizardStep::Network:         return BuildNetworkStep();
	case EWizardStep::Camera:          return BuildCameraStep();
	case EWizardStep::LensSelection:   return BuildLensSelectionStep();
	case EWizardStep::LensCalibration: return BuildLensCalibrationStep();
	case EWizardStep::SensorSettings:  return BuildSensorSettingsStep();
	case EWizardStep::AnchorPoint:     return BuildAnchorPointStep();
	case EWizardStep::Review:          return BuildReviewStep();
	case EWizardStep::Complete:        return BuildCompleteStep();
	default:                           return SNew(STextBlock).Text(LOCTEXT("Unknown", "Unknown step"));
	}
}

FText SFonixFlowTrackerSetupWizard::GetStepTitle() const
{
	switch (CurrentStep)
	{
	case EWizardStep::Protocol:        return LOCTEXT("Step1Title", "Select Tracking Protocol");
	case EWizardStep::Network:         return LOCTEXT("Step2Title", "Network Configuration");
	case EWizardStep::Camera:          return LOCTEXT("Step3Title", "Camera Selection");
	case EWizardStep::LensSelection:   return LOCTEXT("Step4Title", "Lens File Selection");
	case EWizardStep::LensCalibration: return LOCTEXT("Step5Title", "Lens Encoder Calibration");
	case EWizardStep::SensorSettings:  return LOCTEXT("Step6Title", "Sensor & Filmback Settings");
	case EWizardStep::AnchorPoint:     return LOCTEXT("Step7Title", "Anchor Point Setup");
	case EWizardStep::Review:          return LOCTEXT("Step8Title", "Review & Apply");
	case EWizardStep::Complete:        return LOCTEXT("Step9Title", "Setup Complete");
	default:                           return FText::GetEmpty();
	}
}

FText SFonixFlowTrackerSetupWizard::GetStepDescription() const
{
	switch (CurrentStep)
	{
	case EWizardStep::Protocol:
		return LOCTEXT("Step1Desc", "Choose the tracking protocol your camera system uses.");
	case EWizardStep::Network:
		return LOCTEXT("Step2Desc", "Configure the network connection to your tracking system.");
	case EWizardStep::Camera:
		return LOCTEXT("Step3Desc", "Select an existing CineCameraActor or create a new one.");
	case EWizardStep::LensSelection:
		return LOCTEXT("Step4Desc", "Select an existing lens file from your project, or create a new one. A lens file maps raw encoder values to physical lens properties (focus distance, focal length).");
	case EWizardStep::LensCalibration:
		return LOCTEXT("Step5Desc", "Calibrate the lens encoder ranges. Physically rotate each lens control to its MINIMUM and MAXIMUM positions and capture the raw encoder values. This creates the mapping between encoder data and physical lens positions.");
	case EWizardStep::SensorSettings:
		return LOCTEXT("Step6Desc", "Configure the camera sensor dimensions and filmback. These affect field of view and lens distortion calculations.");
	case EWizardStep::AnchorPoint:
		return LOCTEXT("Step7Desc", "Configure the tracking anchor point (origin for all tracking data).");
	case EWizardStep::Review:
		return LOCTEXT("Step8Desc", "Review your settings before applying.");
	case EWizardStep::Complete:
		return LOCTEXT("Step9Desc", "Tracking setup is complete!");
	default:
		return FText::GetEmpty();
	}
}

FText SFonixFlowTrackerSetupWizard::GetStepNumberText() const
{
	int32 StepNum = static_cast<uint8>(CurrentStep) + 1;
	return FText::Format(LOCTEXT("StepOf", "Step {0} of 9"), FText::AsNumber(StepNum));
}

// Step 1: Protocol Selection (same as before)
TSharedRef<SWidget> SFonixFlowTrackerSetupWizard::BuildProtocolStep()
{
	static TArray<TSharedPtr<FString>> RigOptions = {
		MakeShareable(new FString(TEXT("Generic"))),
		MakeShareable(new FString(TEXT("Panasonic"))),
		MakeShareable(new FString(TEXT("Sony"))),
		MakeShareable(new FString(TEXT("stYpe"))),
		MakeShareable(new FString(TEXT("Mosys"))),
		MakeShareable(new FString(TEXT("Ncam")))
	};

	return SNew(SVerticalBox)

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 16)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("ProtocolLabel", "Tracking Protocol:"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
	]

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 8)
	[
		SNew(SBorder)
		.BorderImage(ConnectionSettings.Protocol == ETrackingProtocol::FreeD
			? FAppStyle::GetBrush("ToolPanel.GroupBorder") : FAppStyle::GetBrush("NoBorder"))
		.Padding(12)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(ConnectionSettings.Protocol == ETrackingProtocol::FreeD ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState) { ConnectionSettings.Protocol = ETrackingProtocol::FreeD; })
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8, 0, 0, 0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[ SNew(STextBlock).Text(LOCTEXT("FreeDName", "FreeD Protocol")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
				+ SVerticalBox::Slot().AutoHeight()
				[ SNew(STextBlock).Text(LOCTEXT("FreeDDesc", "Industry standard for camera tracking. Default port: 40000 UDP.")).AutoWrapText(true) ]
			]
		]
	]

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 8)
	[
		SNew(SBorder)
		.BorderImage(ConnectionSettings.Protocol == ETrackingProtocol::OpenTrackIO
			? FAppStyle::GetBrush("ToolPanel.GroupBorder") : FAppStyle::GetBrush("NoBorder"))
		.Padding(12)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(ConnectionSettings.Protocol == ETrackingProtocol::OpenTrackIO ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState) { ConnectionSettings.Protocol = ETrackingProtocol::OpenTrackIO; })
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8, 0, 0, 0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[ SNew(STextBlock).Text(LOCTEXT("OpenTrackName", "OpenTrack IO")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
				+ SVerticalBox::Slot().AutoHeight()
				[ SNew(STextBlock).Text(LOCTEXT("OpenTrackDesc", "Open protocol. Multicast: 235.135.1.[Source]. Port: 55555.")).AutoWrapText(true) ]
			]
		]
	]

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 16, 0, 0)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("RigPresetLabel", "Camera Rig Preset:"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
	]

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 8, 0, 0)
	[
		SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&RigOptions)
		.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewValue, ESelectInfo::Type)
		{
			if (!NewValue.IsValid()) return;
			if (*NewValue == TEXT("Generic")) ConnectionSettings.RigPreset = ECameraRigPreset::Generic;
			else if (*NewValue == TEXT("Panasonic")) ConnectionSettings.RigPreset = ECameraRigPreset::Panasonic;
			else if (*NewValue == TEXT("Sony")) ConnectionSettings.RigPreset = ECameraRigPreset::Sony;
			else if (*NewValue == TEXT("stYpe")) ConnectionSettings.RigPreset = ECameraRigPreset::Stype;
			else if (*NewValue == TEXT("Mosys")) ConnectionSettings.RigPreset = ECameraRigPreset::Mosys;
			else if (*NewValue == TEXT("Ncam")) ConnectionSettings.RigPreset = ECameraRigPreset::Ncam;
		})
		.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item) -> TSharedRef<SWidget>
		{ return SNew(STextBlock).Text(FText::FromString(*Item)); })
		.InitiallySelectedItem(MakeShareable(new FString(TEXT("Generic"))))
		[
			SNew(STextBlock)
			.Text_Lambda([this]() -> FText
			{
				switch (ConnectionSettings.RigPreset)
				{
				case ECameraRigPreset::Panasonic: return FText::FromString(TEXT("Panasonic"));
				case ECameraRigPreset::Sony: return FText::FromString(TEXT("Sony"));
				case ECameraRigPreset::Stype: return FText::FromString(TEXT("stYpe"));
				case ECameraRigPreset::Mosys: return FText::FromString(TEXT("Mosys"));
				case ECameraRigPreset::Ncam: return FText::FromString(TEXT("Ncam"));
				default: return FText::FromString(TEXT("Generic"));
				}
			})
		]
	];
}

// Step 2: Network (same as before, abbreviated)
TSharedRef<SWidget> SFonixFlowTrackerSetupWizard::BuildNetworkStep()
{
	TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);

	if (ConnectionSettings.Protocol == ETrackingProtocol::FreeD)
	{
		Content->AddSlot().AutoHeight().Padding(0, 0, 0, 12)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[ SNew(STextBlock).Text(LOCTEXT("IPLabel", "Tracking Source IP:")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
			+ SVerticalBox::Slot().AutoHeight()
			[ SNew(SEditableTextBox).Text(FText::FromString(ConnectionSettings.IPAddress))
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) { ConnectionSettings.IPAddress = Text.ToString(); })
				.HintText(LOCTEXT("IPHint", "e.g., 192.168.1.100")) ]
		];
		Content->AddSlot().AutoHeight().Padding(0, 0, 0, 12)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[ SNew(STextBlock).Text(LOCTEXT("PortLabel", "UDP Port:")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
			+ SVerticalBox::Slot().AutoHeight()
			[ SNew(SEditableTextBox).Text(FText::AsNumber(ConnectionSettings.FreeDPort))
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
				{ if (Text.ToString().IsNumeric()) ConnectionSettings.FreeDPort = FCString::Atoi(*Text.ToString()); })
				.HintText(LOCTEXT("PortHint", "Default: 40000")) ]
		];
	}
	else if (ConnectionSettings.Protocol == ETrackingProtocol::OpenTrackIO)
	{
		Content->AddSlot().AutoHeight().Padding(0, 0, 0, 12)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[ SNew(STextBlock).Text(LOCTEXT("SourceNumLabel", "Source Number (1-200):")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
			+ SVerticalBox::Slot().AutoHeight()
			[ SNew(SEditableTextBox).Text(FText::AsNumber(ConnectionSettings.OpenTrackSourceNumber))
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
				{ if (Text.ToString().IsNumeric()) ConnectionSettings.OpenTrackSourceNumber = FMath::Clamp(FCString::Atoi(*Text.ToString()), 1, 200); }) ]
		];
		Content->AddSlot().AutoHeight().Padding(0, 0, 0, 12)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[ SNew(SCheckBox).IsChecked(ConnectionSettings.bOpenTrackUseMulticast ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState State) { ConnectionSettings.bOpenTrackUseMulticast = (State == ECheckBoxState::Checked); }) ]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8, 0, 0, 0)
			[ SNew(STextBlock).Text(LOCTEXT("UseMulticast", "Use Multicast (recommended)")) ]
		];
	}

	Content->AddSlot().AutoHeight().Padding(0, 16, 0, 0)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ SNew(STextBlock).Text(LOCTEXT("SubjectLabel", "Live Link Subject Name:")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SEditableTextBox).Text(FText::FromString(ConnectionSettings.SubjectName))
			.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) { ConnectionSettings.SubjectName = Text.ToString(); }) ]
	];

	return Content;
}

// Step 3: Camera Selection (same as before)
TSharedRef<SWidget> SFonixFlowTrackerSetupWizard::BuildCameraStep()
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[ SNew(SCheckBox).IsChecked(CameraConfig.bCreateNewCamera ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State) { CameraConfig.bCreateNewCamera = (State == ECheckBoxState::Checked); }) ]
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8, 0, 0, 0)
		[ SNew(STextBlock).Text(LOCTEXT("CreateNew", "Create new CineCameraActor")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[ SNew(SCheckBox).IsChecked(!CameraConfig.bCreateNewCamera ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State) { CameraConfig.bCreateNewCamera = (State != ECheckBoxState::Checked); }) ]
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8, 0, 0, 0)
		[ SNew(STextBlock).Text(LOCTEXT("UseExisting", "Use existing CineCameraActor")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
	[
		SNew(SVerticalBox)
		.Visibility(this, &SFonixFlowTrackerSetupWizard::GetCreateCameraVisibility)
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ SNew(STextBlock).Text(LOCTEXT("CameraNameLabel", "Camera Name:")) ]
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SEditableTextBox).Text(FText::FromString(CameraConfig.CameraName))
			.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) { CameraConfig.CameraName = Text.ToString(); }) ]
	]
	+ SVerticalBox::Slot().FillHeight(1.0f)
	[
		SNew(SVerticalBox)
		.Visibility(this, &SFonixFlowTrackerSetupWizard::GetExistingCameraVisibility)
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ SNew(STextBlock).Text(LOCTEXT("SelectCamera", "Select Camera:")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ SNew(SButton).Text(LOCTEXT("RefreshCameras", "Refresh"))
			.OnClicked_Lambda([this]() -> FReply { RefreshCameraList(); return FReply::Handled(); }) ]
		+ SVerticalBox::Slot().FillHeight(1.0f)
		[ SAssignNew(CameraListView, SListView<TWeakObjectPtr<ACineCameraActor>>)
			.ListItemsSource(&AvailableCameraWeakPtrs)
			.OnGenerateRow_Lambda([](TWeakObjectPtr<ACineCameraActor> Item, const TSharedRef<STableViewBase>& OwnerTable) -> TSharedRef<ITableRow>
			{
				FString Name = Item.IsValid() ? Item->GetActorLabel() : TEXT("Invalid");
				return SNew(STableRow<TWeakObjectPtr<ACineCameraActor>>, OwnerTable)
				[ SNew(STextBlock).Text(FText::FromString(Name)) ];
			})
			.OnSelectionChanged_Lambda([this](TWeakObjectPtr<ACineCameraActor> Item, ESelectInfo::Type)
			{
				if (Item.IsValid()) { CameraConfig.ExistingCamera = Item.Get(); CameraConfig.CameraName = Item->GetActorLabel(); }
			})
			.SelectionMode(ESelectionMode::Single) ]
	];
}

// Step 4: Lens File Selection (NEW)
TSharedRef<SWidget> SFonixFlowTrackerSetupWizard::BuildLensSelectionStep()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[ SNew(SCheckBox).IsChecked(CameraConfig.LensConfig.bCreateNewLensFile ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State) { CameraConfig.LensConfig.bCreateNewLensFile = (State == ECheckBoxState::Checked); }) ]
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8, 0, 0, 0)
		[ SNew(STextBlock).Text(LOCTEXT("CreateNewLens", "Create new lens file")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
	]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[ SNew(SCheckBox).IsChecked(!CameraConfig.LensConfig.bCreateNewLensFile ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State) { CameraConfig.LensConfig.bCreateNewLensFile = (State != ECheckBoxState::Checked); }) ]
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8, 0, 0, 0)
		[ SNew(STextBlock).Text(LOCTEXT("UseExistingLens", "Use existing lens file from project")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
	]

	// New lens file name
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
	[
		SNew(SVerticalBox)
		.Visibility(this, &SFonixFlowTrackerSetupWizard::GetCreateLensVisibility)
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ SNew(STextBlock).Text(LOCTEXT("LensNameLabel", "Lens File Name:")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
		[ SNew(SEditableTextBox).Text(FText::FromString(CameraConfig.LensConfig.LensFileName))
			.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) { CameraConfig.LensConfig.LensFileName = Text.ToString(); }) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ SNew(STextBlock).Text(LOCTEXT("LensModelLabel", "Lens Model Name (e.g., Cooke S4 50mm):")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
		[ SNew(SEditableTextBox).Text(FText::FromString(CameraConfig.LensConfig.LensModelName))
			.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) { CameraConfig.LensConfig.LensModelName = Text.ToString(); })
			.HintText(LOCTEXT("LensModelHint", "e.g., Cooke S4 50mm")) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ SNew(STextBlock).Text(LOCTEXT("LensSerialLabel", "Serial Number (optional):")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SEditableTextBox).Text(FText::FromString(CameraConfig.LensConfig.LensSerialNumber))
			.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) { CameraConfig.LensConfig.LensSerialNumber = Text.ToString(); }) ]
	]

	// Existing lens file picker
	+ SVerticalBox::Slot().FillHeight(1.0f)
	[
		SNew(SVerticalBox)
		.Visibility(this, &SFonixFlowTrackerSetupWizard::GetExistingLensVisibility)
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ SNew(STextBlock).Text(LOCTEXT("SelectLens", "Select Lens File:")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ SNew(SButton).Text(LOCTEXT("RefreshLens", "Refresh"))
			.OnClicked_Lambda([this]() -> FReply { RefreshLensFileList(); return FReply::Handled(); }) ]
		+ SVerticalBox::Slot().FillHeight(1.0f)
		[ SAssignNew(LensFileListView, SListView<TWeakObjectPtr<ULensFile>>)
			.ListItemsSource(&AvailableLensFileWeakPtrs)
			.OnGenerateRow_Lambda([](TWeakObjectPtr<ULensFile> Item, const TSharedRef<STableViewBase>& OwnerTable) -> TSharedRef<ITableRow>
			{
				FString Name = Item.IsValid() ? Item->GetName() : TEXT("Invalid");
				FString Model = Item.IsValid() ? Item->LensInfo.LensModelName : TEXT("");
				FString Display = Model.IsEmpty() ? Name : FString::Printf(TEXT("%s (%s)"), *Name, *Model);
				return SNew(STableRow<TWeakObjectPtr<ULensFile>>, OwnerTable)
				[ SNew(STextBlock).Text(FText::FromString(Display)) ];
			})
			.OnSelectionChanged_Lambda([this](TWeakObjectPtr<ULensFile> Item, ESelectInfo::Type)
			{
				if (Item.IsValid()) { CameraConfig.LensConfig.LensFile = Item.Get(); }
			})
			.SelectionMode(ESelectionMode::Single) ]
	]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("LensNote", "A lens file maps raw encoder values from the tracking system to physical lens properties. You can create a new one with default values, or use an existing calibrated lens file."))
		.AutoWrapText(true)
		.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
	]
	;
}

// Step 5: Lens Encoder Calibration (updated with new types)
TSharedRef<SWidget> SFonixFlowTrackerSetupWizard::BuildLensCalibrationStep()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 16)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("CalibInstructions",
			"Calibrate the lens encoder ranges. Physically rotate each lens to its MINIMUM and MAXIMUM positions.\n\n"
			"For each control (Focus, Zoom):\n"
			"1. Rotate to MINIMUM → Capture Min\n"
			"2. Rotate to MAXIMUM → Capture Max\n\n"
			"This maps raw encoder values to physical lens values."))
		.AutoWrapText(true)
	]

	// Focus Distance Calibration
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 16)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(12)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
			[ SNew(STextBlock).Text(LOCTEXT("FocusCalib", "Focus Distance Encoder")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f)
				[ SNew(STextBlock).Text(FText::Format(LOCTEXT("FocusMin", "Min Encoder: {0}"), FText::AsNumber(CameraConfig.LensConfig.FocusEncoderRange.RawMin))) ]
				+ SHorizontalBox::Slot().AutoWidth()
				[ SNew(SButton).Text(LOCTEXT("CaptureFocusMin", "Capture Min"))
					.OnClicked_Lambda([this]() -> FReply
					{
						CameraConfig.LensConfig.FocusEncoderRange.RawMin = 0;
						CameraConfig.LensConfig.FocusEncoderRange.bIsCalibrated = true;
						return FReply::Handled();
					}) ]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f)
				[ SNew(STextBlock).Text(FText::Format(LOCTEXT("FocusMax", "Max Encoder: {0}"), FText::AsNumber(CameraConfig.LensConfig.FocusEncoderRange.RawMax))) ]
				+ SHorizontalBox::Slot().AutoWidth()
				[ SNew(SButton).Text(LOCTEXT("CaptureFocusMax", "Capture Max"))
					.OnClicked_Lambda([this]() -> FReply
					{
						CameraConfig.LensConfig.FocusEncoderRange.RawMax = 0x00FFFFFF;
						CameraConfig.LensConfig.FocusEncoderRange.bIsCalibrated = true;
						return FReply::Handled();
					}) ]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[ SNew(STextBlock).Text(LOCTEXT("FocusPhysicalRange", "Physical Range (cm):")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0, 0, 4, 0)
				[ SNew(SEditableTextBox).Text(FText::FromString(FString::SanitizeFloat(CameraConfig.LensConfig.FocusDistanceMinCM)))
					.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) { CameraConfig.LensConfig.FocusDistanceMinCM = FCString::Atof(*Text.ToString()); })
					.HintText(LOCTEXT("FocusMinCM", "Min cm")) ]
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(4, 0, 0, 0)
				[ SNew(SEditableTextBox).Text(FText::FromString(FString::SanitizeFloat(CameraConfig.LensConfig.FocusDistanceMaxCM)))
					.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) { CameraConfig.LensConfig.FocusDistanceMaxCM = FCString::Atof(*Text.ToString()); })
					.HintText(LOCTEXT("FocusMaxCM", "Max cm")) ]
			]
		]
	]

	// Zoom/Focal Length Calibration
	+ SVerticalBox::Slot().AutoHeight()
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(12)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
			[ SNew(STextBlock).Text(LOCTEXT("ZoomCalib", "Focal Length (Zoom) Encoder")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f)
				[ SNew(STextBlock).Text(FText::Format(LOCTEXT("ZoomMin", "Min Encoder: {0}"), FText::AsNumber(CameraConfig.LensConfig.ZoomEncoderRange.RawMin))) ]
				+ SHorizontalBox::Slot().AutoWidth()
				[ SNew(SButton).Text(LOCTEXT("CaptureZoomMin", "Capture Min"))
					.OnClicked_Lambda([this]() -> FReply
					{
						CameraConfig.LensConfig.ZoomEncoderRange.RawMin = 0;
						CameraConfig.LensConfig.ZoomEncoderRange.bIsCalibrated = true;
						return FReply::Handled();
					}) ]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f)
				[ SNew(STextBlock).Text(FText::Format(LOCTEXT("ZoomMax", "Max Encoder: {0}"), FText::AsNumber(CameraConfig.LensConfig.ZoomEncoderRange.RawMax))) ]
				+ SHorizontalBox::Slot().AutoWidth()
				[ SNew(SButton).Text(LOCTEXT("CaptureZoomMax", "Capture Max"))
					.OnClicked_Lambda([this]() -> FReply
					{
						CameraConfig.LensConfig.ZoomEncoderRange.RawMax = 0x00FFFFFF;
						CameraConfig.LensConfig.ZoomEncoderRange.bIsCalibrated = true;
						return FReply::Handled();
					}) ]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[ SNew(STextBlock).Text(LOCTEXT("ZoomPhysicalRange", "Physical Range (mm):")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0, 0, 4, 0)
				[ SNew(SEditableTextBox).Text(FText::FromString(FString::SanitizeFloat(CameraConfig.LensConfig.FocalLengthMinMM)))
					.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) { CameraConfig.LensConfig.FocalLengthMinMM = FCString::Atof(*Text.ToString()); })
					.HintText(LOCTEXT("ZoomMinMM", "Min mm")) ]
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(4, 0, 0, 0)
				[ SNew(SEditableTextBox).Text(FText::FromString(FString::SanitizeFloat(CameraConfig.LensConfig.FocalLengthMaxMM)))
					.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) { CameraConfig.LensConfig.FocalLengthMaxMM = FCString::Atof(*Text.ToString()); })
					.HintText(LOCTEXT("ZoomMaxMM", "Max mm")) ]
			]
		]
	]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("CalibNote", "Encoder values come from the Live Link data stream. Make sure tracking data is flowing before capturing."))
		.AutoWrapText(true).ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
	]
	;
}

// Step 6: Sensor Settings (NEW)
TSharedRef<SWidget> SFonixFlowTrackerSetupWizard::BuildSensorSettingsStep()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 16)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("SensorDesc", "Configure the camera sensor dimensions. These affect field of view calculations and lens distortion mapping."))
		.AutoWrapText(true)
	]

	// Sensor preset selector
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ SNew(STextBlock).Text(LOCTEXT("SensorPreset", "Sensor Preset:")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SComboBox<TSharedPtr<FString>>)
			.OptionsSource(&CachedSensorOptions)
			.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewValue, ESelectInfo::Type)
			{
				if (NewValue.IsValid())
				{
					// Find matching preset
					for (int32 i = 0; i < SensorPresets.Num(); i++)
					{
						FString Name = ULensSetupUtility::GetSensorPresetName(SensorPresets[i]);
						if (NewValue->StartsWith(Name))
						{
							CameraConfig.LensConfig.SensorWidthMM = SensorPresets[i].X;
							CameraConfig.LensConfig.SensorHeightMM = SensorPresets[i].Y;
							CameraConfig.LensConfig.SensorAspectRatio = SensorPresets[i].X / SensorPresets[i].Y;
							CameraConfig.LensConfig.SensorDimensions = SensorPresets[i];
							break;
						}
					}
				}
			})
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item) -> TSharedRef<SWidget>
			{ return SNew(STextBlock).Text(FText::FromString(*Item)); })
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%s (%.1fx%.1f mm)"),
					*ULensSetupUtility::GetSensorPresetName(FVector2D(CameraConfig.LensConfig.SensorWidthMM, CameraConfig.LensConfig.SensorHeightMM)),
					CameraConfig.LensConfig.SensorWidthMM, CameraConfig.LensConfig.SensorHeightMM)))
			]
		]
	]

	// Manual sensor dimensions
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ SNew(STextBlock).Text(LOCTEXT("SensorDims", "Sensor Dimensions (mm):")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0, 0, 4, 0)
			[ SNew(STextBlock).Text(LOCTEXT("Width", "Width:")) ]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(4, 0, 4, 0)
			[ SNew(SEditableTextBox).Text(FText::FromString(FString::SanitizeFloat(CameraConfig.LensConfig.SensorWidthMM)))
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
				{
					CameraConfig.LensConfig.SensorWidthMM = FCString::Atof(*Text.ToString());
					CameraConfig.LensConfig.SensorDimensions.X = CameraConfig.LensConfig.SensorWidthMM;
				}) ]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(4, 0, 4, 0)
			[ SNew(STextBlock).Text(LOCTEXT("Height", "Height:")) ]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(4, 0, 0, 0)
			[ SNew(SEditableTextBox).Text(FText::FromString(FString::SanitizeFloat(CameraConfig.LensConfig.SensorHeightMM)))
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
				{
					CameraConfig.LensConfig.SensorHeightMM = FCString::Atof(*Text.ToString());
					CameraConfig.LensConfig.SensorDimensions.Y = CameraConfig.LensConfig.SensorHeightMM;
				}) ]
		]
	]

	// Squeeze factor
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ SNew(STextBlock).Text(LOCTEXT("SqueezeFactor", "Squeeze Factor (1.0 for spherical, 2.0 for 2x anamorphic):")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
		+ SVerticalBox::Slot().AutoHeight()
		[ SNew(SEditableTextBox).Text(FText::FromString(FString::SanitizeFloat(CameraConfig.LensConfig.SqueezeFactor)))
			.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) { CameraConfig.LensConfig.SqueezeFactor = FCString::Atof(*Text.ToString()); }) ]
	]

	// Image resolution
	+ SVerticalBox::Slot().AutoHeight()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
		[ SNew(STextBlock).Text(LOCTEXT("ImageRes", "Image Resolution:")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0, 0, 4, 0)
			[ SNew(SEditableTextBox).Text(FText::AsNumber(CameraConfig.LensConfig.ImageDimensions.X))
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
				{ if (Text.ToString().IsNumeric()) CameraConfig.LensConfig.ImageDimensions.X = FCString::Atoi(*Text.ToString()); })
				.HintText(LOCTEXT("ResX", "Width")) ]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(4, 0, 0, 0)
			[ SNew(SEditableTextBox).Text(FText::AsNumber(CameraConfig.LensConfig.ImageDimensions.Y))
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
				{ if (Text.ToString().IsNumeric()) CameraConfig.LensConfig.ImageDimensions.Y = FCString::Atoi(*Text.ToString()); })
				.HintText(LOCTEXT("ResY", "Height")) ]
		]
	]
	;
}

// Step 7: Anchor Point (same as before)
TSharedRef<SWidget> SFonixFlowTrackerSetupWizard::BuildAnchorPointStep()
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[ SNew(SCheckBox).IsChecked(CameraConfig.bCreateAnchorPoint ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State) { CameraConfig.bCreateAnchorPoint = (State == ECheckBoxState::Checked); }) ]
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8, 0, 0, 0)
		[ SNew(STextBlock).Text(LOCTEXT("CreateAnchor", "Create tracking anchor point")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
	[ SNew(STextBlock).Text(LOCTEXT("AnchorDesc", "The anchor point is the origin for all tracking data. Set position to match your physical tracking system origin.")).AutoWrapText(true) ]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
	[
		SNew(SBox).Visibility(this, &SFonixFlowTrackerSetupWizard::GetAnchorPointVisibility)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[ SNew(STextBlock).Text(LOCTEXT("AnchorPos", "Position (X, Y, Z):")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0, 0, 4, 0)
				[ SNew(SEditableTextBox).Text(FText::FromString(FString::SanitizeFloat(CameraConfig.AnchorLocation.X)))
					.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) { CameraConfig.AnchorLocation.X = FCString::Atof(*Text.ToString()); }).HintText(LOCTEXT("X", "X")) ]
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(4, 0, 4, 0)
				[ SNew(SEditableTextBox).Text(FText::FromString(FString::SanitizeFloat(CameraConfig.AnchorLocation.Y)))
					.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) { CameraConfig.AnchorLocation.Y = FCString::Atof(*Text.ToString()); }).HintText(LOCTEXT("Y", "Y")) ]
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(4, 0, 0, 0)
				[ SNew(SEditableTextBox).Text(FText::FromString(FString::SanitizeFloat(CameraConfig.AnchorLocation.Z)))
					.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type) { CameraConfig.AnchorLocation.Z = FCString::Atof(*Text.ToString()); }).HintText(LOCTEXT("Z", "Z")) ]
			]
		]
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 16, 0, 0)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[ SNew(SCheckBox).IsChecked(CameraConfig.bEnableVirtualCamera ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State) { CameraConfig.bEnableVirtualCamera = (State == ECheckBoxState::Checked); }) ]
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8, 0, 0, 0)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[ SNew(STextBlock).Text(LOCTEXT("EnableVCam", "Enable Virtual Camera")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]
			+ SVerticalBox::Slot().AutoHeight()
			[ SNew(STextBlock).Text(LOCTEXT("VCamDesc", "Wire tracking into Virtual Camera system.")).AutoWrapText(true).ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f))) ]
		]
	]
	;
}

// Step 8: Review (updated with lens info)
TSharedRef<SWidget> SFonixFlowTrackerSetupWizard::BuildReviewStep()
{
	auto AddRow = [](SVerticalBox& Parent, const FText& Label, const FText& Value)
	{
		Parent.AddSlot().AutoHeight().Padding(0, 0, 0, 8)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[ SNew(STextBlock).Text(Label).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[ SNew(STextBlock).Text(Value) ]
		];
	};

	TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);

	Content->AddSlot().AutoHeight().Padding(0, 0, 0, 16)
	[ SNew(STextBlock).Text(LOCTEXT("ReviewTitle", "Review Your Settings")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)) ];

	// Protocol
	Content->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[ SNew(STextBlock).Text(LOCTEXT("ReviewProtocol", "Protocol: ")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[ SNew(STextBlock).Text(ConnectionSettings.Protocol == ETrackingProtocol::FreeD ? LOCTEXT("FreeDLabel", "FreeD") : LOCTEXT("OpenTrackLabel", "OpenTrack IO")) ]
	];

	// Network
	Content->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[ SNew(STextBlock).Text(LOCTEXT("ReviewNetwork", "Network: ")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[ SNew(STextBlock).Text(ConnectionSettings.Protocol == ETrackingProtocol::FreeD
			? FText::FromString(FString::Printf(TEXT("%s:%d"), *ConnectionSettings.IPAddress, ConnectionSettings.FreeDPort))
			: FText::FromString(FString::Printf(TEXT("Source #%d"), ConnectionSettings.OpenTrackSourceNumber))) ]
	];

	// Camera
	Content->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[ SNew(STextBlock).Text(LOCTEXT("ReviewCamera", "Camera: ")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[ SNew(STextBlock).Text(FText::FromString(CameraConfig.bCreateNewCamera
			? FString::Printf(TEXT("Create: %s"), *CameraConfig.CameraName)
			: FString::Printf(TEXT("Existing: %s"), *CameraConfig.CameraName))) ]
	];

	// Lens File
	Content->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[ SNew(STextBlock).Text(LOCTEXT("ReviewLens", "Lens: ")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[ SNew(STextBlock).Text(CameraConfig.LensConfig.bCreateNewLensFile
			? FText::FromString(FString::Printf(TEXT("Create: %s (%s)"), *CameraConfig.LensConfig.LensFileName, *CameraConfig.LensConfig.LensModelName))
			: FText::FromString(CameraConfig.LensConfig.LensFile ? CameraConfig.LensConfig.LensFile->GetName() : TEXT("None"))) ]
	];

	// Sensor
	Content->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[ SNew(STextBlock).Text(LOCTEXT("ReviewSensor", "Sensor: ")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[ SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("%.1fx%.1f mm (%s)"),
			CameraConfig.LensConfig.SensorWidthMM, CameraConfig.LensConfig.SensorHeightMM,
			*ULensSetupUtility::GetSensorPresetName(FVector2D(CameraConfig.LensConfig.SensorWidthMM, CameraConfig.LensConfig.SensorHeightMM))))) ]
	];

	// Encoder ranges
	Content->AddSlot().AutoHeight().Padding(0, 0, 0, 8)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[ SNew(STextBlock).Text(LOCTEXT("ReviewEncoders", "Encoders: ")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[ SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("Focus: %d-%d → %.0f-%.0f cm, Zoom: %d-%d → %.0f-%.0f mm"),
			CameraConfig.LensConfig.FocusEncoderRange.RawMin, CameraConfig.LensConfig.FocusEncoderRange.RawMax,
			CameraConfig.LensConfig.FocusDistanceMinCM, CameraConfig.LensConfig.FocusDistanceMaxCM,
			CameraConfig.LensConfig.ZoomEncoderRange.RawMin, CameraConfig.LensConfig.ZoomEncoderRange.RawMax,
			CameraConfig.LensConfig.FocalLengthMinMM, CameraConfig.LensConfig.FocalLengthMaxMM))) ]
	];

	return Content;
}

// Step 9: Complete
TSharedRef<SWidget> SFonixFlowTrackerSetupWizard::BuildCompleteStep()
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 16).HAlign(HAlign_Center)
	[
		SNew(STextBlock)
		.Text(SetupResult.bSuccess ? LOCTEXT("Success", "✓ Setup Complete!") : LOCTEXT("Failed", "✗ Setup Failed"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
		.ColorAndOpacity(SetupResult.bSuccess ? FSlateColor(FLinearColor(0.2f, 0.8f, 0.2f)) : FSlateColor(FLinearColor(0.8f, 0.2f, 0.2f)))
	]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 16)
	[ SNew(STextBlock).Text(FText::FromString(SetupResult.Message)).AutoWrapText(true) ]
	;
}

void SFonixFlowTrackerSetupWizard::RefreshCameraList()
{
	AvailableCameras.Empty();
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	for (TActorIterator<ACineCameraActor> It(World); It; ++It)
	{
		if (*It && (*It)->IsValidLowLevel()) AvailableCameras.Add(*It);
	}

	if (CameraListView.IsValid()) CameraListView->RequestListRefresh();
}

void SFonixFlowTrackerSetupWizard::RefreshLensFileList()
{
	AvailableLensFiles = ULensSetupUtility::FindAllLensFiles();
	if (LensFileListView.IsValid()) LensFileListView->RequestListRefresh();
}

void SFonixFlowTrackerSetupWizard::ApplySetup()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	SetupResult = UFonixFlowTrackerSetupSubsystem::SetupTracking(World, ConnectionSettings, CameraConfig);
	CurrentStep = EWizardStep::Complete;

	if (OnSetupComplete.IsBound()) OnSetupComplete.Execute();
}

#undef LOCTEXT_NAMESPACEENDOFFILEENDOFFILEENDOFFILE