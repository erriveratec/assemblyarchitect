# Engineering Baseline

## Reproduction

Baseline revision: `83d21d2`, initially clean working tree. Platform: macOS arm64,
Apple clang 21.0.0, SDL2 frameworks in `/Library/Frameworks`.

Initial commands, before edits:

```sh
git status --short
make clean
make build
make -k test
make -k test-operand-highlights test-registers-highlight-geometry test-tutorial-arrow-animation
make -B build CFLAGS='-g -Isource -Wall -Wextra -Wpedantic'
make -k test CFLAGS='-g -Isource -Wall -Wextra -Wpedantic'
```

The original normal build emitted zero warnings because it enabled no optional
warning groups. The original suite had 15 targets: 12 passed, two failed compilation
(14 errors total), and one aborted. All three failures reproduced individually.
Every test file contains behavioral assertions; none was an assertion-free success.
Tests use SDL software rendering with `SDL_Init(0)`, not a video-driver workaround.

Complete original outputs were retained in `/tmp/assemblygame-baseline-build.log`,
`/tmp/assemblygame-baseline-tests.log`, `/tmp/assemblygame-baseline-reruns.log`,
`/tmp/assemblygame-strict-build.log`, and `/tmp/assemblygame-strict-tests.log`.
The diagnostic appendix below preserves the strict probe's exact source locations
and messages without repeated compilations of the same source/header.

## Original Failures

| Target and original location | Classification | Evidence and correction |
| --- | --- | --- |
| `test-operand-highlights`, `tests/operand_highlights_test.c:164` and subsequent style assertions | Obsolete expectation / stale API | Commit `8cb7058` replaced `dw_get_render_style` and `DW_RENDER_STYLE_*` with explicit texture colors. Test source-owner composition and public animation progress; retain rendered white-glyph and grey-frame checks. |
| Same target, newly reachable pixel assertion in `test_immediate_grid_geometry` | Obsolete expectation | The current renderer pulses white glyphs inside unchanged grey frames, not a cell-sized white region. Assert frame equality, enclosed visible glyphs, nonshrinking pulse, canonical bounds, and restoration. |
| `test-registers-highlight-geometry`, `tests/registers_highlight_geometry_test.c:26` and style mocks | Test defect / stale API | Mocks used deleted `dw_render_style_t` signatures. Intercept the actual draw/color functions, assert grey value boxes, white pulsing labels, grey restoration, positive in-window geometry, and no accumulated offsets. |
| `test-tutorial-arrow-animation`, `tests/tutorial_arrow_animation_test.c:797` | Intentional configuration change | Commits `3739fe6` and `d991ac4` deliberately removed Level 13 ZF arrows in favor of CMP and ZF highlights. Assert zero arrows and both highlight targets, activation and cleanup. Level 14 still requires `AR_ZF` with its real provider. No configuration was changed. |

## Warning Classifications

The strict initial probe produced 645 distinct diagnostics after normalizing
`tests/../source` paths. Counts include different diagnostic columns at one line.

| Diagnostic | Count | Classification and root-cause repair |
| --- | ---: | --- |
| `-Wstrict-prototypes` | 466 | Stale C declarations: compiler-identified zero-argument declarations/definitions now use `(void)`, including public headers. |
| `-Wbitwise-instead-of-logical` | 1 | Boolean modal-state queries used bitwise OR. Use logical OR; both queries are side-effect-free. |
| `-Wunused-variable` | 65 | Dead layout/state calculations, plus argument-discarding test macros. Remove dead declarations; preserve typewriter calls; use typed fixture stubs. |
| `-Wunused-function` | 37 | Stale private declarations/helpers. Remove unreferenced helpers; compile optional register-value helpers under their existing feature flag. |
| `-Wsign-compare` | 17 | Type contracts: signed values use a signed `NO_VALUE`; signed level IDs have signed bounds; array indices use `size_t`; enum comparisons use the enum type. |
| `-Wsometimes-uninitialized` | 30 | Undefined return/error paths or compiler-unproven exhaustive branches. Initialize results before error jumps, return explicit missing/invalid values, release intermediate allocations, and express exhaustive alternatives. Removed dead helpers need no replacement return path. |
| `-Wunused-but-set-variable` | 6 | Dead timestamps/traversal state, or fixture macros discarding live values. Remove dead state and use typed mocks for live state. |
| `-Wunused-const-variable` | 1 | Obsolete sector typing-delay constant removed. |
| `-Wunused-label` | 6 | Error labels with no incoming jumps removed; return statements retained. |
| `-Wempty-translation-unit` | 1 | Empty legacy `source/ui/reset_menu_rm.c` removed. Actual reset-menu implementation remains in `rest_menu_rm.c`. |
| `-Wgnu-zero-variadic-macro-arguments` | 2 | Platform/compiler assumption: logging/check macros now use standard variadic arguments without GNU comma pasting. |
| `-Wunused-parameter` | 6 | Unused private reset/effect parameters removed; test macros replaced with signature-preserving stubs. |
| `-Wlogical-op-parentheses` | 6 | Ambiguous operand predicates simplified without changing valid eligibility. |
| `-Wtautological-overlap-compare` | 1 | Empty reserved memory-operand range removed from the validation predicate; no memory operands were previously accepted. |

Later checks exposed dependent dead locals/helpers, enum warnings previously masked
by test compile errors, and three sanitizer-build `sprintf` deprecations in
`ax_number_to_string`. Bounded `snprintf` replaces floating-point digit counting
and `abs`, preserves the no-value dash, checks allocation failure, and handles
`INT_MIN` without signed overflow. Integer-extreme regression cases were added.

During implementation, focused validation caught incorrect test token spelling,
missing fixture initialization, one incorrect cleanup API name, an overly strict
mock animation-limit assumption, and a misapplied comment-context patch. These
were repaired and rerun; none is a final diagnostic. The intermediate ASan null
read in `cw_get_code_line_x` was an invalid new fixture missing code-window number
textures; initializing its normal assets fixed it without a production workaround.

## Production Changes

- Successful instruction execution and CMP/JE/JNE semantics are unchanged.
- Incomplete saved instructions preserve absent operands and missing-operand states;
  queries return `NO_OPERAND` rather than reading an absent operand.
