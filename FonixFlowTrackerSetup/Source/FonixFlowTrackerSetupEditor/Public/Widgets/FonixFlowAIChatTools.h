// Copyright (c) 2026 Libor Cevelik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FonixFlowTrackerActions.h"

/**
 * AI chat tool definitions for function calling.
 * Each tool maps to an IFonixFlowTrackerActions method.
 */
struct FChatToolDefinition
{
	FString Name;
	FString Description;
	FString ParametersSchema; // JSON Schema string

	/** Build the full tools array as JSON for the API request */
	static FString BuildToolsJSONArray();

	/** Execute a tool call and return the result string */
	static FString ExecuteTool(
		const FString& ToolName,
		const FString& ArgumentsJSON,
		IFonixFlowTrackerActions* Actions);
};
