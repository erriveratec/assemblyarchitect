# Visual Baseline Policy

Existing tests assert deterministic software-rendered pixels/geometry; there is
no approved screenshot-golden update pipeline. Do not create or regenerate golden
files merely to make a test green.

Capture with fixed source revision, level/player fixture, DOSVGA437 font, SDL/TTF
versions, canonical 1920x1080 logical canvas, renderer backend, pixel format, and
scale. Pin assets, record physical dimensions when relevant, control animation
phase/clock and input state explicitly, and avoid arbitrary waits. Capture narrow
regions which isolate the behavior; compare geometry separately from glyph output.

Each future baseline must document its numeric tolerances and their measured
reason. Exact assertions remain exact unless a human approves a justified change.
Never globally loosen tolerances or replace a failing image automatically.
Before/after artifacts, capture metadata, invariant impact, and explanation belong
in the PR. A dedicated maintainer-created visual-baseline label or explicit written
maintainer approval is mandatory. Agents may propose but never approve updates.
Font rasterization/platform differences are investigated, not silently normalized.

No user saves or full workspace uploads. Sanitized isolated fixtures only. A human
must also run the real-display smoke checklist; offscreen pixels cannot prove
window scaling, device behavior, audio, or full instructional correctness.
