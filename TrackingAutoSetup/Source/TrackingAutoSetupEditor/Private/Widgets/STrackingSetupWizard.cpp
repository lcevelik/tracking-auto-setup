// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "Widgets/STrackingSetupWizard.h"
#include "TrackingAutoSetupSubsystem.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SSeparator.h"
#include "Camera/CineCameraActor.h"
#include "Camera/CineCameraComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Editor.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "STrackingSetupWizard"

void STrackingSetupWizard::Construct(const FArguments& InArgs)
{
	OnSetupComplete = InArgs._OnSetupComplete;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(16)
		[
			SNew(SVerticalBox)

			// Header
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("WizardTitle", "Tracking Auto Setup Wizard"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 4)
			[
				SNew(STextBlock)
				.Text_Raw(this, &STrackingSetupWizard::GetStepNumberText)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 4)
			[
				SNew(STextBlock)
				.Text_Raw(this, &STrackingSetupWizard::GetStepTitle)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 12)
			[
				SNew(STextBlock)
				.Text_Raw(this, &STrackingSetupWizard::GetStepDescription)
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
					.BorderImage(FEditorStyle::GetBrush("NoBorder"))
					.Padding(0, 8)
					[
						SNew(SBox)
						.MinDesiredHeight(300)
						[
							SNew(SBorder)
							.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
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
					.IsEnabled_Raw(this, &STrackingSetupWizard::CanGoPrevious)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					// Step dots indicator
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("StepDots", "● ● ● ● ● ●"))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.3f, 0.3f, 0.3f)))
					]
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
					.IsEnabled_Raw(this, &STrackingSetupWizard::CanGoNext)
				]
			]
		]
	];
}

void STrackingSetupWizard::GoToNextStep()
{
	if (CurrentStep < EWizardStep::Complete)
	{
		CurrentStep = static_cast<EWizardStep>(static_cast<uint8>(CurrentStep) + 1);

		if (CurrentStep == EWizardStep::Camera)
		{
			RefreshCameraList();
		}
	}
}

void STrackingSetupWizard::GoToPreviousStep()
{
	if (CurrentStep > EWizardStep::Protocol)
	{
		CurrentStep = static_cast<EWizardStep>(static_cast<uint8>(CurrentStep) - 1);
	}
}

bool STrackingSetupWizard::CanGoNext() const
{
	return CurrentStep < EWizardStep::Complete;
}

bool STrackingSetupWizard::CanGoPrevious() const
{
	return CurrentStep > EWizardStep::Protocol;
}

TSharedRef<SWidget> STrackingSetupWizard::GetCurrentStepContent()
{
	switch (CurrentStep)
	{
	case EWizardStep::Protocol:        return BuildProtocolStep();
	case EWizardStep::Network:         return BuildNetworkStep();
	case EWizardStep::Camera:          return BuildCameraStep();
	case EWizardStep::LensCalibration: return BuildLensCalibrationStep();
	case EWizardStep::AnchorPoint:     return BuildAnchorPointStep();
	case EWizardStep::Review:          return BuildReviewStep();
	case EWizardStep::Complete:        return BuildCompleteStep();
	default:                           return SNew(STextBlock).Text(LOCTEXT("Unknown", "Unknown step"));
	}
}

FText STrackingSetupWizard::GetStepTitle() const
{
	switch (CurrentStep)
	{
	case EWizardStep::Protocol:        return LOCTEXT("Step1Title", "Select Tracking Protocol");
	case EWizardStep::Network:         return LOCTEXT("Step2Title", "Network Configuration");
	case EWizardStep::Camera:          return LOCTEXT("Step3Title", "Camera Selection");
	case EWizardStep::LensCalibration: return LOCTEXT("Step4Title", "Lens Encoder Calibration");
	case EWizardStep::AnchorPoint:     return LOCTEXT("Step5Title", "Anchor Point Setup");
	case EWizardStep::Review:          return LOCTEXT("Step6Title", "Review & Apply");
	case EWizardStep::Complete:        return LOCTEXT("Step7Title", "Setup Complete");
	default:                           return FText::GetEmpty();
	}
}

