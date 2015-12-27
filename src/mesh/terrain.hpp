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

		typedef std::map<long unsigned int, Bundle*>::iterator BundleIterator;
		typedef std::map<long unsigned int, Strip*>::iterator StripIterator;

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

//				tiny::draw::RGBTexture2D * texture;

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

				/** Calculate the number of bytes of memory used. */
				long unsigned int usedMemory(void)
				{
					long unsigned int nbytes = 0;
					for(std::map<long unsigned int, Bundle*>::const_iterator it = bundles.begin(); it != bundles.end(); it++)
						nbytes += it->second->usedMemory();
					for(std::map<long unsigned int, Strip*>::const_iterator it = strips.begin(); it != strips.end(); it++)
						nbytes += it->second->usedMemory();
					return nbytes;
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
					std::vector<Bundle *> baseBundles;
					std::vector<Strip *> baseStrips;
					// First collect bundles and strips of the base layer. Do not add Bundles and Strips yet - that would mess up the std::map.
					for(std::map<long unsigned int, Bundle*>::iterator it = bundles.begin(); it != bundles.end(); it++)
						if(it->second->getParentLayer() == baseLayer)
							baseBundles.push_back(it->second);
					for(std::map<long unsigned int, Strip*>::iterator it = strips.begin(); it != strips.end(); it++)
						if(it->second->getParentLayer() == baseLayer)
							baseStrips.push_back(it->second);
					// Now duplicate all bundles and strips of the base layer.
					std::map<Bundle*, Bundle*> bmap;
					std::map<Strip*, Strip*> smap;
					for(unsigned int i = 0; i < baseBundles.size(); i++)
					{
						Bundle * bundle = makeNewBundle();
						bundle->setParentLayer(layers.back());
						layers.back()->addBundle(bundle);
						baseBundles[i]->duplicateBundle(bundle);
						bmap.emplace(baseBundles[i],bundle);
					}
					for(unsigned int i = 0; i < baseStrips.size(); i++)
					{
						Strip * strip = makeNewStrip();
						strip->setParentLayer(layers.back());
						baseStrips[i]->duplicateStrip(strip);
						smap.emplace(baseStrips[i],strip);
					}
					// Update all cross references: adjust Strip owningBundle, and adjust adjacentBundles/adjacentStrips
					for(unsigned int i = 0; i < baseBundles.size(); i++)
						bmap.at(baseBundles[i])->duplicateAdjustAdjacentStrips(smap);
					for(unsigned int i = 0; i < baseStrips.size(); i++)
					{
						smap.at(baseStrips[i])->duplicateAdjustAdjacentBundles(bmap);
						smap.at(baseStrips[i])->duplicateAdjustOwningBundles(bmap);
					}
					// Copy all other attributes, and initialize meshes.
					for(std::map<Bundle*, Bundle*>::iterator it = bmap.begin(); it != bmap.end(); it++)
					{
						it->second->setScaleFactor(it->first->getScaleFactor());
						it->second->resetTexture(it->first->getTexture());
					}
					for(std::map<Strip*, Strip*>::iterator it = smap.begin(); it != smap.end(); it++)
					{
						it->second->setScaleFactor(it->first->getScaleFactor());
						it->second->resetTexture(it->first->getTexture());
					}
					// Move all vertices of the Mesh a fixed distance along the direction of their respective normals.
					layers.back()->increaseThickness(thickness);
					for(std::map<Strip*, Strip*>::iterator it = smap.begin(); it != smap.end(); it++)
						it->second->recalculateVertexPositions(); // Strip positions are not updated by the Layer and need to be re-set
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
					masterLayer->createFlatLayer(std::bind(&Terrain::makeNewBundle, this), std::bind(&Terrain::makeNewStrip, this), 1000.0f, 300, 0.0f);
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
					std::cout << " Terrain() : Duplicating layer... "<<std::endl;
					duplicateLayer(masterLayer, 50.0f);
					for(unsigned int i = 0; i < 20; i++)
					{
						std::cout << " Terrain() : Duplicating layer... "<<std::endl;
						duplicateLayer(layers.back(), 5.0f);
					}
				}

				/** Get the position of the terrain surface vertically below the 3D-position 'pos'.
				  * This procedure could be considerably more efficient if a decent degree of pre-organization
				  * was performed, such as using a spatial cluster structure on Bundles and Strips to find
				  * more quickly which Bundles and Strips are of interest.
				  */
				float getVerticalHeight(tiny::vec3 pos)
				{
					tiny::vec3 intsec(0.0f, pos.y-10000.0f, 0.0f);
					for(BundleIterator it = bundles.begin(); it != bundles.end(); it++)
					{
						tiny::vec3 center = it->second->findCentralPoint();
						if(calcHorizontalSeparation(pos, center) < it->second->maxVertexDistance(center)) // Check if intersection is possible
						{
							// Use TopologicalMesh::findIntersectionPoint to set intsec to a closer intersection point (if any).
							it->second->findIntersectionPoint(intsec, pos, tiny::vec3(0.0f,-1.0f,0.0f));
						}
					}
					for(StripIterator it = strips.begin(); it != strips.end(); it++)
					{
						tiny::vec3 center = it->second->findCentralPoint();
						if(calcHorizontalSeparation(pos, center) < it->second->maxVertexDistance(center)) // Check if intersection is possible
						{
							// Use TopologicalMesh::findIntersectionPoint to set intsec to a closer intersection point (if any).
							it->second->findIntersectionPoint(intsec, pos, tiny::vec3(0.0f,-1.0f,0.0f));
						}
					}
					return intsec.y;
				}

				void update(void)
				{
					std::cout << " Terrain::update() : Using "<<usedMemory()<<" bytes of memory... "<<std::endl;
				}

				~Terrain(void)
				{
				}
		};
	}
}
