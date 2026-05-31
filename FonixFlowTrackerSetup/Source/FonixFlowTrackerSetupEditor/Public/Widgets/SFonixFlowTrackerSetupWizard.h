// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.
// Wizard deprecated — functionality moved to SFonixFlowTrackerSetupPanel

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * @deprecated — Use SFonixFlowTrackerSetupPanel instead.
 * Kept for ABI compatibility. Will be removed in v2.
 */
class FONIXFLOWTRACKERSETUPEDITOR_API SFonixFlowTrackerSetupWizard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFonixFlowTrackerSetupWizard) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		ChildSlot
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Deprecated — use the main Setup panel")))
		];
	}
};
