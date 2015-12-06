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
#include "strip.hpp"
#include "texture.hpp"

namespace strata
{
	namespace mesh
	{
		class Layer;
		class Stitch;

		/** A Stitch is a class for long but narrow meshes that form the topological edge of a layer. Every layer can have one or several
		  * stitches which connect it to layers under it. Large layers may be cut and then reconnected via stitches in order
		  * to reduce layer size when desirable.
		  * Stitches are more general and less coherent than layers since they define polygons that incorporate vertices
		  * belonging to adjacent objects (layers and other stitches).
		  * Stitches are implemented as Strips (since they borrow vertices and do not contain any vertices themselves) but
		  * it is important to emphasize that Stitches are special Strips in that only they can connect one layer to another layer,
		  * and that they never used to connect several parts of a Layer.
		  */
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
			protected:
				std::vector<Bundle*> bundles; /** The bundles forming this Layer. */
//				double thickness; <-- Thickness may vary, do not define per layer
			public:
				Layer(void)
				{
				}

				/** Add a new Bundle to the Layer. The Bundle class calls this function upon creation of a new Bundle
				  * when it is splitting. The Terrain class calls this on the first Bundle to come into existence, or
				  * when the Bundle is a copy of an existing Bundle from a different layer.  */
				void addBundle(Bundle * bundle)
				{
					for(unsigned int i = 0; i < bundles.size(); i++)
						if(bundles[i] == bundle)
						{
							std::cout << " Layer::addBundle() : WARNING: Attempt to add a Bundle as part of a Layer more than once! "<<std::endl;
							return;
						}
					bundles.push_back(bundle);
				}

				/** Release a Bundle from the Layer. The Bundle class calls this in its destructor. */
				void releaseBundle(Bundle * bundle)
				{
					for(unsigned int i = 0; i < bundles.size(); i++)
						if(bundles[i] == bundle)
						{
							bundles[i] = bundles.back();
							bundles.pop_back();
							return;
						}
					std::cout << " Layer::releaseBundle() : WARNING: Cannot find Bundle to be released! "<<std::endl;
				}

				/** Increase the thickness of this Layer by the specified amount.
				  * This is done by moving every vertex a distance of 'thickness'
				  * along the direction of its normal, defined as the average of
				  * the normals of its adjacent polygons. */
				void increaseThickness(float thickness)
				{
				}

				Bundle * createBundle(std::function<Bundle * (void)> makeNewBundle)
				{
					Bundle * bundle = makeNewBundle();
					bundle->setParentLayer(this);
					bundles.push_back(bundle);
					return bundle;
				}
		};

		/** A MasterLayer is a special layer that underlies all other layers. It generates the primary deformation features of
		  * the terrain, such as the formation of mountains and valleys. It resembles the continental shelf, the thicker
		  * part of the Earth's crust that hosts the world's land masses.
		  * It is not supposed to be eroded away or even form part of the surface, and everything under it is to be considered
		  * a meaningless rocky mass. */
		class MasterLayer : public Layer
		{
			private:
			public:
				MasterLayer(void) : Layer()
				{
				}

				/** Add a single Bundle to the layer, and initialize it as a flat, roughly square mesh of equilateral triangles
				  * with size 'size' and 'ndivs' subdivisions. */
				void createFlatLayer(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> /*makeNewStrip*/, float size, unsigned int ndivs, float height = 0.0f)
				{
					Bundle * bundle = createBundle(makeNewBundle);
					bundle->createFlatLayer(size, ndivs, height);
					bundle->initMesh(); // uses mesh::Drawable::initMesh() which calls TopologicalMesh::convertToMesh()
					bundles.push_back(bundle);
				}
		};
	}
}
