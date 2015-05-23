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

#include "element.hpp"
#include "texture.hpp"

namespace strata
{
	namespace mesh
	{
		/** A Stitch is a class for long but narrow meshes that form the edge of a layer. Every layer can have one or several
		  * stitches which connect it to layers under it. Large layers may be cut and then reconnected via stitches in order
		  * to reduce layer size when desirable.
		  * Stitches are more general and less coherent than layers since they define polygons that incorporate vertices
		  * belonging to adjacent objects (layers and other stitches). */
		class Stitch : public tiny::algo::TypeClusterObject<long unsigned int, Stitch>
		{
			private:
			public:
				Stitch(long unsigned int id, tiny::algo::TypeCluster<long unsigned int, Stitch> &tc, core::intf::RenderInterface * _renderer) :
					tiny::algo::TypeClusterObject<long unsigned int, Stitch>(id, this, tc)
				{
				}
		};

		/** A Layer is a single, more or less smooth mesh that represents the top of a single soil layer.
		  * It is visible thanks to it owning a renderMesh object. It uses a MeshBundle to define its mesh
		  * where necessary. However, the Layer can also inherit all points from the layer under it, in the
		  * process becoming a subordinate layer to it. In this case it defines only its edge and thickness. */
		class Layer : public tiny::algo::TypeClusterObject<long unsigned int, Layer>
		{
			private:
				MeshBundle mesh;

				core::intf::RenderInterface * renderer;
				tiny::draw::StaticMesh * renderMesh;
				tiny::draw::RGBTexture2D * texture;
				
				tiny::draw::StaticMesh * createFlatLayer(float size, unsigned int ndivs, float height = 0.0f)
				{
					mesh.createFlatLayer(size, ndivs, height);
					
					return new tiny::draw::StaticMesh( mesh.convertToMesh(size) );
//					return new tiny::draw::StaticMesh(tiny::mesh::StaticMesh::createCubeMesh(0.5f));
				}
			public:
				Layer(long unsigned int id, tiny::algo::TypeCluster<long unsigned int, Layer> &tc, core::intf::RenderInterface * _renderer,
						float size, unsigned int ndivs) :
					tiny::algo::TypeClusterObject<long unsigned int, Layer>(id, this, tc),
					mesh(),
					renderer(_renderer),
					renderMesh(0),
					texture(createTestTexture())
				{
					renderMesh = createFlatLayer(size, ndivs);
					renderMesh->setDiffuseTexture(*texture);
					renderer->addWorldRenderable(renderMesh);
				}
		};
	}
}
