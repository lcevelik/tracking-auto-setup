// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "FonixFlowTrackerSetupStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/CoreStyle.h"
#include "SlateOptMacros.h"
#include "Brushes/SlateImageBrush.h"
#include "Brushes/SlateVectorImageBrush.h"
#include "Misc/Paths.h"

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef VECTOR_BRUSH
#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BORDER_BRUSH(RelativePath, ...) FSlateBorderBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define VECTOR_BRUSH(RelativePath, ...) FSlateVectorImageBrush(Style->RootToContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)

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

	// Register SVG icon brushes
	// Main plugin icon — blue "FF" square matching the panel header
	Style->Set("FonixFlowTrackerSetup.Icon", new VECTOR_BRUSH("Icons/FonixFlowFF", FVector2D(20, 20)));

	// Toolbar icons (smaller versions)
	Style->Set("FonixFlowTrackerSetup.WizardIcon", new VECTOR_BRUSH("Icons/FonixFlowFF", FVector2D(20, 20)));
	Style->Set("FonixFlowTrackerSetup.AIChatIcon", new VECTOR_BRUSH("Icons/FonixFlowFF", FVector2D(20, 20)));
	Style->Set("FonixFlowTrackerSetup.QuickSetupIcon", new VECTOR_BRUSH("Icons/FonixFlowFF", FVector2D(20, 20)));

	// Tab icons
	Style->Set("FonixFlowTrackerSetup.TabIcon", new VECTOR_BRUSH("Icons/FonixFlowFF", FVector2D(16, 16)));

	// Large icon for panels
	Style->Set("FonixFlowTrackerSetup.PanelIcon", new VECTOR_BRUSH("Icons/FonixFlowFF", FVector2D(40, 40)));

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef VECTOR_BRUSH