FText STrackingSetupWizard::GetStepDescription() const
{
	switch (CurrentStep)
	{
	case EWizardStep::Protocol:
		return LOCTEXT("Step1Desc", "Choose the tracking protocol your camera system uses. FreeD is the industry standard for encoded camera heads. OpenTrack IO is an open protocol for head/camera tracking.");
	case EWizardStep::Network:
		return LOCTEXT("Step2Desc", "Configure the network connection to your tracking system. Enter the IP address and port number.");
	case EWizardStep::Camera:
		return LOCTEXT("Step3Desc", "Select an existing CineCameraActor from your scene, or create a new one. The selected camera will be configured with Live Link tracking.");
	case EWizardStep::LensCalibration:
		return LOCTEXT("Step4Desc", "Calibrate the lens encoder ranges. Rotate each lens (focus and zoom) to its MINIMUM position, then click Capture. Then rotate to MAXIMUM and capture again. This maps the raw encoder values to physical lens values.");
	case EWizardStep::AnchorPoint:
		return LOCTEXT("Step5Desc", "Configure the tracking anchor point. This is the origin point for all tracking data. Set the world position and rotation to match your physical tracking system origin.");
	case EWizardStep::Review:
		return LOCTEXT("Step6Desc", "Review your settings before applying. Click Apply Setup to configure everything.");
	case EWizardStep::Complete:
		return LOCTEXT("Step7Desc", "Tracking setup is complete! Your camera is now configured with Live Link tracking.");
	default:
		return FText::GetEmpty();
	}
}

FText STrackingSetupWizard::GetStepNumberText() const
{
	int32 StepNum = static_cast<uint8>(CurrentStep) + 1;
	return FText::Format(LOCTEXT("StepOf", "Step {0} of 7"), FText::AsNumber(StepNum));
}