- Allocation failures return `NULL` and release intermediate objects where repaired.
- Invalid avatar destinations produce explicit failure; valid I/O routing is unchanged.
- `NO_VALUE` retains its stored signed value, now without unsigned-to-signed conversion.
- Optional animation implementations use the existing feature flag, not a warning suppression.
- Modal ordering, pointer consumption, and centralized `rr_is_valid_operation` are unchanged.

## Test Changes

| File | Previous assumption | New invariant and protection |
| --- | --- | --- |
| `tests/operand_highlights_test.c` | Deleted render-style API and whole-cell white highlight; shared fixture state | Public source composition/progress, real white glyphs within unchanged grey frames, restored geometry, explicit state cleanup; saved missing-operand states, numeric extremes, strict `zero_flag` naming. |
| `tests/registers_highlight_geometry_test.c` | Deleted style mocks and highlighted value box | Typed color/draw capture proves label-only white pulse, unchanged grey value box, canonical metric-derived geometry and repeated-render stability. |
| `tests/tutorial_arrow_animation_test.c` | Level 13 always has ZF arrows | Intentional Level 13 CMP/ZF highlighting and Level 14 ZF arrows; all 17 level files load with valid arrow IDs; mandatory-provider behavior remains tested; provider state resets between cases. |
| `tests/operand_change_selection_test.c` | Only existing-operand queries exercised | Missing operand queries return `NO_OPERAND`; existing selection and replacement assertions retained. |
| `tests/io_rail_geometry_test.c` | Only valid routes exercised | Invalid IDs/unsupported avatar route reject movement without geometry mutation; all existing positive route checks retained. |
| `tests/sector_completion_navigation_test.c` | No-op macros discarded live call arguments | Typed stubs check dimensions, timing, operation and held-line state; all navigation, one-shot win, click-leak checks retained. |
| `tests/run_result_render_order_test.c` | Render mocks discarded animation limits | Typed event recording validates positive fixture limits; all layer-order and modal-action assertions retained. |
| `tests/highlight_animation_sync_test.c` | Unused copied offset calculations | Remove only dead local calculations; all pixel, animation, and source-owner assertions retained. |

No test was deleted, skipped, made expected-to-fail, or given wider tolerances.

## Build Policy

`Makefile` applies `-Wall -Wextra -Wpedantic -Werror` to production and tests,
including command-line `CFLAGS` overrides. SDL include directories use `-isystem`;
there are no project-warning suppressions. Application dependencies include headers
and the Makefile. The complete executable inventory now contains all 15 tests,
including value-box and register-geometry binaries, and drives aggregate targets
and legacy-artifact cleanup. Test targets remain phony compile-and-run targets,
so source/header changes cannot leave stale test executables. `make test` retains
normal make failure semantics; `make -k test` collects all failures.

VS Code now explicitly uses the existing compilation database, with macOS SDL
framework include paths as fallback for sources absent from that database. Removed
two entries for nonexistent translation units. The database remains a partial,
machine-specific developer aid, not the authoritative build recipe.

## Verification Commands

```sh
make clean
make -j4 build test
make test-operand-highlights
make test-registers-highlight-geometry
make test-tutorial-arrow-animation
make test-run-result-render-order test-modal-pointer-capture test-sector-completion-navigation
make -B build CFLAGS='-g -O1 -Isource -fsanitize=address,undefined -fno-omit-frame-pointer'
ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 make -k test \
  CFLAGS='-g -O1 -Isource -fsanitize=address,undefined -fno-omit-frame-pointer'
make clean
make build
make test
git diff --check
```

The normal default is a debug-symbol build (`-g`). Verification uses existing SDL
software-rendered fixtures; no GUI replay is claimed. ASan/UBSan cover exercised
test paths, not every gameplay path or leak detection unavailable on this platform.

## Final Results

| Check | Result |
| --- | --- |
| Clean normal build | Pass; zero compiler/linker warnings or errors |
| Aggregate normal tests | 15/15 passed; zero warnings |
| Individual original failures and modal/navigation regressions | 6/6 passed; zero warnings |
| Clean parallel build and aggregate tests | Pass; all 15 tests passed |
| ASan/UBSan application build | Pass; zero warnings |
| ASan/UBSan aggregate tests | 15/15 passed; zero sanitizer findings or warnings |
| VS Code diagnostics after include configuration | No errors found |
| Editor JSON and compilation-database source paths | Valid; no nonexistent source paths |
| `git diff --check` | Pass |

Final logs are `/tmp/assemblygame-final-build.log`,
`/tmp/assemblygame-final-tests.log`, `/tmp/assemblygame-final-individual.log`,
`/tmp/assemblygame-final-sanitizer-build.log`, and
`/tmp/assemblygame-final-sanitizer-tests.log`. The final application and test
binaries were rebuilt with normal debug-symbol flags after sanitizer verification.

## Remaining Issues

No failing build/test target or editor error remains in the verified environment.
Manual GUI replay and leak detection were not performed. Save/load round-trip
compatibility was not established: the new constructor fixtures use `IMMUP7`,
not bare numeric `7`, which the constructor rejects. The storage loader forwards
tokens directly; numeric-token compatibility was not changed by this cleanup.

## Preserved Behavior

Result dialogs remain the final gameplay layer, including above Level 13 ZF UI.
Reset and escape remain higher-priority application modals. Active results capture
pointer input; BACK/CONTINUE closing clicks do not reach exposed controls;
`NO_OPERATION` permits gameplay input. CMP sets ZF for equality, JE branches on
set ZF, and JNE branches on clear ZF. No tutorial text/configuration was modified.

## Changed Files

The following inventory separates prototype-only files from substantive repairs.

