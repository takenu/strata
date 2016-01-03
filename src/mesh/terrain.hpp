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

#include "../interface/render.hpp"
#include "../interface/ui.hpp"

#include "../tools/convertstring.hpp"

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
		class Terrain : public intf::UISource
		{
			private:
				MasterLayer * masterLayer;
				float maxMeshSize;
				std::vector<Layer *> layers;
				intf::RenderInterface * renderer;
				intf::UIInterface * uiInterface;

//				tiny::draw::RGBTexture2D * texture;

				long unsigned int bundleCounter;
				long unsigned int stripCounter;
				BundleTC bundles;
				StripTC strips;

				/** A function for adding a new Bundle to the Terrain. Most functions
				  * for modifying the Terrain are not implemented by the Terrain but
				  * inside by the object on which the modification is performed. Therefore,
				  * modifying functions are given access to this function by taking it
				  * as an argument, typically using std::bind. */
				Bundle * makeNewBundle(void);

				/** A function for adding a new Strip to the Terrain. Most functions
				  * for modifying the Terrain are not implemented by the Terrain but
				  * inside by the object on which the modification is performed. Therefore,
				  * modifying functions are given access to this function by taking it
				  * as an argument, typically using std::bind. */
				Strip * makeNewStrip(void);

				/** A function for adding a new Stitch-Strip to the Terrain. Stitch
				  * Strips are special Strip objects that define the edge of a
				  * Layer, connecting it to the Layer that lies under it. Since the
				  * Stitch Strip is a somewhat different object than a usual Strip,
				  * it is made impossible to switch on and off the Stitch-ness by a
				  * const bool 'isStitch' flag in the Strip. This requires the Stitch
				  * Strip to also have a creator function of its own kind, used for
				  * example when a Stitch mesh is split. */
				Strip * makeNewStitch(void);

				/** Split very large meshes (either Bundles or Strips) of this Terrain into smaller fragments. The criterium for splitting
				  * is exceedance of the maximal vertex-to-vertex distance of the mesh of a threshold size '_maxSize'. */
				template <typename MeshType>
				void splitLargeMeshes(tiny::algo::TypeCluster<long unsigned int, MeshType> &tc, float _maxSize)
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

				/** Calculate the number of bytes of memory used. */
				long unsigned int usedCapacity(void)
				{
					long unsigned int nbytes = 0;
					for(std::map<long unsigned int, Bundle*>::const_iterator it = bundles.begin(); it != bundles.end(); it++)
						nbytes += it->second->usedCapacity();
					for(std::map<long unsigned int, Strip*>::const_iterator it = strips.begin(); it != strips.end(); it++)
						nbytes += it->second->usedCapacity();
					return nbytes;
				}

				void duplicateLayer(Layer * baseLayer, float thickness);
			public:
				Terrain(intf::RenderInterface * _renderer, intf::UIInterface * _uiInterface) :
					intf::UISource("Terrain",_uiInterface),
					masterLayer(0),
					maxMeshSize(400.0f),
					renderer(_renderer),
					uiInterface(_uiInterface),
					bundleCounter(0),
					stripCounter(0),
					bundles((long unsigned int)(-1), "BundleTC"),
					strips((long unsigned int)(-1), "StripTC")
				{
				}

				void makeFlatLayer(float terrainSize, float _maxMeshSize,
						unsigned int meshSubdivisions, float height)
				{
					if(masterLayer)
					{
						std::cout << " Terrain::makeFlatLayer() : Terrain is not empty!";
						std::cout << " Skipping. "<<std::endl;
						return;
					}
					else
					{
						maxMeshSize = _maxMeshSize;
						masterLayer = new MasterLayer();
						masterLayer->createFlatLayer(
								std::bind(&Terrain::makeNewBundle, this),
								std::bind(&Terrain::makeNewStrip, this),
								terrainSize, meshSubdivisions, height);
						for(unsigned int i = 0; i < 10; i++)
						{
							std::cout << " Terrain::makeFlatLayer() : Splitting bundles... "<<std::endl;
							splitLargeMeshes(bundles, maxMeshSize);
							checkMeshConsistency(bundles);
							checkMeshConsistency(strips);
							std::cout << " Terrain::makeFlatLayer() : Splitting strips... "<<std::endl;
							splitLargeMeshes(strips, maxMeshSize);
							checkMeshConsistency(bundles);
							checkMeshConsistency(strips);
						}
					}
				}

				void addLayer(float thickness)
				{
					std::cout << " Terrain::addLayer() : Duplicating layer... "<<std::endl;
					duplicateLayer((layers.size() == 0 ? masterLayer : layers.back()), thickness);
				}

				/** Find the underlying Vertex (defined uniquely via its owning Bundle, 'bundle',
				  * plus the index of the Vertex in that bundle, 'index') for the Vertex at
				  * position 'v' in the Layer 'baseLayer'.
				  * This function should look across all layers for the most nearby vertex 'w'
				  * with the following requirements:
				  * - 'w' is not a part of baseLayer;
				  * - 'w' has an averaged normal whose inner product with the direction vector
				  *   (v-w) is positive (such that the vertex 'v' can be said to be 'above'
				  *   the underlying Vertex).
				  *
				  * Even with these requirements, there are situations in which the resulting
				  * Vertex would not truly belong to the underlying Layer, so this function
				  * should ideally be used on Layers that are flat enough (i.e. where the
				  * curvature is not strong enough to cause vertices of overlying layers
				  * to have a tangent plane that intersects underlying layers very nearby).
				  *
				  * These problem situations would arise when strong curvature is described
				  * by insufficient mesh divisions (causing the normal to fluctuate wildly
				  * between vertices and their neighbours). It seems to be difficult to
				  * devise rigorous checks on this, but the most logical choice appears to
				  * be to ensure that no underlying vertex has the original vertex as
				  * an underlying vertex.
				  */
//				void getUnderlyingVertex(Bundle * & bundle, xVert & index, const Layer * baseLayer, tiny::vec3 v)
				void getUnderlyingVertex(Bundle * & , xVert & , const Layer * , tiny::vec3 )
				{
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
				}

				~Terrain(void)
				{
				}

				virtual intf::UIInformation getUIInfo(void)
				{
					intf::UIInformation info;
					info.addPair("Memory usage",tool::convertToStringDelimited<long unsigned int>(usedCapacity())+" bytes");
					return info;
				}
		};
	}
}
