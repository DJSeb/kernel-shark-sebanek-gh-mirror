# Navigation

Each of the documents below serve as technical and user documentations for each modification.

- *[Couplebreak](./couplebreak.md)*
- *[Get  Colors](./get-colors.md)*
- *[Mouse Hover Plot Objects](./mouse-hover-plot-objects.md)*
- *[No Boxes](./no-boxes.md)*
- *[NUMA Topology Views](./NUMA-topology-views.md)*
- *[Preview Labels Changeable](./preview-labels-changeable.md)*
- *[Record Kstack](./record-kstack.md)*

# Source code modifications navigation

Each modification has a source code tag in a comment, which navigates to parts of code relevant to that modification.
Each modification is enclosed in `//NOTE: Changed here. ([TAG]) ([DATE OF CHANGE])` starting comment and `// END of change`
closing comment, \[TAG\] being a modification tag and \[DATE OF CHANGE\] being a YYYY-MM-DD date of when a change was made.

# About author

David Jaromír Šebánek (e-mail: `djsebofficial@gmail.com`). Direct all inquiries about this plugin there.

# Acknowledgements and credits
- The motivation for these changes was given by **Jan Kára RNDr.**, who also serves as project's **supervisor**.
- **Authors of KernelShark** deserve a big kudos, as even though there were some hiccups, the program is impressively
  written and not too difficult to modify.
- **ChatGPT** & **Copilot**: while neither is good at helping with actual designs and logic, they do automate certain more
  boring tasks, like finishing a documentation sentence with the right meaning and grammar (sometimes) and picking out
  documentation pages for quick questions, e.g. what are restricted pointers in C. They also serve asa great listeners,
  when an idea needed to be said out loud, they were there to listen (and nod until the idea was proven futile).
- **Hwloc**, specifically its documentation and impressive tools to capture systems' topologies. The documentation was
  rich and full of useful knowledge, the tools already were prepared for all necessary operations needed by NUMA TV modification.