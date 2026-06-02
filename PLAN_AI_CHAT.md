# AI Chat Enhancement Plan — Guided Setup & Collaborator

## Current State

The AI chat (`SFonixFlowTrackerAIChatPanel`) is a basic text-in/text-out panel:
- Hardcoded system prompt with generic VP tracking knowledge
- No awareness of current plugin state (camera, protocol, calibration)
- No ability to trigger actions (can't click SETUP NOW, capture, etc.)
- No streaming, no markdown, no context-aware suggestions
- Calls OpenAI-compatible API (OpenRouter default) with last 10 messages

## Goal

Transform the AI chat from a passive Q&A into an **active collaborator** that:
1. Knows what the user has configured and what's missing
2. Can suggest next steps based on current state
3. Can trigger plugin actions (setup, calibration, settings changes)
4. Guides users through the full workflow step-by-step

---

## Architecture Options

### Option A — Function Calling (OpenAI-style)
The AI returns structured `tool_calls` alongside text. The plugin executes them and feeds results back.

**Pros:**
- Industry standard (OpenAI, Anthropic, OpenRouter all support it)
- AI decides when to act vs when to talk
- Multi-step workflows (AI can chain: "select camera" → "set protocol" → "run setup")
- Model-native — no custom parsing needed

**Cons:**
- Requires `tools` array in API request (more tokens)
- Not all models support function calling equally
- Need to define tool schemas in JSON Schema format
- Response parsing is more complex (tool_calls vs content)

**Implementation:**
```cpp
// In CallAIAPI(), add "tools" array:
// tools: [
//   { type: "function", function: { name: "select_camera", parameters: {camera_name: string} }},
//   { type: "function", function: { name: "set_protocol", parameters: {protocol: "FreeD"|"OpenTrack"} }},
//   { type: "function", function: { name: "set_lens_type", parameters: {type: "Prime"|"Zoom", ...} }},
//   { type: "function", function: { name: "run_setup", parameters: {} }},
//   { type: "function", function: { name: "capture_calibration", parameters: {target: "Near"|"Far"|"Wide"|"Tele"} }},
//   { type: "function", function: { name: "apply_calibration", parameters: {} }},
//   { type: "function", function: { name: "get_state", parameters: {} }}
// ]
```

### Option B — Prompt-Driven with Regex Parsing
Inject full state into system prompt. Parse AI text responses for action keywords.

**Pros:**
- Works with ANY model (no function calling required)
- Simpler implementation
- Lower token overhead per request

**Cons:**
- Fragile — depends on AI formatting output correctly
- No reliable multi-step execution
- Hard to handle edge cases (AI says "click SETUP NOW" but doesn't trigger it)

### Option C — Hybrid (Recommended)
Use function calling when available, fall back to prompt-driven for non-FC models.

**Pros:**
- Best of both worlds
- Graceful degradation
- Works with local models (Ollama) and cloud APIs

**Cons:**
- More code to maintain
- Need to detect FC support at runtime

---

## Recommended: Option A (Function Calling) with Option B fallback

### Phase 1: State Awareness (Foundation)
**Goal:** AI knows the current plugin state at all times.

1. **Create `FFonixFlowTrackerState` struct** — serializable snapshot of all panel state:
   ```cpp
   struct FFonixFlowTrackerState {
       FString SelectedCamera;        // Actor name or "None"
       FString Protocol;              // "FreeD" or "OpenTrack"
       FString LensType;              // "Prime" or "Zoom"
       float FocalLength;             // Prime FL or 0
       float FocalLengthMin/Max;      // Zoom range
       FString IPAddress;
       int32 Port;
       bool bSetupComplete;
       bool bCalibrationApplied;
       bool bFocusCaptured[2];        // Near, Far
       bool bZoomCaptured[2];         // Wide, Tele
       float LiveLinkFocus;           // Current live value
       float LiveLinkZoom;            // Current live value
   };
   ```

2. **Inject state into system prompt** — append JSON snapshot after the static prompt:
   ```
   Current state:
   {"selected_camera": "CineCameraActor_1", "protocol": "FreeD", "lens_type": "Zoom", ...}
   ```

3. **State change notifications** — when user changes anything in the UI, update the state and optionally notify the AI (or just inject fresh state on next message).

### Phase 2: Action Tools (Arms)
**Goal:** AI can trigger plugin actions via function calling.

Define these tools:

| Tool | Parameters | Effect |
|------|-----------|--------|
| `get_plugin_state` | none | Returns full state snapshot |
| `select_camera` | `camera_name: string` | Selects camera in the list |
| `set_protocol` | `protocol: "FreeD"\|"OpenTrack"` | Switches protocol chip |
| `set_lens_type` | `type: "Prime"\|"Zoom", focal_length?, min?, max?` | Sets lens type + values |
| `run_setup` | none | Triggers SETUP NOW |
| `capture_calibration` | `target: "Near"\|"Far"\|"Wide"\|"Tele"` | Triggers capture button |
| `apply_calibration` | none | Triggers APPLY CALIBRATION |
| `set_network` | `port: int` | Changes listening port |

**Implementation approach:**
- Each tool is a C++ function that calls the corresponding panel method
- Tools are defined as JSON Schema in a `BuildToolsArray()` method
- On `tool_calls` response: execute each call, collect results, send back as `tool` role message
- Loop until AI returns `content` only (no more tool calls)

### Phase 3: Guided Workflows (Brain)
**Goal:** AI proactively guides users through setup.

1. **Enhanced system prompt** with workflow awareness:
   ```
   You are a tracking setup collaborator. Based on current state, guide the user:
   - If no camera selected → suggest selecting one
   - If camera selected but no setup → suggest SETUP NOW
   - If setup complete but no calibration → guide through calibration steps
   - If calibration incomplete → suggest which captures are missing
   - If everything done → suggest testing or re-calibration
   
   When the user asks for help, check state first using get_plugin_state.
   When appropriate, take action directly (don't just describe what to do).
   ```

2. **Quick action buttons** in chat UI:
   - "Setup my camera" → sends "Set up tracking for my camera"
   - "Calibrate lens" → sends "Guide me through lens calibration"
   - "What's my status?" → sends "What's the current state?"

3. **Proactive suggestions** — on state change (e.g., setup completes), auto-send a system message:
   ```
   [System] Setup complete. Camera: CineCameraActor_1, Protocol: FreeD, Port: 40000.
   ```
   AI responds with next steps.

### Phase 4: Rich UI (Polish)
**Goal:** Better message rendering and interaction.

1. **Streaming responses** — use `Transfer-Encoding: chunked` or SSE parsing
   - Show tokens as they arrive instead of waiting for full response
   - Replace thinking indicator with streaming text

2. **Markdown rendering** — parse AI responses for:
   - `**bold**` → bold text
   - `` `code` `` → monospace
   - Bullet lists → proper list widgets
   - Code blocks → copyable code block widget

3. **Tool call visualization** — show when AI is taking action:
   ```
   [Assistant]: Let me select the camera for you.
   [Action] Selecting camera: CineCameraActor_1 ✓
   [Assistant]: Done! Now let's run the setup.
   ```

4. **Suggested responses** — after AI message, show clickable chips:
   - "Yes, do it" / "No, I'll do it manually" / "Tell me more"

---

## Implementation Order

### v1.3.0 — State Awareness + Basic Tools
1. Create `FFonixFlowTrackerState` struct + serialization
2. Inject state into system prompt (every message)
3. Add `get_plugin_state` tool (AI can query state)
4. Add `select_camera`, `set_protocol`, `set_lens_type` tools
5. Parse `tool_calls` from response, execute, loop
6. Quick action buttons in chat UI

### v1.4.0 — Full Tool Set + Guided Workflows
1. Add `run_setup`, `capture_calibration`, `apply_calibration` tools
2. Enhanced system prompt with workflow guidance
3. Proactive state notifications
4. Tool call visualization in chat
5. Suggested response chips

### v1.5.0 — Rich UI
1. Streaming responses (SSE/chunked)
2. Markdown rendering in chat messages
3. Code block widget with copy button
4. Model selector in chat settings

---

## Technical Details

### Function Calling API Format
```json
{
  "model": "anthropic/claude-sonnet-4",
  "messages": [...],
  "tools": [
    {
      "type": "function",
      "function": {
        "name": "select_camera",
        "description": "Select a CineCameraActor in the level by name",
        "parameters": {
          "type": "object",
          "properties": {
            "camera_name": {
              "type": "string",
              "description": "The actor label of the camera to select"
            }
          },
          "required": ["camera_name"]
        }
      }
    }
  ]
}
```

### Response with tool_calls
```json
{
  "choices": [{
    "message": {
      "role": "assistant",
      "content": null,
      "tool_calls": [
        {
          "id": "call_abc123",
          "type": "function",
          "function": {
            "name": "select_camera",
            "arguments": "{\"camera_name\": \"CineCameraActor_1\"}"
          }
        }
      ]
    }
  }]
}
```

### Tool result message
```json
{
  "role": "tool",
  "tool_call_id": "call_abc123",
  "content": "Camera 'CineCameraActor_1' selected successfully."
}
```

### Panel → Chat Communication
The AI chat panel needs a reference to the setup panel (or a shared state object) to execute tools. Options:
- **Delegate/callback** — chat panel calls functions on setup panel via interface
- **Shared state object** — both panels read/write a shared `UFonixFlowTrackerState` UObject
- **Subsystem** — use existing `UFonixFlowTrackerSetupSubsystem` as the action executor

Recommended: **Interface pattern** — `IFonixFlowTrackerActions` with methods like `SelectCamera()`, `RunSetup()`, etc. Setup panel implements it. Chat panel holds a pointer to the interface.

### Model Compatibility
| Model | Function Calling | Notes |
|-------|-----------------|-------|
| Claude (Anthropic) | ✅ tool_use | Native via OpenRouter |
| GPT-4o (OpenAI) | ✅ tools | Standard format |
| Gemini (Google) | ✅ tools | Via OpenRouter |
| Llama 3.1+ | ⚠️ Partial | Some providers support it |
| Local Ollama | ⚠️ Varies | Depends on model + version |

For non-FC models: fall back to prompt-driven (Option B) — parse action keywords from text.

---

## Files to Create/Modify

### New Files
- `FonixFlowTrackerSetup/Source/FonixFlowTrackerSetupEditor/Public/Widgets/FonixFlowAIChatTypes.h` — FChatTool, FToolCall, FToolResult structs
- `FonixFlowTrackerSetup/Source/FonixFlowTrackerSetupEditor/Private/Widgets/FonixFlowAIChatTools.cpp` — Tool definitions + execution

### Modified Files
- `SFonixFlowTrackerAIChatPanel.h/.cpp` — Function calling loop, tool execution, state injection, streaming
- `SFonixFlowTrackerSetupPanel.h/.cpp` — Implement IFonixFlowTrackerActions interface
- `FonixFlowTrackerSettings.h/.cpp` — Add model FC support detection setting
- `CLAUDE.md` — Document AI chat architecture
