---
applyTo: "data/**/*.cfg,data/**/*.dat,source/ui/**/*.c,source/ui/**/*.h,source/tutorial_tr.c,source/tutorial_tr.h,source/tutorial_tr_internal.h,source/text_tx.c,source/text_tx.h,source/gameplay/operand_feedback_of.c,source/gameplay/operand_feedback_of.h,source/aux.h,source/buffers_bf.c,source/buffers_bf.h,source/code_window_cw.c,source/code_window_cw.h,source/instruction_window_iw.c,source/instruction_window_iw.h,source/registers_rg.c,source/registers_rg.h,tests/**/*tutorial*.c,tests/**/*text*.c,tests/**/*feedback*.c,tests/**/*result*.c,tests/**/*display*.c,tests/**/*layout*.c,tests/**/*menu*.c,tests/**/*screen*.c"
---

# Assembly Architect UI Language

## Purpose and Scope

This file defines the canonical language and presentation guidance for player-facing content in Assembly Architect. Apply it when designing or reviewing tutorials, challenges, menus, controls, tooltips, system messages, errors, execution results, terminology, accessibility, visual hierarchy, and player-facing labels.

The intended presentation is a restrained terminal interface influenced by firmware, ISA documentation, diagnostic systems, and early computer systems.

Existing content is evidence, not an immutable rule. Preserve effective choices, but prefer alternatives that improve clarity, accessibility, consistency, educational progression, maintainability, technical accuracy, and visual coherence. When changing established terminology, headers, severity, layout behavior, or presentation conventions, explain the reason and identify implementation or migration risks.

This file is development guidance only. Do not load it at runtime, duplicate it in C source or level configuration, or add a runtime dependency on `.github`.

## Rule Priority

When rules conflict, apply this priority:

1. Correct x86 and game-specific semantics
2. Player comprehension
3. Accessibility
4. Consistency with this specification
5. Brevity
6. Firmware-style presentation

Never preserve concise wording when it teaches an incorrect architectural model. Clarity takes priority over forced architectural wording.

## Mandatory Language Rules

### Canonical Terminology

Prefer these nouns:

- Instruction
- Instruction sequence
- Execution
- Source operand
- Destination operand
- Register
- Buffer
- Input buffer
- Output buffer
- Flag
- Data path
- Control flow
- Processor state
- Constraint
- Result
- Immediate
- Address
- Jump target

Prefer these interaction verbs:

- Acquire
- Assign
- Place
- Commit
- Remove
- Release
- Reorder
- Inspect
- Execute
- Reset

Use terminology consistently. Do not switch casually between:

- Run, play, and execute
- Delete and remove
- Code, program, and instruction sequence
- Input, input buffer, and source buffer
- Output, output buffer, and destination buffer

Use `execute` and `execution` for processor operation; `remove` for deleting an instruction from the instruction sequence; and `instruction sequence` for the assembly instructions constructed by the player. Use `input buffer` and `[ib]`, or `output buffer` and `[ob]`, according to whether prose or the architectural identifier is required.

Source code may retain established internal identifiers such as `play_state` when changing them would create unnecessary risk. Player-facing language must still use `execute` and `execution`.

### Architectural Identifiers

Preserve exact architectural identifiers. Examples include `MOV`, `ADD`, `SUB`, `CMP`, `JMP`, `rax`, `rbx`, `[ib]`, `[ob]`, and `ZF`.

Do not title-case, pluralize, translate, or rewrite architectural identifiers. Do not replace `[ib]` with `[IB]` or `rax` with `RAX` unless the simulated architecture is intentionally changed across the entire game.

### Tone and Interaction Language

Use concise, neutral, firmware-style language. Describe machine conditions, architectural effects, required operations, constraints, and observable results. Avoid motivational language, personal blame, conversational filler, and unnecessary direct address.

Avoid phrases such as `You need to`, `Your program`, `Your code`, `Try again`, `Good job`, `Well done`, `Now you can`, `You made an error`, and `You selected the wrong operand`.

Prefer phrases such as `Instruction limit exceeded`, `Source operand missing`, `Output sequence incomplete`, `MOV does not accept this operand`, `Assign rax as the destination operand`, `Initiate sequence execution`, `Constraint not satisfied`, and `Execution halted`.

Avoid unnecessary interface terminology such as `Click`, `Drag`, `Button`, `Box`, `Panel`, `Screen`, `Interface`, and `Mouse`. Describe the intended operation when it remains clear, for example: `Acquire MOV`, `Assign rax`, `Place the instruction at the sequence entry`, `Commit MOV to the instruction sequence`, `Release the instruction outside the sequence`, `Reorder the instruction sequence`, `Inspect the processor state`, and `Initiate sequence execution`.

Use direct physical interaction language when omitting it would make the required action ambiguous. For example, `Release the instruction outside the sequence` is acceptable when the release location determines removal. Do not conceal necessary interaction information to maintain the firmware tone.

