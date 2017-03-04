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

#include "../tools/texture.hpp"

#include "bundle.hpp"
#include "strip.hpp"

namespace strata
{
	namespace mesh
	{
		class Layer;

		/** A Layer is a single, more or less smooth mesh that represents the top of a single soil layer.
		  * It is visible thanks to it owning a renderMesh object. It uses a Bundle to define its mesh
		  * where necessary. However, the Layer can also inherit all points from the layer under it, in the
		  * process becoming a subordinate layer to it. In this case it defines only its edge and thickness.
		  *
		  * The mesh always describes the upper part of the Layer, such that the surface is formed through
		  * clockwise-ordered polygons. Note that this is somewhat contrary to typical computer graphics
		  * which has upward normals if polygons are traversed counterclockwise.
		  *
		  * The Layer defines textures for its constituent Bundles, Strips and Stitches, but these are
		  * not intended to be used as the actual surface of the layer. Instead, they serve to clearly
		  * show where the bundles, strips and stitches of every layer are. In order to look like a
		  * genuine terrain, a much more sophisticated texture would be required that is far outside of
		  * the scope of this class.
		  */
		class Layer
		{
			protected:
				std::vector<Bundle*> bundles; /** The bundles forming this Layer. */
				tiny::draw::RGBTexture2D * bundleTexture; /** Texture of the layer, used for Bundles. */
				tiny::draw::RGBTexture2D * stripTexture; /** Texture of the layer, used for Strips. */
				tiny::draw::RGBTexture2D * stitchTexture; /** Texture of the layer, used for Strips that are at the edge of the Layer. */
			public:
				Layer(void)
				{
				}

				virtual ~Layer(void)
				{
					if(bundleTexture) delete bundleTexture;
					if(stripTexture) delete stripTexture;
					if(stitchTexture) delete stitchTexture;
				}

				/** Add a new Bundle to the Layer. The Bundle class calls this function upon creation
				  * of a new Bundle when it is splitting. The Terrain class calls this on the first
				  * Bundle to come into existence, or when the Bundle is a copy of an existing Bundle
				  * from a different layer.  */
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
					for(unsigned int i = 0; i < bundles.size(); i++)
					{
						Bundle * b = bundles[i];
						std::vector<tiny::vec3> normals;
						for(unsigned int j = 0; j < b->numVertices(); j++)
							normals.push_back(b->getVertexNormal(j)*thickness);
						for(unsigned int j = 0; j < b->numVertices(); j++)
						{
							b->moveVertexAlongVector(j, normals[j]);
							b->addVertexWeight(j, b->calculateVertexSurface( b->getVertexIndex(j) ) );
						}
					}
				}

				Bundle * createBundle(std::function<Bundle * (void)> makeNewBundle)
				{
					Bundle * bundle = makeNewBundle();
					bundle->setParentLayer(this);
					bundles.push_back(bundle);
					return bundle;
				}

				void setBundleTexture(tiny::draw::RGBTexture2D * _texture)
				{
					bundleTexture = _texture;
				}

				void setStripTexture(tiny::draw::RGBTexture2D * _texture)
				{
					stripTexture = _texture;
				}

				void setStitchTexture(tiny::draw::RGBTexture2D * _texture)
				{
					stitchTexture = _texture;
				}

				tiny::draw::RGBTexture2D * getBundleTexture(void)
				{
					return bundleTexture;
				}

				tiny::draw::RGBTexture2D * getStripTexture(void)
				{
					return stripTexture;
				}

				tiny::draw::RGBTexture2D * getStitchTexture(void)
				{
					return stitchTexture;
				}
		};

		/** A MasterLayer is a special layer that underlies all other layers. It generates the primary
		  * deformation features of the terrain, such as the formation of mountains and valleys. It
		  * can be thought of as the lowest effective layer of the Terrain, such as the continental
		  * shelf, or preferably the Mohorovicic discontinuity.
		  * It is not supposed to be eroded away or even form part of the surface, and everything
		  * under it is to be considered a meaningless rocky mass. Its altitude is not necessarily
		  * fixed, and thus one can use it to simulate buoyancy and thereby generate mountain height
		  * in a physically realistic way, e.g. through force equilibrium at the mantle-crust boundary.
		  */
		class MasterLayer : public Layer
		{
			private:
			public:
				MasterLayer(void) : Layer()
				{
				}

				/** Add a single Bundle to the layer, and initialize it as a flat, roughly square mesh
				  * of equilateral triangles with size 'size' and 'ndivs' subdivisions. */
				void createFlatLayer(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> /*makeNewStrip*/, float size, unsigned int ndivs, float height = 0.0f)
				{
					Bundle * bundle = createBundle(makeNewBundle);
					bundle->createFlatLayer(size, ndivs, height);
					bundleTexture = tools::createTestTexture(64, 255, 200, 100);
					stripTexture = tools::createTestTexture(64, 200, 150, 100);
					stitchTexture = tools::createTestTexture(64, 100, 100, 200);
					bundle->resetTexture(bundleTexture);
					bundles.push_back(bundle);
				}
		};
	}
}
