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

#include <tiny/algo/typecluster.h>
#include <tiny/draw/staticmesh.h>

#include "../core/interface/render.hpp"

#include "layer.hpp"

namespace strata
{
	namespace mesh
	{
		typedef tiny::algo::TypeCluster<long unsigned int, Bundle> BundleTC;
		typedef tiny::algo::TypeCluster<long unsigned int, Strip> StripTC;

		/** The Terrain is the master class for an entire terrain object. It manages a set of Bundles, which are small
		  * mesh fragments, and Layers, which are stratigraphical components of the terrain. The Bundles are joined into
		  * Layers using Strip objects, which define the polygons required to join distinct meshes but which do not contain
		  * vertices of their own. Then, the Layers are glued on top of each other using Stitches. */
		class Terrain
		{
			private:
				MasterLayer * masterLayer;
				std::vector<Layer *> layers;
				core::intf::RenderInterface * renderer;

				long unsigned int bundleCounter;
				long unsigned int stripCounter;
				BundleTC bundles;
				StripTC strips;

				/** A function for adding a new Bundle to the Terrain. Most functions for modifying the Terrain are not
				  * implemented by the Terrain but inside by the object on which the modification is performed. Therefore,
				  * modifying functions are given access to this function by taking it as an argument, typically using std::bind. */
				Bundle * makeNewBundle(void);
				/** A function for adding a new Strip to the Terrain. Most functions for modifying the Terrain are not
				  * implemented by the Terrain but inside by the object on which the modification is performed. Therefore,
				  * modifying functions are given access to this function by taking it as an argument, typically using std::bind. */
				Strip * makeNewStrip(void);

				/** Split very large meshes (either Bundles or Strips) of this Terrain into smaller fragments. The criterium for splitting
				  * is exceedance of the maximal vertex-to-vertex distance of the mesh of a threshold size '_maxSize'. */
				template <typename MeshType>
				void splitLargeMeshes(tiny::algo::TypeCluster<long unsigned int, MeshType> &tc, float _maxSize = 400.0f)
				{
					std::vector<MeshType*> largeMeshes;
					for(typename std::map<long unsigned int, MeshType*>::iterator it = tc.begin(); it != tc.end(); it++)
					{
						if(it->second->meshSize() > _maxSize)
							largeMeshes.push_back(it->second);
					}
					for(unsigned int i = 0; i < largeMeshes.size(); i++)
					{
//						std::cout << " Terrain::splitLargeMeshes() : splitting mesh... "<<std::endl;
						if(largeMeshes[i]->split(std::bind(&Terrain::makeNewBundle, this), std::bind(&Terrain::makeNewStrip, this)))
							delete largeMeshes[i];
					}
				}

				/** Check the consistency and coherence of meshes. */
				template <typename MeshType>
				bool checkMeshConsistency(tiny::algo::TypeCluster<long unsigned int, MeshType> &tc)
				{
					bool meshesAreConsistent = true;
					for(typename std::map<long unsigned int, MeshType*>::iterator it = tc.begin(); it != tc.end(); it++)
					{
						meshesAreConsistent &= it->second->checkVertexIndices();
						meshesAreConsistent &= it->second->checkVertexPolyArrays();
						meshesAreConsistent &= it->second->checkPolyIndices();
						meshesAreConsistent &= it->second->checkAdjacentMeshes();
						meshesAreConsistent &= it->second->checkTopology();
					}
					if(!meshesAreConsistent) std::cout << " Terrain::checkMeshConsistency() : WARNING: Consistency checks on meshes FAILED; one or more meshes violate requirements! "<<std::endl;
					return meshesAreConsistent;
				}

				/** Duplicate an existing layer, resulting in a new layer at a given height above the old one.
				  * The positioning of the vertices of the new layer is using the normals from the old layer's vertices.
				  * The Bundle/Strip structure of the new layer will mirror the structure of the underlying layer. Note that
				  * this similarity is not strictly necessary but merely convenient, one should not be concerned about these
				  * properties continuing to be similar. In principle it is expected that the structure will diverge during
				  * terrain manipulation.
				  */
				void duplicateLayer(Layer * baseLayer, float thickness)
				{
					layers.push_back(new Layer());
					layers.back()->increaseThickness(thickness);
				}
			public:
				Terrain(core::intf::RenderInterface * _renderer) :
					masterLayer(0),
					renderer(_renderer),
					bundleCounter(0),
					stripCounter(0),
					bundles((long unsigned int)(-1), "BundleTC"),
					strips((long unsigned int)(-1), "StripTC")
				{
					masterLayer = new MasterLayer();
					masterLayer->createFlatLayer(std::bind(&Terrain::makeNewBundle, this), std::bind(&Terrain::makeNewStrip, this), 1000.0f, 100, 0.0f);
					for(unsigned int i = 0; i < 6; i++)
					{
						std::cout << " Terrain() : Splitting bundles... "<<std::endl;
						splitLargeMeshes(bundles);
						checkMeshConsistency(bundles);
						checkMeshConsistency(strips);
						std::cout << " Terrain() : Splitting strips... "<<std::endl;
						splitLargeMeshes(strips);
						checkMeshConsistency(bundles);
						checkMeshConsistency(strips);
					}
					duplicateLayer(masterLayer, 50.0f);
					for(unsigned int i = 0; i < 3; i++)
					{
						duplicateLayer(layers.back(), 20.0f);
					}
				}

				void update(void)
				{
				}

				~Terrain(void)
				{
				}
		};
	}
}