Use familiar navigation labels when they are clearer: `Continue`, `Restart`, `Settings`, `Return`, `Quit`, `Confirm`, and `Cancel`.

### Tutorial Content and Directives

Each tutorial step should communicate one primary concept or request one primary operation. Separate content by semantic role:

- Explanatory prose describes architecture or processor behavior.
- `@syntax` presents exact instruction or data-path notation.
- `@directive` requests a player operation.
- Result content reports execution outcomes or processor-state changes.

Preferred structure:

```text
MOV transfers the source operand into the destination operand

@syntax MOV destination, source

@directive Acquire MOV
```

Do not repeat the same directive using multiple phrasings in one step. Avoid combining architecture explanation, interaction instructions, exception handling, and result reporting into one dense message.

Tutorial text must agree with the highlighted component, arrow target, enabled and disabled interactions, current processor state, current instruction sequence, completion conditions, and dismissal conditions. Use visible terminology; do not refer to a control or system using a name the player cannot identify.

Write directives as concise imperative phrases, for example `Inspect the challenge specification`, `Acquire MOV`, `Commit MOV to the instruction sequence`, `Assign rax`, `Assign [ib]`, `Reorder the instruction sequence`, and `Initiate sequence execution`. Do not add endpoint punctuation to tutorial directives. Use one directive per tutorial step unless the actions form one indivisible operation.

### Explanations and Results

Explanatory text describes an architectural effect or game-system behavior. Examples:

- MOV transfers the source operand into the destination operand
- Source provides the value
- Destination receives the value
- The value transfers from [ib] into rax
- Values read from [ib] are consumed
- Register values persist across executions

Keep explanations technically accurate within the simulation. When the game simplifies x86 behavior, describe the implemented model rather than making an unbounded claim about all x86 processors.

Use result language for outcomes caused directly by execution. Examples:

- Execution complete
- Output sequence matches the target
- Output sequence incomplete
- rax contains 5
- ZF is set
- Execution halted: input buffer empty

Do not use praise as an execution result. Report observable state or comparison instead.

### Punctuation and Capitalization

Tutorial messages do not end with periods or other endpoint punctuation. Endpoint punctuation may be used when it belongs to assembly syntax, is required to avoid ambiguity, or an established non-tutorial UI convention requires it.

Preserve syntax punctuation, including commas, brackets, colons, arrows, signed immediates, and labels:

- `MOV rax, [ib]`
- `loop:`
- `-1`
- `[ib] -> rax -> [ob]`

Do not remove syntax punctuation to satisfy the no-endpoint-punctuation rule.

Use sentence case for prose. Use uppercase for displayed headers and intentional level or address highlights. Keep assembly mnemonics uppercase (`MOV`, `ADD`, `SUB`, `CMP`, `JMP`) and registers and buffer identifiers in their architectural form (`rax`, `rbx`, `[ib]`, `[ob]`).

Use lowercase in configuration identifiers when required by the existing parser, such as `system_message`, `system_notice`, `system_warning`, and `instruction`. Do not confuse configuration identifiers with their rendered uppercase labels.

## Existing Configuration and Rendering Contracts

Preserve the established configuration grammar. Do not add names for headers, tags, colors, or configuration features unless the relevant parser and renderer support them.

Tutorial files use named sections and text blocks delimited by `text_begin` and `text_end`. The parser also handles `header`, `box`, `box_size`, `text_size`, `dismiss`, state conditions such as `when.code_size`, effects such as `effects.code_editable`, and presentation targets such as `highlight`, `highlight_instruction`, and `arrow`. Keep their values and spelling aligned with `source/tutorial_tr.c` and the relevant declarations. Operand feedback has its own parser and accepts a narrower configuration grammar; do not assume tutorial properties are valid there.

Authoring prefixes recognized by the styled-text renderer are `@directive`, `@syntax`, and `@warning`. They are presentation tags, not tutorial configuration keys. Use the exact supported prefixes and preserve their semantic roles. Do not invent additional prefixes.

Header values are parser-specific:

- Tutorial `header` values currently support `none`, `system_message`, `system_notice`, `system_warning`, and `instruction`.
- Operand-feedback `header` values additionally support `instruction_feedback`; its rendered label is `INSTRUCTION FEEDBACK`.
- The shared renderer currently provides `SYSTEM MESSAGE`, `SYSTEM NOTICE`, `SYSTEM WARNING`, `INSTRUCTION`, and `INSTRUCTION FEEDBACK` labels, or no label for `none`.
- `system_error` and `run_result` are not currently supported `header` values. Do not use them in configuration or document them as existing parser identifiers.
- Execution results are handled by the run-result UI module; `RUN RESULT` is a semantic role, not a supported header key.

Do not select a warning or error presentation merely for visual emphasis, and do not add a header when the component does not require one.

### Header Semantics

Apply these semantic roles using only supported configuration values and rendering APIs:

