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
#include <functional>

#include <tiny/math/vec.h>
#include <tiny/draw/staticmesh.h>

#include "bundle.hpp"
#include "stitch.hpp"
#include "texture.hpp"

namespace strata
{
	namespace mesh
	{
		class Layer;
		class Stitch;

		/** A Stitch is a class for long but narrow meshes that form the edge of a layer. Every layer can have one or several
		  * stitches which connect it to layers under it. Large layers may be cut and then reconnected via stitches in order
		  * to reduce layer size when desirable.
		  * Stitches are more general and less coherent than layers since they define polygons that incorporate vertices
		  * belonging to adjacent objects (layers and other stitches). */
		class Stitch
		{
			private:
				Strip strip;
			public:
				Stitch(long unsigned int id, tiny::algo::TypeCluster<long unsigned int, Strip> &tc, core::intf::RenderInterface * _renderer) :
					strip(id, tc, _renderer)
				{
				}
		};

		/** A Layer is a single, more or less smooth mesh that represents the top of a single soil layer.
		  * It is visible thanks to it owning a renderMesh object. It uses a Bundle to define its mesh
		  * where necessary. However, the Layer can also inherit all points from the layer under it, in the
		  * process becoming a subordinate layer to it. In this case it defines only its edge and thickness.
		  *
		  * The mesh always describes the upper part of the Layer, such that the surface is formed through
		  * clockwise-ordered polygons. Note that this is somewhat contrary to typical computer graphics which
		  * has upward normals if polygons are traversed counterclockwise.
		  */
		class Layer
		{
			private:
				std::vector<long unsigned int> bundles; /** The bundles forming this Layer. */
				double thickness;
			public:
				Layer(void)
				{
				}

				/** Initialize the MeshFragment as a flat, square layer. */
				void createFlatLayer(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip, float size, unsigned int ndivs, float height = 0.0f)
				{
					Bundle * bundle = makeNewBundle();
					bundles.push_back( bundle->getKey() );
					bundle->createFlatLayer(size, ndivs, height);

					bundle->initMesh(bundle);
				}

//				xVert addVertex(tiny::vec3 pos) { return mesh.addVertex(pos); }
//				tiny::vec3 getVertexPosition(xVert v) { return mesh.getVertexPosition(v); }
		};
	}
}