// Step 1: Protocol Selection
TSharedRef<SWidget> STrackingSetupWizard::BuildProtocolStep()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 16)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("ProtocolLabel", "Tracking Protocol:"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
	]

	// FreeD option
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 8)
	[
		SNew(SBorder)
		.BorderImage(ConnectionSettings.Protocol == ETrackingProtocol::FreeD
			? FEditorStyle::GetBrush("ToolPanel.GroupBorder")
			: FEditorStyle::GetBrush("NoBorder"))
		.Padding(12)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(ConnectionSettings.Protocol == ETrackingProtocol::FreeD ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
				{
					ConnectionSettings.Protocol = ETrackingProtocol::FreeD;
				})
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(8, 0, 0, 0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("FreeDName", "FreeD Protocol"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("FreeDDesc", "Industry standard for camera tracking data (PTZ, encoded heads). Default port: 40000 UDP."))
					.AutoWrapText(true)
				]
			]
		]
	]

	// OpenTrack IO option
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 8)
	[
		SNew(SBorder)
		.BorderImage(ConnectionSettings.Protocol == ETrackingProtocol::OpenTrackIO
			? FEditorStyle::GetBrush("ToolPanel.GroupBorder")
			: FEditorStyle::GetBrush("NoBorder"))
		.Padding(12)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(ConnectionSettings.Protocol == ETrackingProtocol::OpenTrackIO ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
				{
					ConnectionSettings.Protocol = ETrackingProtocol::OpenTrackIO;
				})
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(8, 0, 0, 0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("OpenTrackName", "OpenTrack IO"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("OpenTrackDesc", "Open protocol for head/camera tracking. Multicast address: 235.135.1.[Source]. Default port: 55555."))
					.AutoWrapText(true)
				]
			]
		]
	]

	// Camera rig preset
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
	.Padding(0, 4, 0, 0)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("RigPresetDesc", "Select the manufacturer preset for your camera system. This sets default encoder ranges and connection settings."))
		.AutoWrapText(true)
		.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
	]

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 8, 0, 0)
	[
		SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource_Lambda([this]() -> TArray<TSharedPtr<FString>>
		{
			TArray<TSharedPtr<FString>> Options;
			Options.Add(MakeShareable(new FString(TEXT("Generic"))));
			Options.Add(MakeShareable(new FString(TEXT("Panasonic"))));
			Options.Add(MakeShareable(new FString(TEXT("Sony"))));
			Options.Add(MakeShareable(new FString(TEXT("stYpe"))));
			Options.Add(MakeShareable(new FString(TEXT("Mosys"))));
			Options.Add(MakeShareable(new FString(TEXT("Ncam"))));
			return Options;
		})
		.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewValue, ESelectInfo::Type)
		{
			if (NewValue.IsValid())
			{
				if (*NewValue == TEXT("Generic")) ConnectionSettings.RigPreset = ECameraRigPreset::Generic;
				else if (*NewValue == TEXT("Panasonic")) ConnectionSettings.RigPreset = ECameraRigPreset::Panasonic;
				else if (*NewValue == TEXT("Sony")) ConnectionSettings.RigPreset = ECameraRigPreset::Sony;
				else if (*NewValue == TEXT("stYpe")) ConnectionSettings.RigPreset = ECameraRigPreset::Stype;
				else if (*NewValue == TEXT("Mosys")) ConnectionSettings.RigPreset = ECameraRigPreset::Mosys;
				else if (*NewValue == TEXT("Ncam")) ConnectionSettings.RigPreset = ECameraRigPreset::Ncam;
			}
		})
		.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item) -> TSharedRef<SWidget>
		{
			return SNew(STextBlock).Text(FText::FromString(*Item));
		})
		.InitiallySelectedItem(MakeShareable(new FString(TEXT("Generic"))))
		[
			SNew(STextBlock)
			.Text_Lambda([this]() -> FText
			{
				switch (ConnectionSettings.RigPreset)
				{
				case ECameraRigPreset::Generic:   return FText::FromString(TEXT("Generic"));
				case ECameraRigPreset::Panasonic: return FText::FromString(TEXT("Panasonic"));
				case ECameraRigPreset::Sony:      return FText::FromString(TEXT("Sony"));
				case ECameraRigPreset::Stype:     return FText::FromString(TEXT("stYpe"));
				case ECameraRigPreset::Mosys:     return FText::FromString(TEXT("Mosys"));
				case ECameraRigPreset::Ncam:      return FText::FromString(TEXT("Ncam"));
				default:                          return FText::FromString(TEXT("Generic"));
				}
			})
		]
	];
}