| File | Purpose |
| --- | --- |
| ENGINEERING_BASELINE.md | Evidence, classifications, complete diagnostic/file inventories, verification. |
| .vscode/c_cpp_properties.json | Select existing compilation database and provide SDL framework fallback include paths. |
| compile_commands.json | Remove two nonexistent translation-unit entries. |
| Makefile | Shared warning enforcement, system SDL includes, complete test inventory, header dependencies. |
| README.md | Document strict builds, headless tests, and sanitizer commands. |
| source/arrow_ar.c | Explicit prototypes; remove uncalled execution-arrow helper. |
| source/arrow_ar.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/aux.c | Defined rectangle branch and allocation cleanup; bounded integer formatting. |
| source/aux.h | Signed no-value sentinel; explicit prototypes. |
| source/buffers_bf.c | Remove dead locals; express exhaustive buffer coordinate alternatives. |
| source/code_line_cl.c | Remove unreachable labels and empty memory range; clarify valid operand predicate. |
| source/code_window_cw.c | Defined query/error returns, missing saved operands, redundant traversal/dead-code removal, prototypes. |
| source/code_window_cw.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/dbg.h | Portable variadic logging and single-statement check macros. |
| source/draw_dw.c | Remove unused private rendering helpers/labels/locals; explicit prototypes. |
| source/draw_dw.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/electron_fx.c | Remove unused dimensions from private trace measurement signature and call. |
| source/file_fl.c | Defined string allocation failure cleanup; remove unused private legacy parsers/helpers. |
| source/game_mechanics_mc.c | Explicit invalid routing, shared value lookup, dead declarations, prototypes. |
| source/game_mechanics_mc.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/gameplay/code_state_cs.c | Remove unused context snapshot. |
| source/gameplay/code_state_cs.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/gameplay/win_condition_wc.c | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/gameplay/win_condition_wc.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/immediates_im.c | Exhaustive numeric branch, unused layout local, prototypes. |
| source/instruction_window_iw.c | Remove unused layout/list locals; prototypes. |
| source/instruction_window_iw.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/levels_lv.c | Signed level bounds, dead legacy declarations, prototypes. |
| source/levels_lv.h | Signed level bounds and explicit prototypes across the API. |
| source/list.c | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/list.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/main.c | Remove unused timer; explicit prototypes. |
| source/media/audio_au.c | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/media/audio_au.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/mouse_ms.c | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/mouse_ms.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/players_pl.c | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/players_pl.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/registers_rg.c | Constructor cleanup; optional-helper guards; dead locals/declarations; prototypes. |
| source/registers_rg.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/sdl_config.c | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/sdl_config.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/stage_buttons_sb.c | Logical modal-state predicate, stale declarations/constant, prototypes. |
| source/stage_buttons_sb.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/stages.c | Remove unused private reset parameter, locals/timestamp, stale declarations; prototypes. |
| source/storage/save_sv.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/text_tx.c | Remove unused box accessor; prototypes. |
| source/text_tx.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/ui/button_bt.c | Remove dead sizing/state locals; prototypes. |
| source/ui/button_bt.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/ui/escape_menu_em.c | Remove unused layout locals; prototypes. |
| source/ui/escape_menu_em.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/ui/reset_menu_rm.c | Delete empty translation unit; retain actual implementation in rest_menu_rm.c. |
| source/ui/reset_menu_rm.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/ui/rest_menu_rm.c | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/ui/screens/select_level_sl.c | Reject invalid sector, remove unused dimensions/timers/declaration, prototypes. |
| source/ui/screens/select_level_sl.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/ui/screens/select_player_sp.c | Keep typewriter side effects while removing unused result/state/label; prototypes. |
| source/ui/screens/select_player_sp.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/ui/screens/select_sector_ss.c | Keep typewriter side effects while removing unused result/layout/constants; prototypes. |
| source/ui/screens/select_sector_ss.h | Explicit zero-argument C prototypes; no runtime behavior change. |
| source/ui/screens/title_screen_ts.c | Match private timing constants to delay API; remove dead sound flag. |
| tests/highlight_animation_sync_test.c | See the per-test contract and coverage table above. |
| tests/io_rail_geometry_test.c | See the per-test contract and coverage table above. |
| tests/operand_change_selection_test.c | See the per-test contract and coverage table above. |
| tests/operand_highlights_test.c | See the per-test contract and coverage table above. |
| tests/registers_highlight_geometry_test.c | See the per-test contract and coverage table above. |
| tests/run_result_render_order_test.c | See the per-test contract and coverage table above. |
| tests/sector_completion_navigation_test.c | See the per-test contract and coverage table above. |
| tests/tutorial_arrow_animation_test.c | See the per-test contract and coverage table above. |

## Original Strict Diagnostics

Locations refer to the original revision, not post-cleanup line numbers. Each row
identifies whether the diagnostic occurred in `build`, aggregate `test`, or both.
The classification and repair for each warning code are given above.

