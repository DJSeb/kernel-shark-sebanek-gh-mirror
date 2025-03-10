<!-- I could use something more robust, but this will do. -->

These should be interchangeable with GitLab issues and their comments.

If any bug occurs, note it here and its environment + behaviour.
If a bug has been solved, mark it and provide explanation (or a commit ID where it was solved).

# Open bugs

1. Triangle buttons are drawn in the opposite order they're accessible, i.e.
   overlapping buttons don't visually represent the logical overlap order.

- Status: ON HOLD
- Cause: Incorrect rendering ordering
- Possible solution(s):
  1. Change how KernelShark is rendering plot objects
  2. Change how plot objects are being added (in plugin or in kshark)
  3. Change what the cursor is hovering over to objects added first (if possible)
  - Final solution should be applied after others are compared
- Environment: WSL-openSUSE-Tumbleweed

2. Adding the plugin again will have the program do a double free on exit.

- Status: OPEN
- Cause: No protection from double plugin load
- Possible solution: Introduce a guard check before attempting to load the plugin.
- Environment: WSL-openSUSE-Tumbleweed

# Closed bugs

1. Switching between trace files results in a segmentation fault

- Status: CLOSED
- Cause: Most likely a double free in clean_opened_views.
- Solution: Changed semantics of detailed views - they may
  stay after the trace file has been changed, but they will
  be destroyed when closed by the user or when KernelShark's
  main window closes (as its parent, it will close it
  automatically)
- Environment: WSL-openSUSE-Tumbleweed

2. Switching between trace files results in Stacklook's triangle buttons
   losing the ability to assign color to trace files

- Status: CLOSED
- Cause: Most likely staticness of a variable and lack of change
  after trace file switch.
- Solution: Function that restarts color table after tracefile load.
- Environment: WSL-openSUSE-Tumbleweed
