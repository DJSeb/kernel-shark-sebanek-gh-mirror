# Purpose

Visualise topology of a stream, from which we are reading trace data to get more information where a
task ran or which CPU and therefore NUMA node was working more than others. Visualisation should
not battle with other parts of KernelShark's visualisations, at best rearrange some graphs, but
not their data.

# Terms
Below are terms introduced with this modification. Other KernelShark terms, e.g. stream, will not be
explained. Not all terms are 1:1 with hwloc's terms. They are explained to get an idea of them when
used in text.

*Block tree* - Blocks next to each other forming a tree. Best to show it with a visualisation:
```
|------|-----------|------|
|      |           | leaf |                         leaf
|      |           |------|                        /
|      | rootchild |                      rootchild
|      |           |------|              /         \
|      |           | leaf |             /           leaf
|      |-----------|------|            /
| root |                      =    root
|      |-----------|------|            \
|      |           | leaf |             \           leaf
|      |           |------|              \         /
|      | rootchild |                      rootchild
|      |           |------|                        \
|      |           | leaf |                         leaf
|------|-----------|------|
```
*Brief topology* - Topology with only the most necessary parts for a NUMA tree visualisation, that
being indices of NUMA nodes paired with their cores, which are represented by their logical indices
paired with their PUs, which are represented by their logical indices paired with their OS indices.
*C* - Label noting a core node in the topology tree. Usually followed by a number of its logical index.
*Core* - Structure containing one or more PUs in a hwloc topology. On systems with only one 1 PU per
core, e.g. where hyperthreading is disabled, they are interchangeable.
*GL widget* - OpenGL widget used by KernelShark to plot its CPU and task graphs.
*Logical index* - Index given to a part of the topology by hwloc during topology discovery. Together
with object type (i.e. core, NUMA node, PU), they form a unique identifier of a component in a given
topology.
*M* - Label noting a Machine node in the topology tree. This node is the root of the topology tree
for a given stream. This node is colored like the stream would be if multiple streams were present
in a given session.
*NN* - Label noting a NUMA Node node in the topology tree. Color of these nodes is an average of the
colors of its cores.
*Node* - Refers to either a structure containing cores in a topology (group, package or a NUMA node)
or a topology tree node, explained below.
*NUMA TV* - NUMA Topology Views, shortened.
*NUMA TV Context* - Configuration object for a KernelShark process, which manages per stream
configurations of which topologies are loaded, if any, and what view a stream is requesting to be
used during topology visualisation.
*OS index*
*Package* - Physical place where cores are installed (according to hwloc). Another type of a node in
a topology.
*PU* - Processing unit, in hwloc analogous to what KernelShark would call a CPU. Can be grouped in a core.
*Topology* - Structure detailing memory modules, CPUs and other devices on a machine. E.g. machine has
one package, which has four cores, each with just one PU. First two cores are also under the same NUMA
node and the other two are under another NUMA node. The machine's name is Greg and it has 512 GB of total
memory.
*Task padding* - Additional height of a topology widget used when KernelShark displays tasks for a stream.
*Topology tree* - Qt widgets and layouts displayed as a block tree, root being a machine tree node, root's
children are Node nodes and cores are tree's leaves. For a NUMA topology, Node nodes are NUMA node nodes.
*Topology tree node* - Node of a block tree visualising a brief topology.
*Topology visualisation* - Interchangeable with topology tree.
*Topology widget* - Qt widget with a topology tree and possibly task padding, shown in the wrapper topology
widget.
*Trace graph* - Qt widget housing GL widget, wrapper topology widget and controls for the GL widget, class
KsTraceGraph.
*View/view type* - Enumerated option of how to display topology of a stream. Default view is what KernelShark
used before this modification (and can be achieved still).
*Wrapper topology widget* - Shown to the left of KernelShark's GL widget (with the classic KernelShark
graphs), contains topology widgets in a vertical top-down layout.

# Design

## Main design objectives

- Keep existing KShark API
- Configuration-visualisation layers
- Don't change data, only visualisation
- Opt-in usage
- Synchronisation with GL widget's graphs
- Qt utilisation

## Design overview (more in *Solution*)

Modification can be broadly divided into two communicating components - configuration & visualisation.

Configuration component keeps topolgy and view data data for each stream (or lack thereof).
Visualisation component uses configuration's data and GL widget's data to show, change, delete or hide
topology information to relevant CPU graphs in the GL widget.

Each stream may own (0-1) configurations and (0-1) visualisation at a time.

Visualisation never changes data of configuration.

Configuration is the only component to use hwloc.

Both components are owned by the trace graph of KernelShark.

Configuration of a stream can be queried and potentially changed via public API, visualisations can
only be hidden or emptied out by public API.

## Questions and answers

Below are questions that may arise upon inspection of the modification.

### "Why are topology widgets stored as pointers?"

Qt disables copying of widgets, which proved troublesome during development.

But Qt handles pointers well, as long as every pointed to object has a parent,
which is the case for this modifcation. When a parent is destroyed, so are
its alive children.

### "Why not use more of hwloc's topology data?"

It is unnecessary to load a lot of data for the goal of this modification,
which is only concerned with grouping of PUs, cores and NUMA nodes.

