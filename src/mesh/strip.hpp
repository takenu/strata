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

#include <vector>
#include <sstream>

#include <tiny/math/vec.h>
#include <tiny/algo/typecluster.h>

#include "mesh.hpp"

namespace strata
{
	namespace mesh
	{
		class Bundle;

		/** A borrowed vertex from some Bundle. */
		class StripVertex : public Vertex
		{
			private:
				Bundle * owner; /**< The Bundle that owns this Vertex. */
				xVert remoteIndex; /**< The index of this Vertex in its owning Bundle. */
			public:
				/** Create a StripVertex. Note that this does not set the 'index' field of the Vertex. 
				  * This form allows construction from remote index + remote bundle + position. This can be used
				  * when splitting meshes through the TopologicalMesh class. */
				StripVertex(tiny::vec3 _pos, Bundle * _owner, xVert _remoteIndex) : Vertex(_pos), owner(_owner), remoteIndex(_remoteIndex)
				{
				}

				/** Allow construction from existing vertex plus its owner. Used when copying a Vertex from a Bundle into a Strip.
				  * After construction, will not yet have a valid index which must be set by the Strip creating it. */
				StripVertex(const Vertex &v, Bundle * _owner) : StripVertex(v.pos,  _owner, v.index) {}

				/** A constructor for creating uninitialized strip vertices. Used by TopologicalMesh as the generic VertexType constructor. */
				StripVertex(tiny::vec3 _pos) : StripVertex(_pos, 0, 0) {}

				/** Allow construction from existing strip vertex.
				  * This duplicates the remoteIndex and mfid, and is used when making a copy of a StripVertex from a Strip for another Strip object. */
				StripVertex(const StripVertex &v, long unsigned int) : StripVertex(v.pos,  v.owner, v.remoteIndex) {}

				StripVertex(const StripVertex &v) : Vertex(v), owner(v.owner), remoteIndex(v.remoteIndex)
				{
				}

				const xVert & getRemoteIndex(void) const { return remoteIndex; }
				void setRemoteIndex(const xVert &v) { remoteIndex = v; }

				Bundle * getOwningBundle(void) { return owner; }
				const Bundle * getOwningBundle(void) const { return owner; }
				void setOwningBundle(Bundle * _owner) { owner = _owner; }
		};

		/** A standalone polygon defined by three StripVertices. Multiple of these
		  * polygons can be combined into a Strip. */
		class StripPolygon
		{
			private:
			public:
				StripVertex a;
				StripVertex b;
				StripVertex c;

				/** Construct a StripPolygon from three Vertices and their owning Bundles. */
				StripPolygon(Bundle * aowner, const Vertex &_a, Bundle * bowner,
						const Vertex &_b, Bundle * cowner, const Vertex &_c) :
					a(_a, aowner), b(_b, bowner), c(_c, cowner)
				{
				}

				/** Construct a StripPolygon from three StripVertices. */
				StripPolygon(const StripVertex &_a, const StripVertex &_b, const StripVertex &_c) :
					a(_a), b(_b), c(_c)
				{
				}
		};

		/** A class for special stitch-meshes, which do not contain vertices but which are used to link together
		  * meshes that do have vertices. They thus contain polygons whose vertices belong to distinct meshes. */
		class Strip : public tiny::algo::TypeClusterObject<long unsigned int, Strip>, public Mesh<StripVertex>
		{
			private:
				friend class Mesh<Vertex>; // to give the Bundle access to our protected Mesh base functions
				friend class Mesh<StripVertex>; // to let the Mesh access our protected getkey()

				/** A bool to determine whether the Strip is a Stitch (connecting distinct Layers)
				  * or a normal Strip (connecting Bundles of the same Layer).
				  * Normal Strips must have all three StripVertices for every Polygon within the same layer.
				  * Stitches may not have any Polygons for which all three StripVertices are in the same layer.
				  */
				const bool isStitch;

				std::vector<Bundle*> adjacentBundles; /**< A list of all Bundles that contain vertices used by polygons of this Strip. */

				/** Find a vertex neighbor to 'v' with remoteIndex 'r'. */
				virtual xVert findVertexNeighborByRemoteIndex(const Vertex &v, const xVert &r)
				{
					for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++)
					{
						if(v.poly[j] == 0)
						{
							std::cout << " Strip::findVertexNeighborByRemoteIndex() : ERROR: Failed to find neighbor to vertex "<<v.index<<"! "<<std::endl;
						}
						xVert n = findPolyNeighbor(polygons[po[v.poly[j]]],v.index,true);
						if(vertices[ve[n]].getRemoteIndex() == r) return n;
						n = findPolyNeighbor(polygons[po[v.poly[j]]],v.index,false);
						if(vertices[ve[n]].getRemoteIndex() == r) return n;
					}
					return 0;
				}

