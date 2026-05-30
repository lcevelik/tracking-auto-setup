// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "Widgets/SFonixFlowTrackerSetupPanel.h"
#include "Widgets/SFonixFlowTrackerSetupWizard.h"
#include "Widgets/SFonixFlowTrackerAIChatPanel.h"
#include "FonixFlowTrackerSetupStyle.h"
#include "SlateOptMacros.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "SFonixFlowTrackerSetupPanel"

const FName SFonixFlowTrackerSetupPanel::WizardTabId("TrackingWizardTab");
const FName SFonixFlowTrackerSetupPanel::AIChatTabId("TrackingAIChatTab");
const FName SFonixFlowTrackerSetupPanel::SettingsTabId("TrackingSettingsTab");

void SFonixFlowTrackerSetupPanel::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)

		// Header with icon
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 4)
		[
			BuildHeader()
		]

		// Main content area with tab-like buttons
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 4)
		[
			SNew(SHorizontalBox)

			// Wizard tab button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
				.OnClicked_Lambda([this]() -> FReply
				{
					// Switch to wizard content
					return FReply::Handled();
				})
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FFonixFlowTrackerSetupStyle::Get().GetBrush("FonixFlowTrackerSetup.TabIcon"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(4, 0, 0, 0)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("WizardTab", "Wizard"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
					]
				]
			]

			// AI Chat tab button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4, 0, 0, 0)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
				.OnClicked_Lambda([this]() -> FReply
				{
					// Switch to AI chat content
					return FReply::Handled();
				})
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FFonixFlowTrackerSetupStyle::Get().GetBrush("FonixFlowTrackerSetup.TabIcon"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(4, 0, 0, 0)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("AIChatTab", "AI Chat"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
					]
				]
			]

			// Quick Setup button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4, 0, 0, 0)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
				.OnClicked_Lambda([this]() -> FReply
				{
					// Quick setup action
					return FReply::Handled();
				})
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FFonixFlowTrackerSetupStyle::Get().GetBrush("FonixFlowTrackerSetup.TabIcon"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(4, 0, 0, 0)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("QuickSetupTab", "Quick Setup"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
					]
				]
			]
		]

		// Content area
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(0)
			[
				SAssignNew(WizardWidget, SFonixFlowTrackerSetupWizard)
			]
		]
	];
}

TSharedRef<SWidget> SFonixFlowTrackerSetupPanel::BuildHeader()
{
	return SNew(SBorder)
	.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
	.Padding(12, 8)
	[
		SNew(SHorizontalBox)

		// Plugin icon
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

		// Title and description
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PanelTitle", "FonixFlow Tracker Setup"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f)))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 2, 0, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PanelDesc", "One-click Live Link camera tracking setup for FreeD and OpenTrack"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
			]
		]

		// Version badge
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8, 4)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Version", "v0.2.0"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE
