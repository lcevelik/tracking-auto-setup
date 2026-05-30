// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

/**
 * Custom Slate style for Tracking Auto Setup plugin.
 * Registers icons and brushes used in the editor UI.
 */
class FTrackingAutoSetupStyle
{
public:
	/** Initialize the style and register it */
	static void Initialize();

	/** Shutdown and unregister the style */
	static void Shutdown();

	/** Get the style set */
	static const ISlateStyle& Get();

	/** Get the style name */
	static FName GetStyleName();

	/** Reload textures (called when resources change) */
	static void ReloadTextures();

private:
	/** Create the style set */
	static TSharedRef<FSlateStyleSet> Create();

	/** The style set instance */
	static TSharedPtr<FSlateStyleSet> StyleSet;

	/** Style set name */
	static const FName StyleSetName;
};
