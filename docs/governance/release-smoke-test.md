# Manual Release Smoke Test

Run on supported macOS with real display/audio and fixed assets. Use a disposable
copy of the game/resources, not the tracked player saves. Record revision, OS,
compiler/framework versions, fixture copies, and observations. Automated tests
do not replace this checklist.

- [ ] `make all`; start `./assemblyArchitect`, then separately test `--fullscreen`.
- [ ] Select an Architect/player through the real selection screen; verify default
      progress creation only in the disposable resource copy when saves are absent.
- [ ] Load copies of representative existing player_01/player_02/player_03 saves.
- [ ] Open representative introductory, loop, and conditional levels/tutorials;
      include Levels 00/01, 08/09, 13/14/15 and available Level 16 content.
- [ ] Add instructions from the Instruction Roster to the Code Box; move, clone,
      remove, replace operands, and cancel replacement. Verify permission limits.
- [ ] Create/retarget forward and backward JMP/JE/JNE destinations; reorder and
      remove around labels. Verify the destination remains intended or the defined
      missing-operand state appears, with no dangling highlight or crash.
- [ ] Scroll long programs, resize to 1280x720 and 1600x900 physical windows,
      switch fullscreen/windowed, and check canonical alignment and click mapping.
- [ ] Exercise STEP, RUN/FAST and STOP, reset confirmation, Return/escape menus,
      result Continue/retry/replay navigation as offered by the actual screen.
- [ ] Observe execution arrow, pending operands, tutorial arrows, highlights,
      registers, Zero Flag, consumed Input Buffer and evaluated Output Buffer.
- [ ] Complete and fail representative programs; verify results/unlocks occur
      once, reset/retry restores expected state, and tutorial sequence is unchanged.
- [ ] Save by leaving the level through supported navigation; reopen the same
      disposable player/level and verify code, jumps, incomplete operands, progress.
- [ ] Repeat enter/leave/reset cycles; verify audio, no crash, obvious corruption,
      stuck modal capture, misplaced glyphs, or stale highlights.

Any unclear semantic or player-visible change is escalated before release. Attach
before/after artifacts only when useful, without personal player data. Do not
mark steps passed merely because compilation or offscreen tests succeeded.
