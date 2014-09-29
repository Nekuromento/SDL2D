#ifndef SceneGraph_h__
#define SceneGraph_h__

#include <cstdint>

#include "Geom/Matrix2D.hpp"
#include "Geom/Rect.hpp"
#include "GFX/Color.hpp"
#include "Util/defines.hpp"
#include "Util/noncopyable.hpp"

struct SceneGraph : public util::Noncopyable {
    typedef Matrix2D<float> Matrix;

    Matrix* localTransforms;
    Matrix* globalTransforms;
    uint16_t* parents;
    size_t nodeCount;
    Color* localColors;
    Color* globalColors;

    static const size_t MaxNodeCount = 4096;
    static const size_t MatrixStorageSize = MaxNodeCount * sizeof(Matrix);
    static const size_t MatrixAlignment = std::alignment_of<Matrix>::value;
    static const size_t ParentStorageSize = MaxNodeCount * sizeof(uint16_t);
    static const size_t ParentAlignment = std::alignment_of<uint16_t>::value;
    static const size_t ColorStorageSize = MaxNodeCount * sizeof(Color);
    static const size_t ColorAlignment = std::alignment_of<Color>::value;

    template <typename Allocator>
    SceneGraph(Allocator& alloc) NOEXCEPT :
        localTransforms{ static_cast<Matrix*>(alloc.allocate(MatrixStorageSize, MatrixAlignment, 0)) },
        globalTransforms{ static_cast<Matrix*>(alloc.allocate(MatrixStorageSize, MatrixAlignment, 0)) },
        parents{ static_cast<uint16_t*>(alloc.allocate(ParentStorageSize, ParentAlignment, 0)) },
        nodeCount{ 1 },
        localColors{ static_cast<Color*>(alloc.allocate(ColorStorageSize, ColorAlignment, 0)) },
        globalColors{ static_cast<Color*>(alloc.allocate(ColorStorageSize, ColorAlignment, 0)) }
    {
        initRootNode();
    }

    uint16_t addNode(const uint16_t parent,
                     const Matrix& transform,
                     const Color& color) NOEXCEPT;
    void removeNode(const uint16_t node) NOEXCEPT;

    void updateGlobalTransforms() NOEXCEPT;
    void updateGlobalColors() NOEXCEPT;

private:
    void initRootNode() NOEXCEPT;
};

#endif // SceneGraph_h__
