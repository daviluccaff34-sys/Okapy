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

## Final findings: scene changes and refresh behavior

A second pass through `src/main.cpp`, `src/DevTools.cpp`, `src/DevTools.hpp`, and `src/pages/Tree.cpp` clarifies the lifecycle:

- `CCDirector::willSwitchToScene` is hooked. The original method is called first, then DevTools calls `sceneChanged()`.
- `sceneChanged()` currently clears `m_selectedNode`. It does not build or store a replacement tree.
- The Tree page calls `drawTree()` as part of the normal UI drawing path. Each draw obtains the current `getRunningScene()` and the current `OverlayManager`, then starts traversing those roots.
- Consequently, the tree is a live view of the current graph, not a cached list of known screens and not a precomputed snapshot. When the active scene changes, the next draw naturally traverses the new scene graph.
- The overlay is a separate root because some UI elements live outside the running scene.
- The tree recursively renders ordinary children when a branch is expanded. During search, `searchBranch()` recursively checks descendants so matching ancestors can remain visible.
- The node label is composed from the runtime object/class name, sibling index, tag (when present), node ID (when present), and child count. This explains why the UI can show many different object types without a manually maintained list of every screen.
- The selected node is stored as `Ref<CCNode>`; the dragged node is also a `Ref<CCNode>`. This is a lifetime-aware ownership/reference mechanism from the upstream environment, not something to reproduce by keeping arbitrary raw pointers.
- The expansion-state map uses raw `CCNode*` keys. The inspected `sceneChanged()` only clears selection; it does not visibly clear that map. Okapy should avoid carrying expansion state across scene lifetimes using raw addresses. Prefer scene-generation-scoped stable handles or clear per-scene UI state when a scene is replaced.
- DevTools' main rendering hook wraps or follows the game's draw path depending on its window mode. The scene tree itself is still queried by the page's draw function; it is not populated by a separate global scan.

### What the behavior means

The user's observation that entering Build changes the displayed objects is consistent with the active-scene model: the Build editor supplies a different active scene/root graph, so the next tree draw shows its nodes. The source confirms the mechanism, but does not prove that every possible Geometry Dash UI object is reachable from those two roots. Nodes detached from the running scene and overlay, or objects held only in unrelated managers, are outside this traversal unless explicitly added as roots.

### Final Okapy architecture decision

For Okapy, the correct conceptual design is:

1. A **scene source** obtains the current scene root and any separately supported overlay roots from the actual Geometry Dash/Cocos runtime.
2. A **node-tree traverser** walks child relationships from those roots and emits a read-only view/model for the inspector.
3. A **scene identity/generation** changes when the active scene is replaced. Selection and expansion state are scoped to that generation so stale nodes cannot remain selected.
4. A **node identity/lifetime adapter** protects references to live Cocos objects. A raw pointer may be used internally for a short synchronous traversal, but must not be treated as a durable identity across frames or scene changes.
5. A **tree UI** presents the current model, supports expansion and search, and keeps presentation state separate from the live game objects.
6. An **attribute inspector** reads common properties first and adds capability-specific sections only after runtime type/protocol checks.
7. A **custom inspector API** lets mods register read-only inspectors for their own node types. Mutating controls can be a later opt-in capability with permission checks and safety review.

A cached snapshot can be useful for UI performance, but it must be refreshed from the live graph and invalidated on scene changes. The upstream implementation demonstrates that a full persistent inventory of all screens is unnecessary.

### Boundaries and remaining verification

This study establishes the upstream DevTools design, not the exact ABI or Cocos version available in every Geometry Dash target. Before implementing the runtime adapter, Okapy must confirm its target game builds, Cocos class layouts/methods, hook points, and overlay roots. No concrete Cocos hook or scene API should be declared integrated until it is compiled and verified against the supported target.

The initial Okapy inspector should be read-only and should not include node deletion, reparenting, arbitrary memory editing, or assumptions that all process objects belong to the active scene graph.
