# DevTools study

This document records a focused study of the public `geode-sdk/DevTools` mod. It is an architectural reference for Okapy, not a plan to copy its implementation.

Upstream project: https://github.com/geode-sdk/DevTools  
License observed in the upstream repository: MIT. Any future reuse of upstream code must preserve its copyright and license notices. Okapy's implementation should remain independently designed.

## What DevTools does

DevTools is an in-game developer tool for Geode. Its main capabilities include:

- browsing the active Cocos node hierarchy;
- selecting a node and inspecting its attributes;
- editing supported built-in properties;
- allowing other mods to register custom inspectors for their own node types;
- rendering the developer UI with Dear ImGui.

## Node tree

The inspected `src/pages/Tree.cpp` shows that the tree starts from two roots:

- `CCDirector::get()->getRunningScene()`;
- `OverlayManager::get()`.

For each node, DevTools recursively visits `getChildren()`. It also has a special path for nodes stored in the `extra-children` user object, which it labels as fake children. The tree displays information such as the runtime class name, tag, node ID, and child count.

Search checks the class name and node ID and keeps ancestors visible when a descendant matches. Selection is tracked separately from tree expansion. The selected node is held using Geode's `Ref<CCNode>`, which is safer than retaining an unowned raw pointer when nodes are removed.

Important scope limit: this is not a scan of every object in the process. It traverses the scene and overlay roots that DevTools explicitly knows about, plus the extra-child collection it handles.

## Attribute inspector

The inspected `src/pages/Attributes.cpp` divides inspection into groups, including:

- basic node properties (position, scale, rotation, skew, anchor point, content size, z-order, tag, visibility);
- color and opacity when the node supports the relevant protocol;
- label text and label-specific settings;
- texture/sprite information;
- menu-item and layout-related attributes;
- custom properties supplied by mods.

Some controls write changes directly back to the live node. The inspected implementation also exposes destructive or structurally risky operations, such as deleting a node and dragging it to a different parent. These should not be copied into an initial Okapy inspector. A first version should be read-only, with editing introduced deliberately and with clear safeguards.

The basic inspector groups controls by capability rather than presenting one undifferentiated property list. For example, sprite-specific controls are shown only when the selected node can be treated as a `CCSprite`, and touch-priority controls are shown only when a matching touch handler exists. This is a useful pattern: inspect a capability only after checking that the selected object supports it.

The screenshot action captures a weak node reference and checks it again when the asynchronous save operation completes. That avoids blindly using a node pointer that may have become invalid while the file picker was open.

## Extension API

The upstream `include/API.hpp` and `src/API.cpp` define a registration mechanism for custom node inspectors.

At a high level:

1. A mod registers a callback for a Cocos node type.
2. When a compatible node is selected, DevTools invokes that callback while drawing its Attributes view.
3. The callback can add labels, editable properties, enum choices, buttons, and layout helpers.
4. The implementation uses typed event dispatch to connect the public API to the ImGui widgets.

The documented property types include arithmetic values, strings, Cocos colors, points, sizes, and rectangles. Type-specific callbacks provide the corresponding ImGui editor.

This is a useful design lesson for Okapy: custom inspection should be an extension point, not a collection of hard-coded checks for every third-party mod.

## UI and integration

The inspected `src/DevTools.cpp` uses Dear ImGui for its developer interface. It keeps the selected node separately, exposes a custom-inspector callback collection, and creates a draggable floating button as a child of Geode's `OverlayManager`. Settings are serialized through Geode's saved-value mechanism. The CMake project builds a shared Geode mod and links the Geode SDK and ImGui.

This confirms that DevTools is implemented as an in-game Geode mod, rather than as an Android launcher screen. Its use of Geode's overlay and saved-value services is specific to that environment; Okapy will need its own host-side equivalents. The source study does not establish the exact UI framework or rendering path Okapy should use.

## Okapy takeaways

The following are design lessons, not implemented features:

1. Keep node discovery, tree presentation, selection, and attribute inspection as separate responsibilities.
2. Start from explicitly supplied scene/overlay roots; do not claim to enumerate all process objects.
3. Traverse child relationships recursively and guard against stale node references.
4. Keep selection lifetime-safe when the underlying Cocos objects can be removed.
5. Make search operate on useful node metadata and preserve matching ancestors.
6. Provide an extension API so mods can contribute inspectors for their own node classes.
7. Begin with read-only inspection. Treat live mutation, deletion, and reparenting as later, separately reviewed capabilities.
8. Keep the inspector's Cocos integration behind a small adapter so the actual game/Cocos version can be accounted for.

## Current Okapy status

Okapy does not yet contain a Cocos node-tree inspector, an in-game DevTools-style UI, or a custom node-inspector API. The existing native Core is not connected to the Geometry Dash process or its Cocos runtime.

Implementation should wait until the supported Cocos/Geometry Dash types, scene roots, and integration approach are confirmed. This avoids inventing APIs or assuming that the game's Cocos version matches the upstream DevTools environment.
