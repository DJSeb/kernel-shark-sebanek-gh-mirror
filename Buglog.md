These should be interchangeable with GitLab issues and their comments.

If any bug occurs, note it here and its environment + behaviour.
If a bug has been solved, mark it and provide explanation (or a commit ID where it was solved).

# Open bugs

# Resolved/Ignored bugs

1. Switching between trace files results in a segmentation fault.

   - Status: CLOSED
   - Cause: Most likely a double free in clean_opened_views.
   - Solution: Changed semantics of detailed views - they may
     stay after the trace file has been changed, but they will
     be destroyed when closed by the user or when KernelShark's
     main window closes (as its parent, it will close it
     automatically)
   - Environment: WSL-openSUSE-Tumbleweed

2. Switching between trace files results in Stacklook's triangle buttons
   losing the ability to assign color to trace files.

   - Status: CLOSED
   - Cause: Most likely staticness of a variable and lack of change after trace file switch.
   - Solution: Function that restarts color table after tracefile load.
   - Environment: WSL-openSUSE-Tumbleweed

3. Adding the plugin again will have the program do a double free on exit.

   - Status: CLOSED
   - Cause: No protection from double plugin deload
   - Solution: Introduce a guard check before attempting to deload the plugin.
   - Environment: WSL-openSUSE-Tumbleweed

4. Triangle buttons are drawn in the opposite order they're accessible, i.e.
   clicking on/hovering over a button will select the button drawn UNDER the one
   clicked at.

   - Status: IGNORED
   - Cause: Incorrect rendering ordering
   - Possible solution(s):
     1. Change how KernelShark is rendering plot objects
     - Rejected: way too much work for this scope, but probably the only real solution.
     - Attempted solution to reverse the drawn forwards list each time introduced more issues than wanted.
     2. Change how plot objects are being added (in plugin or in kshark)
     - Rejected: goes against the API
     3. Change what the cursor is hovering over to objects added first (if possible)
     - Rejected: Same reason as point 1
   - Environment: WSL-openSUSE-Tumbleweed

5. Loading user plugins via a session while they are already loaded upon KernelShark launch will result
   in a double free on program exit.

   - Status: CLOSED
   - Cause: Plugins will attempt to close an already closed stream, visible through an invalid -1 stream id.
   - Solution: Introduced a guard check against closing a stream with an invalid ID, therefore a stream already closed.
   - Environment: WSL-openSUSE-Tumbleweed

6. Loading a session with a user plugin, which uses newly introduced `getPidColors` member function of the
   `KsGLWidget` class, will result in a segmentation fault and an immediate crash of the program.

   - Status: CLOSED (somewhat)
   - Cause: Unknown (object seems to already exist, but calling on its `_pidColors` field will return an unreadable
     address). Problem happens only in this context, if the plugin was already loaded, this issue doesn't appear.
   - Solution: Any user plugin using this function must include behaviour that will guard against this - which will
     most likely be a default behaviour and an optional usage of the task color table (this was the approach chosen
     for Naps and Stacklook). Codewise, the issue must be somewhere in the `importSession` function's calls, but fixing
     this is beyond the scope of the project.
   - Environment: WSL-openSUSE-Tumbleweed

7. Opening KernelShark without preloading Stacklook in any way and then importing
   session where Stacklook was loaded will result in a segmentation fault upon button
   hover.

- Status: IGNORED
- Cause: Apparently KernelShark doesn't actually load the plugin, at least not fully.
  This becomes very obvious once no configuration menu is available and program crashes when it would usually
  work just fine.
- Solution: Preload Stacklook. Real solution would be fixing this maybe-bug in KernelShark code, but that is
  beyonf the scope of this project (like bug 6).
- Environment: WSL-openSUSE-Tumbleweed

8.  Events ftrace/kernel_stack and couplebreak/sched_waking\[target\] were drawing
    taskboxes unnecessarily.

    - Status: FIXED
    - Cause: There is no way to say which bins should have a taskbox between each
      other, so every event entry draws a taskbox, unless it's from the \<idle\> task
    - Solution: New visibility mask introduced in NOBOXES modification + NoBoxes plugin
      made to enable its functionality. Boxes starting at events with this bit unset will
      not be drawn, boxes passing through bins with that mask will skip them until they find
      a better bin to end at.
    - Environment: (any) Linux

