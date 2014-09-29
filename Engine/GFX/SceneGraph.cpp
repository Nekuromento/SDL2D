#include "SceneGraph.hpp"

#include <cassert>

void SceneGraph::initRootNode() NOEXCEPT {
    parents[0] = -1;
    localTransforms[0] = Matrix().identify();
    localColors[0] = Color();
}

void SceneGraph::updateGlobalTransforms() NOEXCEPT {
    for (size_t i = 1; i < nodeCount; ++i) {
        globalTransforms[i] = globalTransforms[parents[i]] * localTransforms[i];
    }
}

void SceneGraph::updateGlobalColors() NOEXCEPT {
    for (size_t i = 1; i < nodeCount; ++i) {
        globalColors[i] = globalColors[parents[i]] * localColors[i];
    }
}

uint16_t SceneGraph::addNode(const uint16_t parent,
                             const Matrix& transform,
                             const Color& color) NOEXCEPT {
    assert(("Max node count reached", nodeCount != MaxNodeCount));
    assert(("Only root can be without a parent", parent != static_cast<uint16_t>(-1)));
    assert(("Invalid parent", parent < nodeCount));

    parents[nodeCount] = parent;
    localTransforms[nodeCount] = transform;
    localColors[nodeCount] = color;

    return nodeCount++;
}

void SceneGraph::removeNode(const uint16_t node) NOEXCEPT {
    assert(("Unimplemented", false));
}