**SYSTEM MESSAGE** is for context, initialization, addresses, completion, and neutral system states. Examples: `ADDRESS 00`, `SYSTEM READY`, `SECTOR COMPLETE`.

**INSTRUCTION** is for required operations, rules, constraints, operand assignments, instruction editing, and sequence manipulation.

**SYSTEM NOTICE** is for architectural explanations, newly available capabilities, non-error state changes, and processor behavior explanations.

**SYSTEM WARNING** is for recoverable or non-fatal conditions, conditions requiring attention, and states from which the player can continue after correction.

**SYSTEM ERROR** describes an error severity in prose, not a currently supported header value. Use only a header the relevant parser supports; do not imply that a `system_error` header exists.

**RUN RESULT** is the semantic role for outcomes produced directly by execution, processor-state results, output comparisons, and execution completion states. Results are owned by the run-result UI; do not imply that a `run_result` configuration header exists.

Severity must reflect actual behavior. A recoverable condition is not a system error merely because an operation was rejected.

### Error Language

Describe the machine condition rather than personal failure. State the problem first. Add recovery guidance only when it provides useful information not already visible through highlighting or interaction restrictions.

Prefer specific messages such as `Instruction limit exceeded`, `Source operand missing`, `Destination operand missing`, `Output sequence incomplete`, `MOV does not accept this operand`, `Jump target unresolved`, `Input buffer empty`, `Execution halted`, `Constraint not satisfied`, `Source operand is not valid for MOV`, and `Execution halted: input buffer empty`.

Avoid `You selected the wrong operand`, `Your code is incorrect`, `You made an error`, `Try again`, and `Invalid choice`. Prefer specific conditions over generic messages such as `Invalid operation`.

## Design Guidance

### Layout, Typography, and Overflow

Player-facing text must fit at supported resolutions, UI scales, and content boundaries. Text overflow is a non-fatal layout warning and must not invalidate an otherwise functional tutorial.

When text does not fit:

1. Confirm that the correct typography role and box size are used.
2. Simplify wording without removing required meaning.
3. Reuse existing text measurement and layout APIs.
4. Consider an established larger semantic box size.
5. Correct the owning layout system if the problem affects multiple messages.

Do not fix overflow with level-specific coordinates, arbitrary offsets, unrelated font-size reductions, manual line breaks that only work at one resolution, silent truncation, or a tutorial failure state. Manual line breaks may express semantic grouping when they remain robust under supported layouts.

Keep exact color values, typography metrics, box dimensions, and layout coordinates owned by existing centralized UI modules. Reuse existing text measurement and layout APIs rather than creating one-off values.

### Visual Language and Accessibility

Preserve the restrained terminal and diagnostic-system identity. Prefer limited color use, high contrast, stable semantic colors, clear hierarchy, monospaced presentation where established, consistent borders, spacing and typography roles, visible processor-state changes, and functional animation.

Avoid decorative color without semantic purpose, excessive gradients or effects, celebration effects that conflict with the terminal tone, severity colors used for ordinary emphasis, dense text that competes with the simulated architecture, and animation that hides or delays an important state transition.

Color must not be the only carrier of meaning. Pair it with at least one of a header, label, shape, border treatment, icon, spatial relationship, or state change. Do not invent exact color values here; inspect and reuse the project's existing palette and rendering APIs when changing colors.

Maintain readable contrast and clear hierarchy. Do not rely only on color, animation, position, sound, or hover state. Critical messages must remain understandable without audio, and semantic states must remain distinguishable when animation is disabled, interrupted, or complete.

Prefer concise text, but do not remove words needed to distinguish source from destination, input from output, warning from error, instruction from execution result, or architectural state from player action.

## Review Checklist

Before adding or changing player-facing text, verify:

- Is the x86 or simulated architectural meaning correct?
- Does the message describe the machine rather than blame the player?
- Is canonical terminology used and are architectural identifiers preserved exactly?
- Does the supported header match semantic role and actual severity?
- Does the step communicate one primary concept or operation?
- Does the directive match the enabled interaction?
- Does wording match arrows and highlights?
- Is endpoint punctuation omitted where required, and is capitalization consistent?
- Does the text fit supported layouts?
- Is meaning available without relying only on color?
- Does equivalent text elsewhere need the same update?
- Are parser, rendering, or regression tests required?

## Change Discipline

Do not rewrite all existing UI text merely because this file is introduced. When a task touches existing player-facing text:

1. Inspect related messages for consistency.
2. Correct directly related inconsistencies.
3. Keep the patch focused.
4. Avoid broad wording migrations without dedicated review.
5. Preserve exact configuration syntax.
6. Add or update tests when wording is contractually significant.
7. Report intentional terminology changes.

If existing behavior conflicts with this specification, determine whether the conflict is a legacy inconsistency, a required parser limitation, an intentional gameplay exception, or a technically necessary architectural term. Document material exceptions instead of silently creating a second convention.