// Step 2: Network Configuration
TSharedRef<SWidget> STrackingSetupWizard::BuildNetworkStep()
{
	TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);

	if (ConnectionSettings.Protocol == ETrackingProtocol::FreeD)
	{
		Content->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 12)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 4)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("IPLabel", "Tracking Source IP Address:"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SEditableTextBox)
				.Text(FText::FromString(ConnectionSettings.IPAddress))
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
				{
					ConnectionSettings.IPAddress = Text.ToString();
				})
				.HintText(LOCTEXT("IPHint", "e.g., 192.168.1.100"))
			]
		];

		Content->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 12)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 4)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PortLabel", "UDP Port:"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SEditableTextBox)
				.Text(FText::AsNumber(ConnectionSettings.FreeDPort))
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
				{
					FString Str = Text.ToString();
					if (Str.IsNumeric())
					{
						ConnectionSettings.FreeDPort = FCString::Atoi(*Str);
					}
				})
				.HintText(LOCTEXT("PortHint", "Default: 40000"))
			]
		];

		Content->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FreeDPortNote", "Most FreeD systems use port 40000. Check your tracking system documentation."))
			.AutoWrapText(true)
			.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
		];
	}
	else if (ConnectionSettings.Protocol == ETrackingProtocol::OpenTrackIO)
	{
		Content->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 12)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 4)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SourceNumLabel", "Source Number (1-200):"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SEditableTextBox)
				.Text(FText::AsNumber(ConnectionSettings.OpenTrackSourceNumber))
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
				{
					FString Str = Text.ToString();
					if (Str.IsNumeric())
					{
						int32 Val = FCString::Atoi(*Str);
						ConnectionSettings.OpenTrackSourceNumber = FMath::Clamp(Val, 1, 200);
					}
				})
				.HintText(LOCTEXT("SourceNumHint", "Multicast address: 235.135.1.[Number]"))
			]
		];

		Content->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 12)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(ConnectionSettings.bOpenTrackUseMulticast ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
				{
					ConnectionSettings.bOpenTrackUseMulticast = (State == ECheckBoxState::Checked);
				})
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(8, 0, 0, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("UseMulticast", "Use Multicast (recommended)"))
			]
		];

		if (!ConnectionSettings.bOpenTrackUseMulticast)
		{
			Content->AddSlot()
			.AutoHeight()
			.Padding(0, 0, 0, 12)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 4)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("UnicastPortLabel", "Unicast Port:"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SEditableTextBox)
					.Text(FText::AsNumber(ConnectionSettings.OpenTrackUnicastPort))
					.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
					{
						FString Str = Text.ToString();
						if (Str.IsNumeric())
						{
							ConnectionSettings.OpenTrackUnicastPort = FCString::Atoi(*Str);
						}
					})
					.HintText(LOCTEXT("UnicastPortHint", "0 = auto-assign"))
				]
			];
		}

		Content->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("OpenTrackNote", "OpenTrack IO uses multicast by default. Source number maps to multicast address 235.135.1.[N]."))
			.AutoWrapText(true)
			.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
		];
	}

	// Subject name
	Content->AddSlot()
	.AutoHeight()
	.Padding(0, 16, 0, 0)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SubjectLabel", "Live Link Subject Name:"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SEditableTextBox)
			.Text(FText::FromString(ConnectionSettings.SubjectName))
			.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
			{
				ConnectionSettings.SubjectName = Text.ToString();
			})
			.HintText(LOCTEXT("SubjectHint", "e.g., MainCamera"))
		]
	];

	return Content;
}

// Step 3: Camera Selection
TSharedRef<SWidget> STrackingSetupWizard::BuildCameraStep()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 12)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SCheckBox)
			.IsChecked(CameraConfig.bCreateNewCamera ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
			{
				CameraConfig.bCreateNewCamera = (State == ECheckBoxState::Checked);
			})
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(8, 0, 0, 0)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CreateNew", "Create new CineCameraActor"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
	]

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 12)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SCheckBox)
			.IsChecked(!CameraConfig.bCreateNewCamera ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
			{
				CameraConfig.bCreateNewCamera = (State != ECheckBoxState::Checked);
			})
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(8, 0, 0, 0)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("UseExisting", "Use existing CineCameraActor from scene"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
	]

	// Camera name (for new)
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 12)
	[
		SNew(SVerticalBox)
		.Visibility_Lambda([this]() -> EVisibility
		{
			return CameraConfig.bCreateNewCamera ? EVisibility::Visible : EVisibility::Collapsed;
		})
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CameraNameLabel", "Camera Name:"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SEditableTextBox)
			.Text(FText::FromString(CameraConfig.CameraName))
			.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
			{
				CameraConfig.CameraName = Text.ToString();
			})
		]
	]

	// Existing camera picker
	+ SVerticalBox::Slot()
	.FillHeight(1.0f)
	[
		SNew(SVerticalBox)
		.Visibility_Lambda([this]() -> EVisibility
		{
			return !CameraConfig.bCreateNewCamera ? EVisibility::Visible : EVisibility::Collapsed;
		})
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SelectCamera", "Select Camera:"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 4)
		[
			SNew(SButton)
			.Text(LOCTEXT("RefreshCameras", "Refresh Camera List"))
			.OnClicked_Lambda([this]() -> FReply
			{
				RefreshCameraList();
				return FReply::Handled();
			})
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(CameraListView, SListView<TWeakObjectPtr<ACineCameraActor>>)
			.ListItemsSource(&AvailableCameras)
			.OnGenerateRow_Lambda([](TWeakObjectPtr<ACineCameraActor> Item, const TSharedRef<STableViewBase>& OwnerTable) -> TSharedRef<ITableRow>
			{
				FString Name = Item.IsValid() ? Item->GetActorLabel() : TEXT("Invalid");
				return SNew(STableRow<TWeakObjectPtr<ACineCameraActor>>, OwnerTable)
				[
					SNew(STextBlock).Text(FText::FromString(Name))
				];
			})
			.OnSelectionChanged_Lambda([this](TWeakObjectPtr<ACineCameraActor> Item, ESelectInfo::Type)
			{
				if (Item.IsValid())
				{
					CameraConfig.ExistingCamera = Item.Get();
					CameraConfig.CameraName = Item->GetActorLabel();
				}
			})
			.SelectionMode(ESelectionMode::Single)
		]
	]
	;
}