				/** Look through all vertices for the vertex to be removed. When found, merge the removed vertex with the designated new vertex. */
				void purgeVertex(Bundle * _owner, const xVert & oldVert, const xVert & newVert)
				{
					for(unsigned int i = 1; i < vertices.size(); i++)
					{
						if(vertices[i].getOwningBundle() == _owner)
						{
							if(vertices[i].getRemoteIndex() == oldVert)
							{
								mergeVertices(vertices[i].index, findVertexNeighborByRemoteIndex(vertices[i],newVert));
							}
						}
					}
				}

				/** Find the owning Bundle of the vertex indexed by 'v'. Originally declared as pure virtual in the Mesh class. */
				virtual Bundle * getVertexOwner(const xVert &v)
				{
					assert(v < ve.size());
					return vertices[ve[v]].getOwningBundle();
				}

				/** Find the remote index of a vertex indexed by 'v'. Originally declared as a pure virtual function in the Mesh class. */
				virtual xVert getRemoteVertexIndex(const xVert &v)
				{
					assert(v < ve.size());
					return vertices[ve[v]].getRemoteIndex();
				}

				/** Find out whether a set of vertices is adjacent to this mesh. The vertices are 'adjacent' if at least one
				  * of the vertices of this Strip has a remoteIndex equal to b's index. */
				virtual bool isAdjacentToVertices(const Bundle * b) const;

				virtual std::string printVertexInfo(const StripVertex & v) const
				{
					std::stringstream ss;
					ss << " r="<<v.getRemoteIndex();
					return ss.str();
				}
			protected:
			public:
				Strip(long unsigned int meshId, tiny::algo::TypeCluster<long unsigned int, Strip> &tc, intf::RenderInterface * _renderer, bool _isStitch) :
					tiny::algo::TypeClusterObject<long unsigned int, Strip>(meshId, this, tc),
					Mesh<StripVertex>(_renderer),
					isStitch(_isStitch)
				{
				}

				unsigned int usedMemory(void) const
				{
					return vertices.size()*sizeof(StripVertex) + polygons.size()*sizeof(Polygon)
						+ ve.size()*sizeof(xVert) + po.size()*sizeof(xPoly) + renderMesh->bufferSize();
				}

				unsigned int usedCapacity(void) const
				{
					return vertices.capacity()*sizeof(StripVertex) + polygons.capacity()*sizeof(Polygon)
						+ ve.capacity()*sizeof(xVert) + po.capacity()*sizeof(xPoly) + renderMesh->bufferSize();
				}

				unsigned int numberOfVertices(void) const
				{
					return vertices.size();
				}

				/** Re-calculate the Strip's vertex positions, bringing them back in line with the positions of the Bundle vertices
				  * that they were based upon. */
				void recalculateVertexPositions(void);

				/** Duplicate the strip, turning 's' into a copy of this Strip. This duplication is only possible
				  * if 's' is uninitialized and does not already have vertices and polygons.
				  * References of 's' into other Mesh objects (in particular, the Bundles from which this Strip
				  * borrows its vertices) are copied verbatim. The result should be that 's' is as close to a clone
				  * of this Strip as possible.
				  * The exception is that 's' is not given a parentLayer by this function in order to avoid 'accidental' or careless
				  * duplication. The caller of this function should ensure that 's' is sensibly assigned to a layer, either the same
				  * layer as 'this' (in this case the 'this' object probably would need to be removed eventually), or another layer
				  * (but in that case all the references to adjacent Bundles need to be adjusted to copies of these Bundles).
				  */
				void duplicateStrip(Strip * s) const;

				void duplicateAdjustAdjacentBundles(std::map<Bundle*, Bundle*> &bmap)
				{
					for(unsigned int i = 0; i < adjacentBundles.size(); i++)
					{
						if(bmap.find(adjacentBundles[i]) == bmap.end())
							std::cout << " Strip::duplicateAdjustAdjacentBundles() : WARNING: Failed to find adjacent bundle in map! "<<std::endl;
						else adjacentBundles[i] = bmap.at(adjacentBundles[i]);
					}
				}

				void duplicateAdjustOwningBundles(std::map<Bundle*, Bundle*> &bmap)
				{
					for(unsigned int i = 1; i < vertices.size(); i++)
					{
						if(bmap.find(vertices[i].getOwningBundle()) == bmap.end())
							std::cout << " Strip::duplicateAdjustOwningBundles() : WARNING: Failed to adjust owning bundle of a Strip vertex! "<<std::endl;
						else vertices[i].setOwningBundle(bmap.at(vertices[i].getOwningBundle()));
					}
				}

				bool isAdjacentToBundle(const Bundle * bundle) const
				{
					for(unsigned int i = 0; i < adjacentBundles.size(); i++)
						if(adjacentBundles[i] == bundle)
							return true;
					return false;
				}