### "Why are there no other views?"

Frankly, because the modification was on a tight schedule. It is possible to
make more views, with changes:

1. Subclass StreamNUMATopologyConfig as a child of some abstract class,
   from which more StreamXYZTopologyConfigs would arise. Add a new class
   for the view.
2. Add a new item into the ViewType enumeration class specifying a view.
3. Add a radio button to KsNUMATVDialog.
4. Subclass KsStreamTopology as a child of some abstract class, which would be
   held by KsTraceGraph. Add a new class for the view.

Not too much work, would be great for an extension of this modification.

### "Why aren't topology views more interactive?"

There were ideas about clicking on tree nodes and them hiding CPU graphs that belonged
to them. This would be rather easy to implement, but there is no reason to not do
that through KShark's existing GUI.

Adding more information to e.g. the tooltips would require more information from
hwloc to be passed, which was not necessary per specification.

### "Why divide the topology widgets from configuratons?"

To separate concerns. Widgets are only for displaying and can exist without a
configuration. Configurations change under different circumstances and exist to
inform topology widgets about how they should be drawn.

It would be possible to group them together in code as a pair under one
stream ID key, but the concerns would be less delimited. Price to pay
for the division is negligible.

### "Why use block trees?"

Block trees ensure that the tree node, which is a parent of another,
is always visible, whether you're scrolled all the way up or in the center or
all the way down. It makes hierarchy more explicit, improving information given
to the user.

It is also easier to create in Qt, which is a welcome bonus.

# Solution

## How configuration is solved

### Creating a topology configuration

### Updating a topology configuration

### Removal of topology configuration

### Lifetimes of topology configurations

## How visualisation is solved

### Creating a topology widget

<!--TODO: Integrate below. -->
```
Tree-like layout of the topology view with NUMA nodes.
It shall be constructed right to left, first matching CPUs
in KernelShark's GL window to PUs in the topology.
NUMA TV rearranges the CPUs, which allows construction of
such UI, that nodes and cores can be sorted by their logical
indices, as specified by hwloc. By creating the rightmost
column first, the core column can then create cores with the
height of the CPU(s) it owns - analogously for NUMA nodes.
By doing this, if some cores have more or less PUs, their
height will adjust accordingly. Similarly for NUMA nodes
and their cores. It also comes with a bonus reactivity to 
hidden & visible CPUs as specified by KernelShark - construction
right to left also allows to show only the relevant parts of the
topology.

Total height is then used for the machine column, which
also denotes the height of the stream's graph. It shall use
the stream's color (if there are more streams open).

Each tree node will have a tooltip to display less compact
information (useful if losing track of the label's text).

Caveats: Some exotic topologies won't work, e.g. nested NUMA nodes,
PUs shared across cores or cores shared across NUMA nodes.

Technically, it should be a visualisation of the NodeCorePU
mappings.

To take task graphs into account, spacing is added at the bottom of
the topology 

Example look:
__________________________________________________
|-----------------------------------------------|| KS GL graphs
||              |               |               ||  CPU 1
||              |               |    core L1    ||----------
||              |               |               ||  CPU 8
||              |   Nnode L1    |---------------||----------
||              |               |               ||  CPU 2
||              |               |    core L2    ||----------
||              |               |               ||  CPU 7
||   machine    |---------------|---------------||----------
||  (stream) X  |               |               ||  CPU 3
||              |               |    core L3    ||----------
||              |               |               ||  CPU 6
||              |   Nnode L2    |---------------||----------
||              |               |               ||  CPU 4
||              |               |    core L4    ||----------
||              |               |               ||  CPU 5
|------------------------------------------------|----------
|[                  SPACING                     ]|  taskXYZ
--------------------------------------------------
```

### Updating a topology widget

### Removal of topology widgets

### Lifetimes of topology widgets

### Positioning of topology widgets

Source code change tag: `NUMA TV`.

# Usage

## Stream confgurations

### Configuration menu

### Loading a file

### Clearing file selection

### Choosing a view type

### Applying or cancelling

## GUI

### Main window integration

### Scrolling in the GL widget with active topology widgets

### CPU graph rearrangements

### Hide button

### Tooltips on topology tree nodes

## Session support

NUMA Topology Views configuration can be saved in a session using new API of KsSession. Each stream has their view and
topology file path saved. A simple session import takes care of using such session ata.

## API

Any API that got introduced is either explicitly labeled with "numatv" ot "topology" or "topo"
somewhere (letters can be uppercase). If not, it belongs to a type with that label or a header
file with that label.

Use carefully.

# Bugs

None known to the author. But the modification was rather significant and there are always possibilites something
went wrong in secret.

# Trivia

- This modification was particularly difficult to write, as the D key on main developer's keyboard randomly stopped working
  and many re-reads had to be done. If any Ds are missing, let the main eveloper know.
- Main developer's quirk is that projects should have slightly quirky names - it helps marketability and breathes some life into them.
  NUMA topology views sounds boring, but it's abbreviation, NUMA TV, is a rather fitting one - just like a television, the modification
  displays information to the user via a screen. Rather weak, but a connection nonetheless, boosting development morale.