// Step 4: Lens Calibration
TSharedRef<SWidget> STrackingSetupWizard::BuildLensCalibrationStep()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 16)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("CalibInstructions", 
			"This step calibrates the lens encoder ranges. You need to physically rotate each lens to its MINIMUM and MAXIMUM positions.\n\n"
			"For each lens control (Focus, Zoom):\n"
			"1. Rotate the lens to its MINIMUM position\n"
			"2. Click 'Capture Min'\n"
			"3. Rotate the lens to its MAXIMUM position\n"
			"4. Click 'Capture Max'\n\n"
			"This maps raw encoder values (0-16777215 for 24-bit) to physical lens values."))
		.AutoWrapText(true)
	]

	// Focus Distance Calibration
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 16)
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(12)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("FocusCalib", "Focus Distance Encoder"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(FText::Format(LOCTEXT("FocusMin", "Min Value: {0}"), FText::AsNumber(CurrentFocusEncoderMin)))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("CaptureFocusMin", "Capture Min"))
					.OnClicked_Lambda([this]() -> FReply
					{
						OnCaptureFocusMin();
						return FReply::Handled();
					})
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(FText::Format(LOCTEXT("FocusMax", "Max Value: {0}"), FText::AsNumber(CurrentFocusEncoderMax)))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("CaptureFocusMax", "Capture Max"))
					.OnClicked_Lambda([this]() -> FReply
					{
						OnCaptureFocusMax();
						return FReply::Handled();
					})
				]
			]
		]
	]

	// Zoom/Focal Length Calibration
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(12)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ZoomCalib", "Focal Length (Zoom) Encoder"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(FText::Format(LOCTEXT("ZoomMin", "Min Value: {0}"), FText::AsNumber(CurrentZoomEncoderMin)))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("CaptureZoomMin", "Capture Min"))
					.OnClicked_Lambda([this]() -> FReply
					{
						OnCaptureZoomMin();
						return FReply::Handled();
					})
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(FText::Format(LOCTEXT("ZoomMax", "Max Value: {0}"), FText::AsNumber(CurrentZoomEncoderMax)))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("CaptureZoomMax", "Capture Max"))
					.OnClicked_Lambda([this]() -> FReply
					{
						OnCaptureZoomMax();
						return FReply::Handled();
					})
				]
			]
		]
	]

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 8, 0, 0)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("CalibNote", "Note: Encoder values are read from the incoming FreeD/OpenTrack data stream. Make sure your tracking system is sending data before capturing."))
		.AutoWrapText(true)
		.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
	]
	;
}

