// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "FonixFlowTrackerSetupStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/CoreStyle.h"
#include "SlateOptMacros.h"
#include "Brushes/SlateImageBrush.h"
#include "Misc/Paths.h"

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BORDER_BRUSH(RelativePath, ...) FSlateBorderBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

const FName FFonixFlowTrackerSetupStyle::StyleSetName("FonixFlowTrackerSetupStyle");
TSharedPtr<FSlateStyleSet> FFonixFlowTrackerSetupStyle::StyleSet = nullptr;

void FFonixFlowTrackerSetupStyle::Initialize()
{
	if (!StyleSet.IsValid())
	{
		StyleSet = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
	}
}

void FFonixFlowTrackerSetupStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

const ISlateStyle& FFonixFlowTrackerSetupStyle::Get()
{
	return *StyleSet;
}

FName FFonixFlowTrackerSetupStyle::GetStyleName()
{
	return StyleSetName;
}

void FFonixFlowTrackerSetupStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

TSharedRef<FSlateStyleSet> FFonixFlowTrackerSetupStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(StyleSetName));

	// Set content root to the plugin's Resources directory
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("FonixFlowTrackerSetup")->GetBaseDir() / TEXT("Resources"));

	// Register icon brushes (using .png icons)
	// Main plugin icon (camera with tracking waves)
	Style->Set("FonixFlowTrackerSetup.Icon", new IMAGE_BRUSH("Icons/FonixFlowTrackerSetup", FVector2D(64, 64)));

	// Toolbar icons (smaller versions)
	Style->Set("FonixFlowTrackerSetup.WizardIcon", new IMAGE_BRUSH("Icons/FonixFlowTrackerSetup", FVector2D(40, 40)));
	Style->Set("FonixFlowTrackerSetup.AIChatIcon", new IMAGE_BRUSH("Icons/FonixFlowTrackerSetup", FVector2D(40, 40)));
	Style->Set("FonixFlowTrackerSetup.QuickSetupIcon", new IMAGE_BRUSH("Icons/FonixFlowTrackerSetup", FVector2D(40, 40)));

	// Tab icons
	Style->Set("FonixFlowTrackerSetup.TabIcon", new IMAGE_BRUSH("Icons/FonixFlowTrackerSetup", FVector2D(16, 16)));

	// Large icon for panels
	Style->Set("FonixFlowTrackerSetup.PanelIcon", new IMAGE_BRUSH("Icons/FonixFlowTrackerSetup", FVector2D(128, 128)));

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
