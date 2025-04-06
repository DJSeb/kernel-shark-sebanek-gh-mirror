//NOTE: Changed here. (NUMA TV) (2025-04-04)

/* Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> */

/**
 *  @file    KsNUMATopologyViews.hpp
 *  @brief   Interface for working with NUMA topologies.
*/

#ifndef _KS_NUMA_TV_HPP
#define _KS_NUMA_TV_HPP

// C++
#include <vector>

// Qt
#include <QtWidgets>

/*
Machine:
  - packages (physical division) & NUMA nodes ("logical" divison)
    - cores
      - physical units (variable number, but usually not more than 2)

Separately, there can be GROUPS (of packages and nodes and cores and PUs and other groups).
They are kind of a fallback if nothing makes sense as a package or node.

So, uhm, I suppose I'll first try to visualise every CPU, then
I'll select those which are supposed to be hidden and then I'll
hide appropriately. (Maybe can skip the first step honestly, just
"re-hide" the whole list.)

The grouping (tree-like) will be layers - machine/packageORnode/core/PU
PUs are apparently the thing KernelShark shows as CPUs, since it cares not for
the NUMA stuff or HT.
*/

// Enum classes
/// @brief Differentiable view types, mainly used by the radio buttons per each stream.
enum class ViewType { DEFAULT = 0, TREE };

// Usings
/// @brief Simpler name to package a view type and chosen topology file.
using ViewTopologyPair = std::pair<ViewType, QString>;

// Classes
/*
class GroupItemAbstract {
public:
    virtual ~GroupItemAbstract() = default;
    virtual GroupItemType type() const = 0;
    virtual int id() const = 0;
};

enum class GroupItemType { // or change this to a class hierarchy as above
    GROUP_ITEM_PACKAGE,
    GROUP_ITEM_NODE,
    GROUP_ITEM_CORE,
    GROUP_ITEM_PU,
    GROUP_ITEM_GROUP
};

class GroupItem {
private:
    int _id;
    GroupItemType _type;
public:
    GroupItem(GroupItemType type, int id) : _type(type), _id(id) {}

    GroupItemType type() const { return _type; }
    int id() const { return _id; }
};
*/
class TopoCore {
    int _core_id;
    int _owner_node;
    int _owner_package;
    std::vector<int> _PUs;
    std::vector<int> _groups;
};

class NUMATopology {
    std::vector<int> _PUs; // Do I need this? eh....
    std::vector<TopoCore> _cores;
    std::vector<int> _packages; // ints are cpu ids
    std::vector<int> _nodes; // ints are cpu ids
    std::vector<void*> _groups; // this will be a bit more complicated, since groups cna group anything
};

#endif
// END of change