// Step 5: Anchor Point
TSharedRef<SWidget> STrackingSetupWizard::BuildAnchorPointStep()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 12)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SCheckBox)
			.IsChecked(CameraConfig.bCreateAnchorPoint ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
			{
				CameraConfig.bCreateAnchorPoint = (State == ECheckBoxState::Checked);
			})
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(8, 0, 0, 0)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CreateAnchor", "Create tracking anchor point"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
	]

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 12)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("AnchorDesc", "The anchor point is the origin for all tracking data. The camera will be attached to this point. Set the position to match your physical tracking system origin."))
		.AutoWrapText(true)
	]

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 12)
	.Visibility_Lambda([this]() -> EVisibility
	{
		return CameraConfig.bCreateAnchorPoint ? EVisibility::Visible : EVisibility::Collapsed;
	})
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("AnchorPos", "Anchor Position (X, Y, Z):"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0, 0, 4, 0)
			[
				SNew(SEditableTextBox)
				.Text(FText::FromString(FString::SanitizeFloat(CameraConfig.AnchorLocation.X)))
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
				{
					CameraConfig.AnchorLocation.X = FCString::Atof(*Text.ToString());
				})
				.HintText(LOCTEXT("X", "X"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(4, 0, 4, 0)
			[
				SNew(SEditableTextBox)
				.Text(FText::FromString(FString::SanitizeFloat(CameraConfig.AnchorLocation.Y)))
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
				{
					CameraConfig.AnchorLocation.Y = FCString::Atof(*Text.ToString());
				})
				.HintText(LOCTEXT("Y", "Y"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(4, 0, 0, 0)
			[
				SNew(SEditableTextBox)
				.Text(FText::FromString(FString::SanitizeFloat(CameraConfig.AnchorLocation.Z)))
				.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
				{
					CameraConfig.AnchorLocation.Z = FCString::Atof(*Text.ToString());
				})
				.HintText(LOCTEXT("Z", "Z"))
			]
		]
	]

	// Virtual camera option
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 16, 0, 0)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SCheckBox)
			.IsChecked(CameraConfig.bEnableVirtualCamera ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
			{
				CameraConfig.bEnableVirtualCamera = (State == ECheckBoxState::Checked);
			})
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(8, 0, 0, 0)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EnableVCam", "Enable Virtual Camera"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("VCamDesc", "Wire tracking data into the Virtual Camera system for in-editor virtual camera operation."))
				.AutoWrapText(true)
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
			]
		]
	]
	;
}

// Step 6: Review
TSharedRef<SWidget> STrackingSetupWizard::BuildReviewStep()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 16)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("ReviewTitle", "Review Your Settings"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
	]

	// Protocol
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 8)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReviewProtocol", "Protocol: "))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(ConnectionSettings.Protocol == ETrackingProtocol::FreeD
				? LOCTEXT("FreeDLabel", "FreeD")
				: LOCTEXT("OpenTrackLabel", "OpenTrack IO"))
		]
	]

	// Network
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 8)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReviewNetwork", "Network: "))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(ConnectionSettings.Protocol == ETrackingProtocol::FreeD
				? FText::FromString(FString::Printf(TEXT("%s:%d"), *ConnectionSettings.IPAddress, ConnectionSettings.FreeDPort))
				: FText::FromString(FString::Printf(TEXT("Source #%d"), ConnectionSettings.OpenTrackSourceNumber)))
		]
	]

	// Camera
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 8)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReviewCamera", "Camera: "))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(CameraConfig.bCreateNewCamera
				? FString::Printf(TEXT("Create new: %s"), *CameraConfig.CameraName)
				: FString::Printf(TEXT("Use existing: %s"), *CameraConfig.CameraName)))
		]
	]

	// Subject name
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 8)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReviewSubject", "Subject: "))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(ConnectionSettings.SubjectName))
		]
	]

	// Anchor
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 8)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReviewAnchor", "Anchor: "))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(CameraConfig.bCreateAnchorPoint
				? FText::FromString(FString::Printf(TEXT("Yes, at %s"), *CameraConfig.AnchorLocation.ToString()))
				: LOCTEXT("NoAnchor", "None"))
		]
	]

	// Virtual Camera
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 8)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReviewVCam", "Virtual Camera: "))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(CameraConfig.bEnableVirtualCamera ? LOCTEXT("Enabled", "Enabled") : LOCTEXT("Disabled", "Disabled"))
		]
	]
	;
}