				/** A function called by an adjacent Bundle that ceases to exist. All references to the Bundle should be considered invalid.
				  * In practice, except when the program is shutting down, the function that destroys the Bundle is responsible for repairing
				  * all the vertex references of the Strip, since the Strip cannot own vertices and needs to borrow its vertices from a valid
				  * Bundle.
				  */
				bool releaseAdjacentBundle(Bundle * bundle)
				{
					for(unsigned int i = 0; i < adjacentBundles.size(); i++)
						if(adjacentBundles[i] == bundle)
						{
							adjacentBundles[i] = adjacentBundles.back();
							adjacentBundles.pop_back();
							return true;
						}
					std::cout << " Strip::releaseAdjacentBundle() : Bundle should be adjacent but was not found! "<<std::endl;
					return false;
				}

				/** Add the Bundle as being adjacent to this Strip. */
				void addAdjacentBundle(Bundle * bundle)
				{
					for(unsigned int i = 0; i < adjacentBundles.size(); i++)
						if(adjacentBundles[i] == bundle) return;
					adjacentBundles.push_back(bundle);
				}

				/** Update all vertices in the Strip to refer to the new indices of the new Bundle, instead of the old one.
				  * This function also checks whether the old Bundle is actually adjacent to this Strip, and if so it returns
				  * 'true'. In that case the Bundle is expected to also add this Strip to its adjacentStrips vector.
				  */
				bool updateAdjacentBundle(const std::map<xVert, xVert> & vmap, Bundle * oldBundle, Bundle * newBundle)
				{
					bool isAdjacentMesh = false;
					for(unsigned int i = 1; i < vertices.size(); i++)
						if(vertices[i].getOwningBundle() == oldBundle && vmap.find(vertices[i].getRemoteIndex()) != vmap.end())
						{
							vertices[i].setOwningBundle(newBundle);
							vertices[i].setRemoteIndex(vmap.at(vertices[i].getRemoteIndex()));
							isAdjacentMesh = true;
						}
						else if(vertices[i].getOwningBundle() == newBundle)
							isAdjacentMesh = true; // If remote index and owning bundle are already adjusted, simply mark mesh as adjacent.
					if(isAdjacentMesh) addAdjacentBundle(newBundle);
					return isAdjacentMesh;
				}

				~Strip(void);

				unsigned int nPolys(void) const { return polygons.size(); }
				unsigned int nPolyIndices(void) const { return po.size(); }

//				virtual xVert addVertex(const StitchVertex &v)
//				{
//					return Mesh<StripVertex>::addVertex(v);
////					xVert _v = Mesh<StripVertex>::addVertex(v);
////					vertices[ve[_v]].setRemoteIndex(v.getRemoteIndex()); // Copy remote index of vertex in owner's mesh. (Shouldn't be necessary if copy constructor does what it is expected to do.)
//				}

				/** Create a Strip in order to connect meshbundle 'a' to 'b' on all vertices 'aVerts'. */
				void connectMeshes(Bundle &a, Bundle &b, std::vector<xVert> aVerts);

				/** Get the remote vertex that is the (counter)clockwise neighbor
				  * of the vertex with remote index 'w' with respect to 'v'. In other words,
				  * find the 'a' of the following situation (if clockwise=true):
				  *
				  * w---a
				  *  \ /
				  *   v
				  */
				xVert findRemoteVertexPolyNeighbor(const Bundle * &neighborBundle, xVert v, xVert w,
						const Bundle * vBundle, const Bundle * wBundle, bool clockwise) const;

				/** For stitch meshes, use direct analysis to calculate shape (i.e. skip first finding the edge vertices) since all
				  * stitch vertices are already edge vertices. */
				virtual float findFarthestPair(VertPair &farthestPair)
				{
					return analyseShapeDirect(farthestPair);
				}

				virtual bool isStitchMesh(void) const
				{
					return isStitch;
				}

				virtual bool split(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip);

				/** Check the correctness of the contents of the adjacentBundles array.
				  * This function checks the following:
				  * - Whether all Bundles in the array are the owner of at least one of the Strip's vertices,
				  * - whether all Bundles in the array contain a reverse reference to the Strip,
				  * - whether all vertices of this Strip refer to a Bundle that is in the adjacentBundles list,
				  * - whether there are no polygons for which all three vertices are borrowed from the same Bundle,
				  * - whether the referred Bundle actually contains a valid vertex referenced by the Strip vertex's remoteIndex,
				  * - whether the referred vertex has the same location as the Strip's vertex,
				  * - whether the Bundle also has a reference to this Strip.
				  */
				virtual bool checkAdjacentMeshes(void) const;
		};
	}
}
