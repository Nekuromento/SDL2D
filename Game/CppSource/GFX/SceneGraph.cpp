#include "SceneGraph.hpp"

#include <cassert>

void SceneGraph::initRootNode() NOEXCEPT {
    parents[0] = -1;
    localTransforms[0] = Matrix().identify();
    localBounds[0] = Rect();
    localColors[0] = Color();
    localColorOffsets[0] = ColorOffset();
}

void SceneGraph::updateGlobalTransforms() NOEXCEPT {
    for (size_t i = 1; i < nodeCount; ++i) {
        globalTransforms[i] = globalTransforms[parents[i]] * localTransforms[i];
    }
}

uint16_t SceneGraph::addNode(const uint16_t parent,
                             const Matrix& transform,
                             const Rect& bounds,
                             const Color& color,
                             const ColorOffset& offset) NOEXCEPT {
    assert(("Max node count reached", nodeCount != MaxNodeCount));
    assert(("Only root can be without a parent", parent != -1));
    assert(("Invalid parent", parent < nodeCount));

    parents[nodeCount] = parent;
    localTransforms[nodeCount] = transform;
    localBounds[nodeCount] = bounds;
    localColors[nodeCount] = color;
    localColorOffsets[nodeCount] = offset;

    return nodeCount++;
}

void SceneGraph::removeNode(const uint16_t node) NOEXCEPT {
    assert(("Unimplemented", false));
}