// Step 7: Complete
TSharedRef<SWidget> STrackingSetupWizard::BuildCompleteStep()
{
	return SNew(SVerticalBox)

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 16)
	.HAlign(HAlign_Center)
	[
		SNew(STextBlock)
		.Text(SetupResult.bSuccess ? LOCTEXT("Success", "✓ Setup Complete!") : LOCTEXT("Failed", "✗ Setup Failed"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
		.ColorAndOpacity(SetupResult.bSuccess
			? FSlateColor(FLinearColor(0.2f, 0.8f, 0.2f))
			: FSlateColor(FLinearColor(0.8f, 0.2f, 0.2f)))
	]

	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 16)
	[
		SNew(STextBlock)
		.Text(FText::FromString(SetupResult.Message))
		.AutoWrapText(true)
	]

	// Warnings
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 8)
	[
		SNew(SVerticalBox)
		.Visibility(SetupResult.Warnings.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Warnings", "Warnings:"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.8f, 0.0f)))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]() -> FText
			{
				return FText::FromString(FString::Join(SetupResult.Warnings, TEXT("\n")));
			})
			.AutoWrapText(true)
		]
	]

	// Errors
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SVerticalBox)
		.Visibility(SetupResult.Errors.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Errors", "Errors:"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.2f, 0.2f)))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]() -> FText
			{
				return FText::FromString(FString::Join(SetupResult.Errors, TEXT("\n")));
			})
			.AutoWrapText(true)
		]
	]
	;
}

void STrackingSetupWizard::RefreshCameraList()
{
	AvailableCameras.Empty();

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	for (TActorIterator<ACineCameraActor> It(World); It; ++It)
	{
		ACineCameraActor* Camera = *It;
		if (Camera && Camera->IsValidLowLevel())
		{
			AvailableCameras.Add(Camera);
		}
	}

	if (CameraListView.IsValid())
	{
		CameraListView->RequestListRefresh();
	}
}

void STrackingSetupWizard::OnCaptureFocusMin()
{
	// TODO: Read current encoder value from Live Link data stream
	// For now, use a placeholder that would be replaced with actual Live Link data reading
	CurrentFocusEncoderMin = 0;
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: Captured focus encoder MIN: %d"), CurrentFocusEncoderMin);
}

void STrackingSetupWizard::OnCaptureFocusMax()
{
	CurrentFocusEncoderMax = 0x00FFFFFF;
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: Captured focus encoder MAX: %d"), CurrentFocusEncoderMax);
}

void STrackingSetupWizard::OnCaptureZoomMin()
{
	CurrentZoomEncoderMin = 0;
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: Captured zoom encoder MIN: %d"), CurrentZoomEncoderMin);
}

void STrackingSetupWizard::OnCaptureZoomMax()
{
	CurrentZoomEncoderMax = 0x00FFFFFF;
	UE_LOG(LogTemp, Log, TEXT("TrackingAutoSetup: Captured zoom encoder MAX: %d"), CurrentZoomEncoderMax);
}

void STrackingSetupWizard::ApplySetup()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	SetupResult = UTrackingAutoSetupSubsystem::SetupTracking(World, ConnectionSettings, CameraConfig);

	CurrentStep = EWizardStep::Complete;

	if (OnSetupComplete.IsBound())
	{
		OnSetupComplete.Execute();
	}
}

#undef LOCTEXT_NAMESPACE
