// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "TrackingAutoSetupStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/CoreStyle.h"
#include "SlateOptMacros.h"
#include "Brushes/SlateImageBrush.h"
#include "Brushes/SlateSVGBrush.h"
#include "Misc/Paths.h"

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define SVG_BRUSH(RelativePath, ...) FSlateSVGBrush(Style->RootToContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BORDER_BRUSH(RelativePath, ...) FSlateBorderBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

const FName FTrackingAutoSetupStyle::StyleSetName("TrackingAutoSetupStyle");
TSharedPtr<FSlateStyleSet> FTrackingAutoSetupStyle::StyleSet = nullptr;

void FTrackingAutoSetupStyle::Initialize()
{
	if (!StyleSet.IsValid())
	{
		StyleSet = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
	}
}

void FTrackingAutoSetupStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

const ISlateStyle& FTrackingAutoSetupStyle::Get()
{
	return *StyleSet;
}

FName FTrackingAutoSetupStyle::GetStyleName()
{
	return StyleSetName;
}

void FTrackingAutoSetupStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

TSharedRef<FSlateStyleSet> FTrackingAutoSetupStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(StyleSetName));

	// Set content root to the plugin's Resources directory
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("TrackingAutoSetup")->GetBaseDir() / TEXT("Resources"));

	// Register SVG icon brushes
	// Main plugin icon (camera with tracking waves)
	Style->Set("TrackingAutoSetup.Icon", new SVG_BRUSH("Icons/TrackingAutoSetup", FVector2D(64, 64)));

	// Toolbar icons (smaller versions)
	Style->Set("TrackingAutoSetup.WizardIcon", new SVG_BRUSH("Icons/TrackingAutoSetup", FVector2D(40, 40)));
	Style->Set("TrackingAutoSetup.AIChatIcon", new SVG_BRUSH("Icons/TrackingAutoSetup", FVector2D(40, 40)));
	Style->Set("TrackingAutoSetup.QuickSetupIcon", new SVG_BRUSH("Icons/TrackingAutoSetup", FVector2D(40, 40)));

	// Tab icons
	Style->Set("TrackingAutoSetup.TabIcon", new SVG_BRUSH("Icons/TrackingAutoSetup", FVector2D(16, 16)));

	// Large icon for panels
	Style->Set("TrackingAutoSetup.PanelIcon", new SVG_BRUSH("Icons/TrackingAutoSetup", FVector2D(128, 128)));

	return Style;
}

#undef IMAGE_BRUSH
#undef SVG_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
