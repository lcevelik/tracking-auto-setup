// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#include "EditorUtility/TrackingAutoSetupEditorUtilityWidgetFactory.h"
#include "EditorUtility/TrackingAutoSetupEditorUtilityWidget.h"
#include "Editor/EditorUtilityWidgetBlueprint.h"

#define LOCTEXT_NAMESPACE "TrackingAutoSetupEditorUtilityWidgetFactory"

UTrackingAutoSetupEditorUtilityWidgetFactory::UTrackingAutoSetupEditorUtilityWidgetFactory()
{
	SupportedClass = UEditorUtilityWidgetBlueprint::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UTrackingAutoSetupEditorUtilityWidgetFactory::FactoryCreateNew(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UEditorUtilityWidgetBlueprint* NewBP = NewObject<UEditorUtilityWidgetBlueprint>(
		InParent, InClass, InName, Flags);

	if (NewBP)
	{
		// Set the parent class to our custom EUW
		NewBP->GeneratedClass = UTrackingAutoSetupEditorUtilityWidget::StaticClass();
		NewBP->ParentClass = UTrackingAutoSetupEditorUtilityWidget::StaticClass();
	}

	return NewBP;
}

FText UTrackingAutoSetupEditorUtilityWidgetFactory::GetDisplayName() const
{
	return LOCTEXT("DisplayName", "Tracking Auto Setup Widget");
}

uint32 UTrackingAutoSetupEditorUtilityWidgetFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::EditorUtilities;
}

#undef LOCTEXT_NAMESPACE
