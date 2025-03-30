# Purpose

Allow code with access to the KernelShark graph to change the preview labels (figure 1).

![Figure 1](./images/preview-labels-changeable-1.png)

# Main design objectives

- Simplicity
- KernelShark code similarity
- Safe access

# Solution

Introduced public method `setPreviewLabels` to the `KsTraceGraph` class, which every stream has access to. This method
expects 5 QString arguments, but by default they are set to an empty string. Behaviour was copied from
how labels are set when hovering over an entry in the graph. The method just sets each of the five labels
the graph has to the given string.

Source code change tag: `PREVIEW LABELS CHANGEABLE`.

# Usage

When developing a plugin and having access to the graph (most likely via the public `graphPtr()` method on main window),
just ask the graph via the new method to update its preview labels. Example: Stacklook plugin uses this to display
a few of the kernel stack entries when mouse hovers over its button (mouse hover reactions of plot objects are another 
modification). See figure 2.

![Figure 2](./images/preview-labels-changeable-2.png)
Figure 2 - Stacklook button (highlighted by the red circle) asks preview labels to change upon mouse hover to some 
information

Make sure that any plugin using this modification is either preloaded via CLI or GUI before importing a session where such
plugin was active; or include defaults and an option to use functionality with this modification - failure to do so will
result in KernelShark crashing with a segmentation fault.

# Bugs

If a plugin using the new method is not loaded when KernelShark starts or before a session where it was active is loaded,
said session upon import will experience a segmentation fault when setting text to a label and the program will crash. 
This seems to be a quirk with KernelShark when plugins aren't properly loaded when importing a session or when plugins are 
built outside the `src/plugins` directory.
- One could argue this goes against the safe access goal, but as outlined above, the problem lies with the "unstable" way
  KernelShark loads plugins upon session import - every other way of access is problem-free.