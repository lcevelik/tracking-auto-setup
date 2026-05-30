// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "EditorUtility/TrackingAutoSetupEditorUtilityWidget.h"
#include "TrackingAutoSetupEditorUtilityWidgetFactory.generated.h"

/**
 * Factory for creating UTrackingAutoSetupEditorUtilityWidget blueprints.
 * Makes the widget appear in Content Browser > Editor Utilities section.
 */
UCLASS()
class UTrackingAutoSetupEditorUtilityWidgetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UTrackingAutoSetupEditorUtilityWidgetFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override { return true; }
	virtual FText GetDisplayName() const override;
	virtual uint32 GetMenuCategories() const override;
};
