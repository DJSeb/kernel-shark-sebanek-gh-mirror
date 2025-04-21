# Purpose
# Main design objectives
# Solution
Source code change tag: `NUMA TV`.

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

# Usage
# Terms
Brief topology
Topology Widget
Wrapper topology widget
Topology
Topology tree
Task padding
Node
Core
PU
View/view type
NUMA TV
NUMATV Context
NN
C
M
Logical index
OS index
# Bugs