| Targets | Original diagnostic |
| --- | --- |
| build, test | source/arrow_ar.c:100:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:101:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:102:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:103:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:104:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:105:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:106:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:107:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:108:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:111:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:141:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:227:13: warning: unused function 'check_execution_arrow_in_place' [-Wunused-function] |
| build, test | source/arrow_ar.c:250:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:278:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:294:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:324:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:353:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:385:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:420:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:456:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:485:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:517:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:545:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:574:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:609:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:637:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:674:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:707:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:734:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:764:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:793:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:96:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:97:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:98:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.c:99:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.h:53:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.h:55:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.h:56:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/arrow_ar.h:58:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/aux.c:111:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/aux.c:135:13: warning: variable 'r' is used uninitialized whenever 'if' condition is false [-Wsometimes-uninitialized] |
| build, test | source/aux.c:244:2: warning: variable 'string' is used uninitialized whenever 'if' condition is true [-Wsometimes-uninitialized] |
| build, test | source/aux.c:522:20: warning: comparison of integers of different signs: 'int' and 'unsigned int' [-Wsign-compare] |
| build, test | source/aux.c:620:6: warning: unused variable 'x_pos' [-Wunused-variable] |
| build, test | source/aux.h:69:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/aux.h:70:28: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/aux.h:71:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/buffers_bf.c:1124:8: warning: unused variable 'input_list' [-Wunused-variable] |
| build, test | source/buffers_bf.c:1129:13: warning: variable 'x' is used uninitialized whenever 'if' condition is false [-Wsometimes-uninitialized] |
| build, test | source/buffers_bf.c:1148:8: warning: unused variable 'input_list' [-Wunused-variable] |
| build, test | source/buffers_bf.c:1153:13: warning: variable 'y' is used uninitialized whenever 'if' condition is false [-Wsometimes-uninitialized] |
| build, test | source/buffers_bf.c:1212:6: warning: unused variable 'ofs' [-Wunused-variable] |
| build, test | source/code_line_cl.c:503:1: warning: unused label 'error' [-Wunused-label] |
| build, test | source/code_line_cl.c:637:1: warning: unused label 'error' [-Wunused-label] |
| build, test | source/code_line_cl.c:848:19: warning: '&&' within '\|\|' [-Wlogical-op-parentheses] |
| build, test | source/code_line_cl.c:849:22: warning: '&&' within '\|\|' [-Wlogical-op-parentheses] |
| build, test | source/code_line_cl.c:849:22: warning: non-overlapping comparisons always evaluate to false [-Wtautological-overlap-compare] |
| build, test | source/code_line_cl.c:850:19: warning: '&&' within '\|\|' [-Wlogical-op-parentheses] |
| build, test | source/code_line_cl.c:851:19: warning: '&&' within '\|\|' [-Wlogical-op-parentheses] |
| build, test | source/code_window_cw.c:1018:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:1022:6: warning: unused variable 'x' [-Wunused-variable] |
| build, test | source/code_window_cw.c:1047:24: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:1080:23: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:1130:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:114:31: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:1154:31: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:1243:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:1248:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:1310:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:137:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:1499:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:1506:11: warning: unused variable 'cbut' [-Wunused-variable] |
| build, test | source/code_window_cw.c:1543:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:1595:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:1597:8: warning: unused variable 'code' [-Wunused-variable] |
| build, test | source/code_window_cw.c:1700:6: warning: unused variable 'w' [-Wunused-variable] |
| build, test | source/code_window_cw.c:1741:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:1752:6: warning: unused variable 'number_ofs' [-Wunused-variable] |
| build, test | source/code_window_cw.c:1803:2: warning: variable 'y' is used uninitialized whenever 'for' loop exits because its condition is false [-Wsometimes-uninitialized] |
| build, test | source/code_window_cw.c:1825:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:1835:11: warning: unused variable 'cb' [-Wunused-variable] |
| build, test | source/code_window_cw.c:1841:8: warning: unused variable 'number' [-Wunused-variable] |
| build, test | source/code_window_cw.c:1879:11: warning: unused variable 'cb' [-Wunused-variable] |
| build, test | source/code_window_cw.c:1903:11: warning: unused variable 'cb' [-Wunused-variable] |
| build, test | source/code_window_cw.c:1952:18: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:1965:6: warning: unused variable 'h' [-Wunused-variable] |
| build, test | source/code_window_cw.c:206:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:2078:11: warning: unused variable 'cb' [-Wunused-variable] |
| build, test | source/code_window_cw.c:2108:6: warning: unused variable 'list_count' [-Wunused-variable] |
| build, test | source/code_window_cw.c:2224:24: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:2330:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:2357:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:2384:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:2412:46: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:2489:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:2510:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:2519:6: warning: unused variable 'h' [-Wunused-variable] |
| build, test | source/code_window_cw.c:2584:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:2632:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:2785:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:2788:9: warning: unused variable 'x' [-Wunused-variable] |
| build, test | source/code_window_cw.c:2807:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:2816:2: warning: variable 'new' is used uninitialized whenever 'if' condition is true [-Wsometimes-uninitialized] |
| build, test | source/code_window_cw.c:301:1: warning: unused label 'error' [-Wunused-label] |
| build, test | source/code_window_cw.c:316:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:400:21: warning: unused function 'get_clicked_label_code_line' [-Wunused-function] |
| build, test | source/code_window_cw.c:400:48: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:403:2: warning: variable 'clicked_label' is used uninitialized whenever 'if' condition is true [-Wsometimes-uninitialized] |
| build, test | source/code_window_cw.c:435:24: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:438:2: warning: variable 'selected' is used uninitialized whenever 'if' condition is true [-Wsometimes-uninitialized] |
| build, test | source/code_window_cw.c:466:38: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:49:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:52:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:55:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:60:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:61:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:611:2: warning: variable 'label_counter' is used uninitialized whenever 'if' condition is true [-Wsometimes-uninitialized] |
| build, test | source/code_window_cw.c:63:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:648:2: warning: variable 'label' is used uninitialized whenever 'if' condition is true [-Wsometimes-uninitialized] |
| build, test | source/code_window_cw.c:649:2: warning: variable 'label' is used uninitialized whenever 'if' condition is true [-Wsometimes-uninitialized] |
| build, test | source/code_window_cw.c:655:6: warning: unused variable 'list_size' [-Wunused-variable] |
| build, test | source/code_window_cw.c:663:16: warning: variable 'c' set but not used [-Wunused-but-set-variable] |
| build, test | source/code_window_cw.c:67:38: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:68:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:70:48: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:72:31: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:779:11: warning: unused variable 'cb' [-Wunused-variable] |
| build, test | source/code_window_cw.c:800:14: warning: variable 'op1' is used uninitialized whenever '&&' condition is false [-Wsometimes-uninitialized] |
| build, test | source/code_window_cw.c:800:14: warning: variable 'op1' is used uninitialized whenever 'if' condition is false [-Wsometimes-uninitialized] |
| build, test | source/code_window_cw.c:811:14: warning: variable 'op2' is used uninitialized whenever '&&' condition is false [-Wsometimes-uninitialized] |
| build, test | source/code_window_cw.c:811:14: warning: variable 'op2' is used uninitialized whenever 'if' condition is false [-Wsometimes-uninitialized] |
| build, test | source/code_window_cw.c:889:45: warning: '&&' within '\|\|' [-Wlogical-op-parentheses] |
| build, test | source/code_window_cw.c:890:40: warning: '&&' within '\|\|' [-Wlogical-op-parentheses] |
| build, test | source/code_window_cw.c:892:13: warning: variable 'operand' is used uninitialized whenever '&&' condition is false [-Wsometimes-uninitialized] |
| build, test | source/code_window_cw.c:892:13: warning: variable 'operand' is used uninitialized whenever 'if' condition is false [-Wsometimes-uninitialized] |
| build, test | source/code_window_cw.c:936:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:953:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:971:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:988:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.c:992:6: warning: unused variable 'x' [-Wunused-variable] |
| build, test | source/code_window_cw.h:22:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:23:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:32:23: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:33:24: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:35:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:36:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:39:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:40:18: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:41:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:42:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:43:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:44:46: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:45:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:48:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:49:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:53:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:55:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:59:24: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:61:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:63:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:64:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:69:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:72:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:73:24: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:75:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/code_window_cw.h:76:31: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/dbg.h:19:22: warning: token pasting of ',' and __VA_ARGS__ is a GNU extension [-Wgnu-zero-variadic-macro-arguments] |
| build, test | source/dbg.h:29:14: warning: token pasting of ',' and __VA_ARGS__ is a GNU extension [-Wgnu-zero-variadic-macro-arguments] |
| build, test | source/draw_dw.c:101:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/draw_dw.c:122:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/draw_dw.c:156:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/draw_dw.c:171:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/draw_dw.c:183:38: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/draw_dw.c:198:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/draw_dw.c:66:43: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/draw_dw.c:67:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/draw_dw.c:68:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/draw_dw.c:82:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/draw_dw.c:823:6: warning: unused variable 'y_offset' [-Wunused-variable] |
| build, test | source/draw_dw.c:889:1: warning: unused label 'error' [-Wunused-label] |
| build, test | source/draw_dw.c:927:1: warning: unused label 'error' [-Wunused-label] |
| build, test | source/draw_dw.c:946:13: warning: unused function 'draw_text' [-Wunused-function] |
| build, test | source/draw_dw.h:63:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/draw_dw.h:68:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/draw_dw.h:69:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/draw_dw.h:70:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/electron_fx.c:193:53: warning: unused parameter 'W' [-Wunused-parameter] |
| build, test | source/electron_fx.c:193:59: warning: unused parameter 'H' [-Wunused-parameter] |
| build, test | source/file_fl.c:132:2: warning: variable 'id' is used uninitialized whenever 'if' condition is true [-Wsometimes-uninitialized] |
| build, test | source/file_fl.c:159:14: warning: unused function 'create_string_with_number' [-Wunused-function] |
| build, test | source/file_fl.c:164:2: warning: variable 'string' is used uninitialized whenever 'if' condition is true [-Wsometimes-uninitialized] |
| build, test | source/file_fl.c:190:14: warning: unused function 'get_delimeter_level_string' [-Wunused-function] |
| build, test | source/file_fl.c:193:2: warning: variable 'string' is used uninitialized whenever 'if' condition is true [-Wsometimes-uninitialized] |
| build, test | source/file_fl.c:282:13: warning: unused function 'parse_saved_code' [-Wunused-function] |
| build, test | source/file_fl.c:402:8: warning: unused variable 'text' [-Wunused-variable] |
| build, test | source/file_fl.c:501:13: warning: unused function 'parse_message' [-Wunused-function] |
| build, test | source/file_fl.c:61:14: warning: unused function 'get_player_end_string' [-Wunused-function] |
| build, test | source/file_fl.c:63:14: warning: unused function 'get_level_id_number' [-Wunused-function] |
| build, test | source/file_fl.c:64:14: warning: unused function 'get_delimiter_level_string' [-Wunused-function] |
| build, test | source/file_fl.c:67:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/file_fl.c:95:2: warning: variable 'id' is used uninitialized whenever 'if' condition is true [-Wsometimes-uninitialized] |
| build, test | source/game_mechanics_mc.c:1005:21: warning: comparison of integers of different signs: 'int' and 'unsigned int' [-Wsign-compare] |
| build, test | source/game_mechanics_mc.c:1037:21: warning: comparison of integers of different signs: 'int' and 'unsigned int' [-Wsign-compare] |
| build, test | source/game_mechanics_mc.c:111:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:142:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:158:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:1606:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:1634:46: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:1770:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:1800:23: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:1813:17: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:188:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:206:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:271:6: warning: unused variable 'ofs' [-Wunused-variable] |
| build, test | source/game_mechanics_mc.c:312:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:340:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:446:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:519:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:591:18: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:659:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:720:13: warning: variable 'y' is used uninitialized whenever 'if' condition is false [-Wsometimes-uninitialized] |
| build, test | source/game_mechanics_mc.c:745:13: warning: variable 'x' is used uninitialized whenever 'if' condition is false [-Wsometimes-uninitialized] |
| build, test | source/game_mechanics_mc.c:801:14: warning: variable 'y' is used uninitialized whenever 'if' condition is false [-Wsometimes-uninitialized] |
| build, test | source/game_mechanics_mc.c:82:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:880:13: warning: variable 'op_value_box' is used uninitialized whenever 'if' condition is false [-Wsometimes-uninitialized] |
| build, test | source/game_mechanics_mc.c:89:46: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:90:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:901:28: warning: comparison of integers of different signs: 'int' and 'unsigned int' [-Wsign-compare] |
| build, test | source/game_mechanics_mc.c:942:25: warning: comparison of integers of different signs: 'int' and 'unsigned int' [-Wsign-compare] |
| build, test | source/game_mechanics_mc.c:95:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:96:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:97:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.c:99:13: warning: unused function 'handle_ravatar_cmp' [-Wunused-function] |
| build, test | source/game_mechanics_mc.h:22:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.h:23:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.h:24:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.h:25:17: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.h:26:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.h:27:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.h:28:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.h:30:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.h:32:23: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/game_mechanics_mc.h:34:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/gameplay/code_state_cs.c:80:15: warning: unused variable 'context' [-Wunused-variable] |
| build, test | source/gameplay/code_state_cs.h:60:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/gameplay/code_state_cs.h:61:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/gameplay/win_condition_wc.c:866:31: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/gameplay/win_condition_wc.c:917:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/gameplay/win_condition_wc.h:10:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/gameplay/win_condition_wc.h:5:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/gameplay/win_condition_wc.h:6:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/gameplay/win_condition_wc.h:7:31: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/gameplay/win_condition_wc.h:8:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/gameplay/win_condition_wc.h:9:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/immediates_im.c:108:11: warning: unused variable 'input_bounds' [-Wunused-variable] |
| build, test | source/immediates_im.c:200:15: warning: variable 'num' is used uninitialized whenever 'if' condition is false [-Wsometimes-uninitialized] |
| build, test | source/immediates_im.c:421:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/immediates_im.c:48:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.c:118:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.c:137:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.c:154:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.c:172:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.c:195:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.c:215:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.c:218:9: warning: unused variable 'x' [-Wunused-variable] |
| build, test | source/instruction_window_iw.c:248:42: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.c:252:6: warning: unused variable 'x' [-Wunused-variable] |
| build, test | source/instruction_window_iw.c:287:11: warning: unused variable 'cb' [-Wunused-variable] |
| build, test | source/instruction_window_iw.c:30:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.c:31:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.c:326:28: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.c:348:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.c:44:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.h:10:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.h:11:42: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.h:12:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.h:16:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.h:18:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.h:19:28: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.h:7:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/instruction_window_iw.h:8:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:234:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| test | source/levels_lv.c:266:44: warning: unused parameter 'animation_limit' [-Wunused-parameter] |
| build, test | source/levels_lv.c:296:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:33:19: warning: unused variable 'g_lv_rules' [-Wunused-variable] |
| build, test | source/levels_lv.c:352:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:36:13: warning: unused function 'level_0' [-Wunused-function] |
| build, test | source/levels_lv.c:36:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:37:13: warning: unused function 'level_1' [-Wunused-function] |
| build, test | source/levels_lv.c:37:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:377:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:38:13: warning: unused function 'level_2' [-Wunused-function] |
| build, test | source/levels_lv.c:38:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:39:13: warning: unused function 'level_3' [-Wunused-function] |
| build, test | source/levels_lv.c:39:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:40:13: warning: unused function 'level_4' [-Wunused-function] |
| build, test | source/levels_lv.c:40:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:41:13: warning: unused function 'level_5' [-Wunused-function] |
| build, test | source/levels_lv.c:41:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:42:13: warning: unused function 'level_6' [-Wunused-function] |
| build, test | source/levels_lv.c:42:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:43:13: warning: unused function 'level_7' [-Wunused-function] |
| build, test | source/levels_lv.c:43:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:44:13: warning: unused function 'level_8' [-Wunused-function] |
| build, test | source/levels_lv.c:44:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:45:13: warning: unused function 'level_9' [-Wunused-function] |
| build, test | source/levels_lv.c:45:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:46:13: warning: unused function 'level_10' [-Wunused-function] |
| build, test | source/levels_lv.c:46:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:47:13: warning: unused function 'level_11' [-Wunused-function] |
| build, test | source/levels_lv.c:47:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:48:13: warning: unused function 'level_12' [-Wunused-function] |
| build, test | source/levels_lv.c:48:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:49:13: warning: unused function 'level_13' [-Wunused-function] |
| build, test | source/levels_lv.c:49:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:50:13: warning: unused function 'level_14' [-Wunused-function] |
| build, test | source/levels_lv.c:50:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:51:13: warning: unused function 'level_15' [-Wunused-function] |
| build, test | source/levels_lv.c:51:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:52:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:53:13: warning: unused function 'level_17' [-Wunused-function] |
| build, test | source/levels_lv.c:53:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:54:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:55:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.c:56:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.h:101:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.h:90:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.h:94:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.h:95:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.h:96:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.h:97:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/levels_lv.h:98:23: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/list.c:4:18: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/list.h:20:18: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build | source/main.c:31:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build | source/main.c:43:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build | source/main.c:87:9: warning: unused variable 'next_game_tick' [-Wunused-variable] |
| build, test | source/media/audio_au.c:28:19: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/media/audio_au.c:74:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/media/audio_au.h:20:19: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/media/audio_au.h:21:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.c:107:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.c:122:23: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.c:137:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.c:152:23: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.c:168:19: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.c:348:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.c:394:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.c:423:19: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.c:438:19: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.c:443:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.c:460:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.c:77:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.c:92:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.h:12:19: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.h:13:19: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.h:14:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.h:15:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.h:17:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.h:18:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.h:19:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.h:20:23: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.h:21:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.h:22:23: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.h:26:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.h:27:19: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.h:28:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/mouse_ms.h:29:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/players_pl.c:21:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/players_pl.c:36:23: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/players_pl.c:51:23: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/players_pl.h:48:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/players_pl.h:49:23: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/players_pl.h:50:23: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:1018:31: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:1041:23: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:1045:9: warning: unused variable 'x' [-Wunused-variable] |
| build, test | source/registers_rg.c:1077:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:1080:9: warning: unused variable 'x' [-Wunused-variable] |
| build, test | source/registers_rg.c:1210:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:158:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:174:19: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:193:19: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:257:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:273:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:289:18: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:305:18: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:321:18: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:339:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:372:28: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:390:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:43:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:44:31: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:45:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:46:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:461:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:465:6: warning: unused variable 'list_size' [-Wunused-variable] |
| build, test | source/registers_rg.c:47:13: warning: unused function 'display_arrow_registers' [-Wunused-function] |
| build, test | source/registers_rg.c:47:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:475:6: warning: unused variable 'ofs' [-Wunused-variable] |
| build, test | source/registers_rg.c:476:6: warning: unused variable 'bet_reg' [-Wunused-variable] |
| build, test | source/registers_rg.c:499:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:516:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:537:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:560:43: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:577:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:594:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:615:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.c:702:2: warning: variable 'op' is used uninitialized whenever 'if' condition is true [-Wsometimes-uninitialized] |
| build, test | source/registers_rg.c:706:6: warning: unused variable 'register_text_w' [-Wunused-variable] |
| build, test | source/registers_rg.c:711:2: warning: variable 'op' is used uninitialized whenever 'if' condition is true [-Wsometimes-uninitialized] |
| build, test | source/registers_rg.c:757:6: warning: unused variable 'register_text_w' [-Wunused-variable] |
| build, test | source/registers_rg.c:900:13: warning: unused function 'draw_register_value_highlight' [-Wunused-function] |
| build, test | source/registers_rg.c:993:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:19:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:33:23: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:34:43: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:35:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:41:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:42:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:43:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:44:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:45:18: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:46:18: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:47:18: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:48:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:49:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:52:19: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:53:19: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:54:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:55:28: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:56:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:57:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/registers_rg.h:64:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/sdl_config.c:122:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/sdl_config.c:147:15: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/sdl_config.c:272:15: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/sdl_config.h:20:15: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/sdl_config.h:21:15: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/sdl_config.h:23:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:100:17: warning: use of bitwise '\|' with boolean operands [-Wbitwise-instead-of-logical] |
| build, test | source/stage_buttons_sb.c:114:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:130:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:153:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:176:38: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:24:15: warning: unused variable 'STAGE_BUTTON_Y' [-Wunused-variable] |
| build, test | source/stage_buttons_sb.c:266:13: warning: unused function 'sb_check_released_in_stage_button' [-Wunused-function] |
| build, test | source/stage_buttons_sb.c:266:46: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:305:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:400:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:415:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:435:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:457:24: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:479:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:495:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:513:24: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:53:38: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:54:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:55:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:56:17: warning: unused function 'get_rst_b1_box' [-Wunused-function] |
| build, test | source/stage_buttons_sb.c:56:31: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:560:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:57:17: warning: unused function 'get_rst_b2_box' [-Wunused-function] |
| build, test | source/stage_buttons_sb.c:57:31: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:582:28: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:607:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.c:98:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.h:25:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.h:26:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.h:28:24: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.h:30:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.h:37:28: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.h:38:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.h:39:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.h:40:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.h:48:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.h:49:21: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.h:51:24: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.h:53:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stage_buttons_sb.h:54:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| test | source/stages.c:197:25: warning: unused parameter 'level' [-Wunused-parameter] |
| test | source/stages.c:197:36: warning: unused parameter 'operation_id' [-Wunused-parameter] |
| test | source/stages.c:202:8: warning: unused variable 'animation_limit' [-Wunused-variable] |
| build, test | source/stages.c:227:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stages.c:356:44: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stages.c:360:7: warning: unused variable 'label_sel' [-Wunused-variable] |
| build, test | source/stages.c:43:44: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stages.c:48:38: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stages.c:50:21: warning: unused function 'set_code_editable' [-Wunused-function] |
| build, test | source/stages.c:50:38: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stages.c:51:21: warning: unused function 'reset_code_editable' [-Wunused-function] |
| build, test | source/stages.c:51:40: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/stages.c:539:29: warning: unused parameter 'level_id' [-Wunused-parameter] |
| build, test | source/stages.c:575:15: warning: comparison of integers of different signs: 'int' and 'const Uint32' (aka 'const unsigned int') [-Wsign-compare] |
| build, test | source/stages.c:579:15: warning: comparison of integers of different signs: 'int' and 'const Uint32' (aka 'const unsigned int') [-Wsign-compare] |
| build, test | source/stages.c:583:15: warning: comparison of integers of different signs: 'int' and 'const Uint32' (aka 'const unsigned int') [-Wsign-compare] |
| build, test | source/stages.c:587:15: warning: comparison of integers of different signs: 'int' and 'const Uint32' (aka 'const unsigned int') [-Wsign-compare] |
| test | source/stages.c:607:24: warning: unused variable 'W' [-Wunused-variable] |
| test | source/stages.c:608:24: warning: unused variable 'H' [-Wunused-variable] |
| test | source/stages.c:609:24: warning: variable 'fx' set but not used [-Wunused-but-set-variable] |
| build, test | source/stages.c:610:24: warning: variable 'last_type_ms' set but not used [-Wunused-but-set-variable] |
| test | source/stages.c:621:8: warning: unused variable 'dt' [-Wunused-variable] |
| test | source/stages.c:629:23: warning: variable 'hold_line' set but not used [-Wunused-but-set-variable] |
| build, test | source/stages.c:87:30: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/storage/save_sv.h:16:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:107:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:119:28: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:139:41: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:161:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:182:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:205:42: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:229:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:253:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:275:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:297:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:320:28: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:494:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:526:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:550:12: warning: unused function 'get_box_member' [-Wunused-function] |
| build, test | source/text_tx.c:59:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:60:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:61:31: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:62:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:63:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:64:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:65:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:66:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:67:41: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.c:68:42: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.h:66:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.h:69:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/text_tx.h:76:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/button_bt.c:103:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/button_bt.c:118:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/button_bt.c:133:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/button_bt.c:148:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/button_bt.c:22:13: warning: unused variable 'g_btns_blocked' [-Wunused-variable] |
| build, test | source/ui/button_bt.c:286:8: warning: unused variable 'scale_w' [-Wunused-variable] |
| build, test | source/ui/button_bt.c:287:8: warning: unused variable 'scale_h' [-Wunused-variable] |
| build, test | source/ui/button_bt.c:37:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/button_bt.c:38:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/button_bt.c:39:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/button_bt.c:40:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/button_bt.c:469:12: warning: unused variable 'r' [-Wunused-variable] |
| build, test | source/ui/button_bt.c:54:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/button_bt.c:70:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/button_bt.c:88:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/button_bt.h:59:26: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/button_bt.h:60:20: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/button_bt.h:62:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/escape_menu_em.c:101:11: warning: unused variable 'mb' [-Wunused-variable] |
| build, test | source/ui/escape_menu_em.c:112:43: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/escape_menu_em.c:144:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/escape_menu_em.c:166:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/escape_menu_em.c:187:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/escape_menu_em.c:36:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/escape_menu_em.c:37:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/escape_menu_em.c:38:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/escape_menu_em.c:39:43: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/escape_menu_em.c:54:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/escape_menu_em.c:76:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/escape_menu_em.c:78:11: warning: unused variable 'mb' [-Wunused-variable] |
| build, test | source/ui/escape_menu_em.c:99:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/escape_menu_em.h:16:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/escape_menu_em.h:17:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/escape_menu_em.h:19:25: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/reset_menu_rm.c:1:1: warning: ISO C requires a translation unit to contain at least one declaration [-Wempty-translation-unit] |
| build, test | source/ui/reset_menu_rm.h:12:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/reset_menu_rm.h:9:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/rest_menu_rm.c:102:31: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/rest_menu_rm.c:127:31: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/rest_menu_rm.c:153:22: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/rest_menu_rm.c:33:31: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/rest_menu_rm.c:34:31: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/rest_menu_rm.c:46:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_level_sl.c:129:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_level_sl.c:141:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_level_sl.c:156:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_level_sl.c:159:6: warning: unused variable 'screen_height' [-Wunused-variable] |
| build, test | source/ui/screens/select_level_sl.c:182:6: warning: unused variable 'screen_width' [-Wunused-variable] |
| build, test | source/ui/screens/select_level_sl.c:183:6: warning: unused variable 'screen_height' [-Wunused-variable] |
| build, test | source/ui/screens/select_level_sl.c:204:45: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_level_sl.c:207:6: warning: unused variable 'screen_height' [-Wunused-variable] |
| build, test | source/ui/screens/select_level_sl.c:230:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_level_sl.c:246:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_level_sl.c:249:6: warning: unused variable 'screen_height' [-Wunused-variable] |
| build, test | source/ui/screens/select_level_sl.c:297:13: warning: variable 'finish' is used uninitialized whenever 'if' condition is false [-Wsometimes-uninitialized] |
| build, test | source/ui/screens/select_level_sl.c:297:13: warning: variable 'start' is used uninitialized whenever 'if' condition is false [-Wsometimes-uninitialized] |
| build, test | source/ui/screens/select_level_sl.c:332:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_level_sl.c:342:16: warning: variable 'anim_prev_ms' set but not used [-Wunused-but-set-variable] |
| build, test | source/ui/screens/select_level_sl.c:36:13: warning: unused function 'create_select_level_buttons' [-Wunused-function] |
| build, test | source/ui/screens/select_level_sl.c:37:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_level_sl.c:38:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_level_sl.c:40:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_level_sl.c:429:21: warning: comparison of integers of different signs: 'int' and 'const Uint32' (aka 'const unsigned int') [-Wsign-compare] |
| build, test | source/ui/screens/select_level_sl.c:43:45: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_level_sl.c:44:32: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_level_sl.c:45:38: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_level_sl.c:457:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_level_sl.c:467:16: warning: variable 'anim_prev_ms' set but not used [-Wunused-but-set-variable] |
| build, test | source/ui/screens/select_level_sl.c:81:28: warning: comparison of integers of different signs: 'int' and 'const Uint32' (aka 'const unsigned int') [-Wsign-compare] |
| build | source/ui/screens/select_level_sl.h:8:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build | source/ui/screens/select_level_sl.h:9:29: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_player_sp.c:257:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_player_sp.c:279:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_player_sp.c:300:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_player_sp.c:322:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_player_sp.c:358:7: warning: unused variable 'title_done' [-Wunused-variable] |
| build, test | source/ui/screens/select_player_sp.c:36:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_player_sp.c:37:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_player_sp.c:38:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_player_sp.c:42:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_player_sp.c:421:4: warning: unused label 'error' [-Wunused-label] |
| build, test | source/ui/screens/select_player_sp.c:54:33: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_player_sp.c:79:14: warning: unused variable 'chime_played' [-Wunused-variable] |
| build | source/ui/screens/select_player_sp.h:4:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_sector_ss.c:114:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_sector_ss.c:117:6: warning: unused variable 'screen_height' [-Wunused-variable] |
| build, test | source/ui/screens/select_sector_ss.c:139:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_sector_ss.c:18:21: warning: unused variable 'TYPE_DELAY_MS' [-Wunused-const-variable] |
| build, test | source/ui/screens/select_sector_ss.c:215:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_sector_ss.c:239:7: warning: unused variable 'title_done' [-Wunused-variable] |
| build, test | source/ui/screens/select_sector_ss.c:33:34: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_sector_ss.c:34:35: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_sector_ss.c:35:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_sector_ss.c:36:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_sector_ss.c:37:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_sector_ss.c:50:37: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_sector_ss.c:52:6: warning: unused variable 'screen_width' [-Wunused-variable] |
| build, test | source/ui/screens/select_sector_ss.c:53:6: warning: unused variable 'screen_height' [-Wunused-variable] |
| build, test | source/ui/screens/select_sector_ss.c:75:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_sector_ss.c:90:36: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/select_sector_ss.c:93:6: warning: unused variable 'screen_height' [-Wunused-variable] |
| build | source/ui/screens/select_sector_ss.h:5:27: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/title_screen_ts.c:227:14: warning: unused variable 'sfx_played' [-Wunused-variable] |
| build, test | source/ui/screens/title_screen_ts.c:229:12: warning: comparison of integers of different signs: 'int' and 'const Uint32' (aka 'const unsigned int') [-Wsign-compare] |
| build, test | source/ui/screens/title_screen_ts.c:233:19: warning: comparison of integers of different signs: 'int' and 'const Uint32' (aka 'const unsigned int') [-Wsign-compare] |
| build, test | source/ui/screens/title_screen_ts.c:242:12: warning: comparison of integers of different signs: 'int' and 'const Uint32' (aka 'const unsigned int') [-Wsign-compare] |
| build, test | source/ui/screens/title_screen_ts.c:37:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| build, test | source/ui/screens/title_screen_ts.c:52:39: warning: a function declaration without a prototype is deprecated in all versions of C [-Wstrict-prototypes] |
| test | tests/highlight_animation_sync_test.c:594:6: warning: unused variable 'op1_offset' [-Wunused-variable] |
| test | tests/highlight_animation_sync_test.c:595:6: warning: unused variable 'op2_offset' [-Wunused-variable] |
| test | tests/io_rail_geometry_test.c:78:28: warning: comparison of integers of different signs: 'int' and 'unsigned long' [-Wsign-compare] |
| test | tests/run_result_render_order_test.c:131:27: warning: comparison of integers of different signs: 'enum draw_event' and 'int' [-Wsign-compare] |
| test | tests/tutorial_arrow_animation_test.c:642:44: warning: comparison of integers of different signs: 'const highlight_target_t' (aka 'const enum highlight_target_t') and 'int' [-Wsign-compare] |