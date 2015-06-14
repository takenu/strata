/*
This file is part of Chathran Strata: https://github.com/takenu/strata
Copyright 2015, Matthijs van Dorp.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <set>

#include <tiny/math/vec.h>
#include <tiny/draw/staticmesh.h>
#include <tiny/algo/typecluster.h>

#include "../core/interface/render.hpp"

#include "bundle.hpp"
#include "stitch.hpp"
#include "texture.hpp"

namespace strata
{
	namespace mesh
	{
		/** A MeshFragment is the base class for all objects that are to be represented by a mesh (i.e.
		  * an object consisting of a set of polygons). In other words, the terrain is defined through the
		  * set of all MeshFragments.
		  * In order to bundle these fragments together, they are combined in a TypeCluster, which enables
		  * memory management through deletion of MeshFragment pointers. */
		class MeshFragment : public tiny::algo::TypeClusterObject<long unsigned int, MeshFragment>
		{
			private:
				/** A list of fragments adjacent to the current one. If a vertex is deleted, all adjacent mesh fragments are notified such that
				  * they can update their mapping. */
				std::vector<MeshFragment*> adjacentFragments;
			protected:
				core::intf::RenderInterface * renderer;
				tiny::draw::StaticMesh * renderMesh;
				tiny::draw::RGBTexture2D * texture;

				void initMesh(MeshBundle * mesh)
				{
					renderMesh = new tiny::draw::StaticMesh( mesh->convertToMesh() );
					renderMesh->setDiffuseTexture(*texture);
					renderer->addWorldRenderable(renderMesh);
				}

				/** Purge a vertex, cleaning it from all references. After this operation, the derived class should consider the vertex with
				  * index "oldVert" belonging to the mesh fragment with id "mfid" as no longer existing. The vertex with index "newVert" is the
				  * suggested replacement of the old vertex. */
				virtual void purgeVertex(long unsigned int mfid, xVert oldVert, xVert newVert) = 0;
			public:
				MeshFragment(long unsigned int id, tiny::algo::TypeCluster<long unsigned int, MeshFragment> &tc, core::intf::RenderInterface * _renderer) :
					tiny::algo::TypeClusterObject<long unsigned int, MeshFragment>(id, this, tc),
					renderer(_renderer),
					renderMesh(0),
					texture(createTestTexture(512,255,0,0))
				{
				}

				void resetTexture(MeshBundle * mesh, unsigned int _size, unsigned char _r, unsigned char _g, unsigned char _b)
				{
					delete texture;
					texture = createTestTexture(_size, _r, _g, _b);
					renderer->freeRenderable(renderMesh);
					initMesh(mesh);
				}

				virtual ~MeshFragment(void) {}

				/** Split the MeshFragment into two parts. This operation should always reduce the size() of the MeshFragment. */
				virtual void split(void) = 0;

				/** Determine the size of the fragment, defined as the maximal end-to-end distance between two edge vertices. */
				virtual float meshSize(void) = 0;
		};

		/** A Stitch is a class for long but narrow meshes that form the edge of a layer. Every layer can have one or several
		  * stitches which connect it to layers under it. Large layers may be cut and then reconnected via stitches in order
		  * to reduce layer size when desirable.
		  * Stitches are more general and less coherent than layers since they define polygons that incorporate vertices
		  * belonging to adjacent objects (layers and other stitches). */
		class Stitch : public MeshFragment
		{
			private:
				MeshStitch stitch;

				virtual void purgeVertex(long unsigned int /*mfid*/, xVert /*oldVert*/, xVert /*newVert*/)
				{
				}
			public:
				Stitch(long unsigned int id, tiny::algo::TypeCluster<long unsigned int, MeshFragment> &tc, core::intf::RenderInterface * _renderer) :
					MeshFragment(id, tc, _renderer)
				{
				}

				virtual void split (void) {}

				virtual float meshSize(void)
				{
					return stitch.size();
				}
		};

		/** A Layer is a single, more or less smooth mesh that represents the top of a single soil layer.
		  * It is visible thanks to it owning a renderMesh object. It uses a MeshBundle to define its mesh
		  * where necessary. However, the Layer can also inherit all points from the layer under it, in the
		  * process becoming a subordinate layer to it. In this case it defines only its edge and thickness.
		  *
		  * The mesh always describes the upper part of the Layer, such that the surface is formed through
		  * clockwise-ordered polygons. Note that this is somewhat contrary to typical computer graphics which
		  * has upward normals if polygons are traversed counterclockwise.
		  */
		class Layer : public MeshFragment
		{
			private:
				MeshBundle mesh;
				double thickness;

				virtual void purgeVertex(long unsigned int , xVert , xVert ) {}
			public:
				Layer(long unsigned int id, tiny::algo::TypeCluster<long unsigned int, MeshFragment> &tc, core::intf::RenderInterface * _renderer) :
					MeshFragment(id, tc, _renderer),
					mesh()
				{
				}

				/** Initialize the MeshFragment as a flat, square layer. */
				void createFlatLayer(float size, unsigned int ndivs, float height = 0.0f)
				{
					mesh.createFlatLayer(size, ndivs, height);

					initMesh(&mesh);
				}

				virtual void split (void)
				{
					VertPair farthestPair(0,0);

				}

				virtual float meshSize(void)
				{
					return mesh.size();
				}
		};
	}
}
