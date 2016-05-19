// __!!rengine_copyright!!__ //

#ifndef __RENGINE_HEIGHTMAP_H__
#define __RENGINE_HEIGHTMAP_H__

#include <rengine/geometry/Mesh.h>

namespace rengine
{
    class Heightmap : public Mesh
    {
        public:
			Heightmap();
			~Heightmap() {}

			//
			// Width -> X, Height -> Y, Depth -> Z
			//
			void setWidth(Real const value) { width_ = value; }
			void setHeight(Real const value) { height_ = value; }
			void setDepth(Real const value) { depth_ = value; }

			Real width() const { return width_; }
			Real height() const { return height_; }
			Real depth() const { return depth_; }

			Uint xPoints() const { return x_vertex; }
			Uint zPoints() const { return z_vertex; }

			Real xInterval() const { return (Real(xPoints()) / x_vertex); }
			Real zInterval() const { return (Real(zPoints()) / z_vertex); }

			// function to build the mesh from a image
			Bool load(std::string const& filename);

			//
			// Memory Friend Normal algorithm
			//
			void computeSmoothNormals();
        private:
			Vector3D computeFlatNormal(IndexType const x0, IndexType const z0, IndexType const x1, IndexType const z1, IndexType const x2, IndexType const z2);

			Real width_;
			Real height_;
			Real depth_;

			Uint x_vertex;
			Uint z_vertex;
    };

} // end of namespace

#endif // __RENGINE_HEIGHTMAP_H__
