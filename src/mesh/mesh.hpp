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

#include <deque>
#include <map>

#include <tiny/math/vec.h>
#include <tiny/mesh/staticmesh.h>

#include "vecmath.hpp"
#include "interface.hpp"
#include "toplmesh.hpp"

namespace strata
{
	namespace mesh
	{
		class Layer; // for parentLayer, a pointer to the Layer to which this Mesh belongs

		/** A class to contain the parameters for constructing remote vertices for Strip objects. */
		class RemoteVertex
		{
			public:
				Bundle * owner;
				xVert index;
				tiny::vec3 pos;
				RemoteVertex(Bundle * b, xVert v, tiny::vec3 p) : owner(b), index(v), pos(p) {}
		};

		/** The Mesh is a base class for objects that contain parts of the terrain as a set of vertices connected via polygons.
		  * The VertexType is a type that represents a point in space. It should derive from the Vertex struct, or be a Vertex. It 
		  * needs a constructor that takes the form VertexType(float, float, float).
		  *
		  * Note that Mesh objects CANNOT HAVE HOLES in them, they can be strongly warped whatsoever but they must be isomorphic to
		  * the 2-dimensional unit disk. This requirement is made in order to ease finding the edge: the Mesh is widely assumed
		  * to have one continuous set of edge vertices. In addition, bottleneck vertices are not allowed (i.e. vertices that have
		  * four or more outgoing edges that are not adjacent to a polygon). For edge vertices, there should be only two vertices
		  * along which one can follow the edge of the mesh.
		  *
		  * This no-hole no-bottleneck requirement imposes strict limits on vertex deletion.
		  */
		template <typename VertexType>
		class Mesh : public TopologicalMesh<VertexType>
		{
			public:
				/** Add a vertex and return the xVert reference to that vertex. Note that careless construction of meshes will likely
				  * result in invalid meshes, this function should only be used if one ensures that all vertices end up being properly
				  * linked into a mesh (without holes or bottlenecks) by polygons.*/
				virtual xVert addVertex(const VertexType &v)
				{
					if(ve.size() == ve.capacity()) ve.reserve(ve.size()*1.05);
					if(vertices.size() == vertices.capacity()) vertices.reserve(vertices.size()*1.05);
					ve.push_back( vertices.size() );
					vertices.push_back(v);
					vertices.back().clearPolys(); // The vertex should not use the polygons from the original copy (if any)
					vertices.back().index = ve.size()-1;
					return ve.size()-1;
				}

				void setParentLayer(Layer * layer) { parentLayer = layer; }
				Layer * getParentLayer(void) { return parentLayer; }
				const Layer * getParentLayer(void) const { return parentLayer; }
			protected:
				friend class Bundle; // the Bundle also must use this class's protected functions for creating Strip objects

				using TopologicalMesh<VertexType>::vertices;
				using TopologicalMesh<VertexType>::polygons;
				using TopologicalMesh<VertexType>::ve;
				using TopologicalMesh<VertexType>::po;

				using TopologicalMesh<VertexType>::comparePolygons;
				using TopologicalMesh<VertexType>::computePolygonSkew;
				using TopologicalMesh<VertexType>::findPolygon;
				using TopologicalMesh<VertexType>::findPolyNeighbor;
				using TopologicalMesh<VertexType>::findPolyNeighborFromIndex;
				using TopologicalMesh<VertexType>::findFarthestPair;
				using TopologicalMesh<VertexType>::findCommonEdgeVertex;
				using TopologicalMesh<VertexType>::verticesHaveCommonNeighbor;
//				using TopologicalMesh<VertexType>::getVertexPosition;
				using TopologicalMesh<VertexType>::scaleTexture;

				using TopologicalMesh<VertexType>::printPolygons;
				using TopologicalMesh<VertexType>::printLists;

				Layer * parentLayer;

				Mesh(intf::RenderInterface * _renderer) :
					TopologicalMesh<VertexType>(_renderer),
					parentLayer(0)
				{
				}

				/** Get the owning Bundle of a vertex. If called on a Bundle, returns 'this'. If called on a
				  * Strip, returns the owning Bundle of the vertex v instead. */
				virtual Bundle * getVertexOwner(const xVert &v) = 0;

				/** Get the remote index of a vertex. If called on a Bundle, returns its argument. If called on a
				  * Strip, returns the remote index of the vertex in the Bundle returned by getVertexOwner. */
				virtual xVert getRemoteVertexIndex(const xVert &v) = 0;

				/** Delete a vertex. This function is in principle unsafe, may result in invalid meshes, and does not delete its adjacent polygons. */
				void delVertex(xVert j)
				{
					assert(j < ve.size());
					vertices[ve[j]] = vertices.back(); // copy last vertex to deleted vertex
					ve[vertices.back().index] = ve[j]; // delete last vertex
					vertices.pop_back(); // remove from vertex list
					ve[j] = 0; // remove from index list
				}

				/** Add a vertex v if it isn't added already. The tolerance determines the maximal difference between v's position
				  * and an existing vertex's position for which v is considered 'already present' in the mesh. */
				xVert addIfNewVertex(const VertexType &v, float tolerance)
				{
					for(unsigned int i = 1; i < vertices.size(); i++)
						if( tiny::length2(v.pos - vertices[i].pos) < tolerance*tolerance ) return vertices[i].index;
					return addVertex(v);
				}

				/** Add a vertex as a copy of another vertex. */
				void duplicateVertex(const VertexType &v)
				{
					if(vertices.size() == vertices.capacity()) vertices.reserve(vertices.size()*1.05);
					vertices.push_back(v);
				}

				/** Add a polygon as a copy of another polygon. */
				void duplicatePolygon(const Polygon &p)
				{
					if(polygons.size() == polygons.capacity()) polygons.reserve(polygons.size()*1.05);
					polygons.push_back(p);
				}

				/** Add a polygon using vertex indices rather than vertex references. */
				bool addPolygonFromVertexIndices(xVert _a, xVert _b, xVert _c)
				{
					return addPolygon(vertices[ve[_a]], vertices[ve[_b]], vertices[ve[_c]]); // Add polygon using vertices from this mesh.
				}

				/** Add a polygon between vertices a, b and c to the Mesh. Returns true if and only if the polygon was added. Checks for prior
				  * existence of the polygon so that duplicates are avoided. */
				bool addPolygon(VertexType &a, VertexType &b, VertexType &c)
				{
					// check whether polygon exists (by using a's list)
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(a.poly[i] >= po.size())
						{
							std::cout << " Mesh::addPolygon() : Bad poly array for vertex "<<a.index<<" in mesh with "<<po.size()<<" polygons: " << std::endl;
							for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++) std::cout << " poly["<<j<<"] = "<<a.poly[j]<<std::endl;
						}
						assert(a.poly[i] < po.size());
						assert(po[a.poly[i]] < polygons.size());
						if(a.poly[i] > 0 && comparePolygons(a.index, b.index, polygons[po[a.poly[i]]])) return false; // Polygon found
					}
					if(a.poly[STRATA_VERTEX_MAX_LINKS-1] > 0 || b.poly[STRATA_VERTEX_MAX_LINKS-1] > 0 || c.poly[STRATA_VERTEX_MAX_LINKS-1] > 0)
					{ std::cerr << " Mesh::addPolygon() : ERROR: Polygon has too many links, cannot add polygon! "<<std::endl; return false; }
					if(polygons.size() == polygons.capacity()) polygons.reserve(polygons.size()*1.05);
					if(po.size() == po.capacity()) po.reserve(po.size()*1.05);
					po.push_back( polygons.size() );
					polygons.push_back( Polygon(a.index, b.index, c.index) );
					polygons.back().index = po.size()-1;
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) if(a.poly[i] == 0) { a.poly[i] = po.size()-1; break; }
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) if(b.poly[i] == 0) { b.poly[i] = po.size()-1; break; }
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) if(c.poly[i] == 0) { c.poly[i] = po.size()-1; break; }
					return true;
				}

				/** Add a polygon and, if they do not exist yet, add the vertices as well. */
				bool addPolygonWithVertices(RemoteVertex a, RemoteVertex b, RemoteVertex c, float relativeTolerance = 0.001)
				{
					// Use tolerance of (relativeTolerance) times the smallest edge of the polygon to be added.
					float tolerance = std::min( tiny::length(a.pos - b.pos), std::min( tiny::length(a.pos - c.pos), tiny::length(b.pos - c.pos) ) )*relativeTolerance;
					xVert _a = addIfNewVertex(VertexType(a.pos, a.owner, a.index), tolerance);
					xVert _b = addIfNewVertex(VertexType(b.pos, b.owner, b.index), tolerance);
					xVert _c = addIfNewVertex(VertexType(c.pos, c.owner, c.index), tolerance);
					return addPolygonFromVertexIndices(_a, _b, _c);
				}

				/** Adjust the indexation of a polygon 'p' such that references to 'a' are replaced by references to 'b'.
				  * Additionally, this function removes the reference to 'p' in vertex 'a'.
				  */
				void adjustPolygonIndices(Polygon &p, const xVert &a, const xVert &b)
				{
					if(p.a == a) { p.a = b; deletePolygonFromVertex(p, vertices[ve[a]]); }
					if(p.b == a) { p.b = b; deletePolygonFromVertex(p, vertices[ve[a]]); }
					if(p.c == a) { p.c = b; deletePolygonFromVertex(p, vertices[ve[a]]); }
				}

				/** Adjust the indexation of all polygons next to vertex 'v', such that all references to vertex
				  * 'a' are replaced by references to vertex 'b'.
				  * This operation may be performed when an edge is split, e.g. by splitEdge().
				  * However, one can alternatively delete obsolete polygons and then add all necessary polygons.
				  */
				void adjustPolygons(const xVert &_v, const xVert &a, const xVert &b)
				{
					assert(_v!=a); // Forbid adjusting the vertex itself.
					assert(_v!=b);
					Vertex & v = vertices[ve[_v]];
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(v.poly[i] == 0) break;
						adjustPolygonIndices(polygons[po[v.poly[i]]], a, b);
					}
				}

				/** Merge a vertex 'v' with another vertex 'w', effectively removing 'v' from the Mesh. */
				void mergeVertices(const xVert &v, const xVert &w)
				{
					// All polygons currently using 'v' should use 'w' instead
					for(unsigned int i = 1; i < polygons.size(); i++)
						mergeAdjustPolygonIndices(polygons[i], v, w);
					// Remove the vertex from the list.
					deleteVertexFromArray(v);
				}

				/** Swap the common edge between polygons p and q.
				  * This deletes the current polygons p and q and replaces them by two new polygons. It also updates
				  * the polygon indices of all involved vertices.
				  *
				  * The picture is as follows:
				  *        * c                * c
				  *       /|\                /r\
				  *    a *p|q* b  ---->   a *---* b
				  *       \|/                \s/
				  *        * v                * v
				  */
				void swapEdge(const xPoly &p, const xPoly &q)
				{
					xVert v = findCommonEdgeVertex(p, q);
					if(v==0) { std::cout << " Mesh::swapEdge error: p="<<polygons[po[p]].a<<","<<polygons[po[p]].b<<","<<polygons[po[p]].c
						<<", q="<<polygons[po[q]].a<<","<<polygons[po[q]].b<<","<<polygons[po[q]].c<<std::endl; }
					assert(v>0);
					xVert a = findPolyNeighborFromIndex(p, v, true); // First collect vertex indices so we can delete p and q
					xVert b = findPolyNeighborFromIndex(q, v, false);
					xVert c = findPolyNeighborFromIndex(q, v, true);
					deletePolygon(p);
					deletePolygon(q);
					addPolygonFromVertexIndices(v, a, b);
					addPolygonFromVertexIndices(b, a, c);
				}

				/** Try to swap either of the two edges of 'p' of which 'v' is one of the two ends.
				  * The function first finds the vertices that could participate in the swap (getting an additional
				  * link in the process), and then carries out the swap provided both of the new polygons would have
				  * a skew (determined by TopologicalMesh::computePolygonSkew) that is better than the skew
				  * of 'p', and that both vertices that would add a link currently have fewer links than 'v'.
				  *
				  * The topology logic is as follows:
				  *        * p_b              *
				  *       /|\                / \
				  *   p_a*p| * b  ---->     *---*       (such that 'v' loses the connection to the top vertex)
				  *     / \|/              / \ /
				  *  a *---* v            *---* v
				  *
				  */
				bool attemptEdgeSwap(const xVert &v, const xPoly &p)
				{
					xVert p_a = findPolyNeighborFromIndex(p, v, true);
					xVert p_b = findPolyNeighborFromIndex(p, v, false);
					xPoly vaa = findPolygon(p_a, v, false);
					xPoly vbb = findPolygon(v, p_b, false);
					float pScore = computePolygonSkew(p);
					float aScore = (vaa > 0 ? computePolygonSkew(polygons[po[vaa]]) : pScore + 1.0f);
					float bScore = (vbb > 0 ? computePolygonSkew(polygons[po[vbb]]) : pScore + 1.0f);
					// Check all requirements for swapping the v-p_a edge to a a-p_b edge. This one also includes that a must score better than b.
					if(vaa > 0 && !(aScore < pScore && vertices[ve[findPolyNeighborFromIndex(vaa, v, true)]].nPolys() < vertices[ve[v]].nPolys()
								&& vertices[ve[p_b]].nPolys() < vertices[ve[v]].nPolys()))
						aScore = pScore + 1.0f; // If the swap is impossible, signal it by resetting its score.
					if(vbb > 0 && !(bScore < pScore && vertices[ve[findPolyNeighborFromIndex(vbb, v, false)]].nPolys() < vertices[ve[v]].nPolys()
								&& vertices[ve[p_a]].nPolys() < vertices[ve[v]].nPolys()))
						bScore = pScore + 1.0f;
					if(aScore < bScore && aScore < 0.999*pScore)
					{
						std::cout << " Swapping edge a, aScore="<<aScore<<", bScore="<<bScore<<", pScore="<<pScore<<std::endl;
						swapEdge(p, vaa);
						return true;
					}
					else if(bScore < 0.999*pScore)
					{
						std::cout << " Swapping edge b, aScore="<<aScore<<", bScore="<<bScore<<", pScore="<<pScore<<std::endl;
						swapEdge(p, vbb);
						return true;
					}
					else return false;
				}

				/** Balance a vertex's connections such that it is guaranteed to have space in its poly array.
				  * This function is allowed to modify a Mesh's polygon structure but it may not modify a Mesh's
				  * vertex structure.
				  *
				  * In order to rebalance, we search for the worst polygon adjacent to the Vertex referenced
				  * by 'v'. We then try to reduce v's connections by swapping one of the two edges of that polygon
				  * that are connecting to 'v'. If this edge is on the edge, it cannot be swapped. If the edge
				  * swapping would result in a polygon with a worse score (as determined by computePolygonSkew)
				  * the swap is also rejected.
				  *
				  * If both edges are not swappable, the function goes to the next worst edge, and so on, until
				  * either all edges are tried or a swap is successful (reducing the number of connections to 'v'
				  * by 1).
				  */
				void pruneExcessiveConnections(const xVert &v)
				{
					float pruneScore[STRATA_VERTEX_MAX_LINKS];
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(vertices[ve[v]].poly[i] == 0) pruneScore[i] = 0.0f;
						else pruneScore[i] = computePolygonSkew(i,v); // Note that computePolygonSkew should return a number >= 1.0f
					}
					unsigned int highestPruneScoreIndex = 0;
					bool pruningIsSuccessful = false;
					while(!pruningIsSuccessful)
					{
						highestPruneScoreIndex = 0;
						for(unsigned int i = 1; i < STRATA_VERTEX_MAX_LINKS; i++)
						{
							if(vertices[ve[v]].poly[i] == 0) break;
							if(pruneScore[i] > pruneScore[highestPruneScoreIndex])
								highestPruneScoreIndex = i;
						}
						if(pruneScore[highestPruneScoreIndex] == 0.0f) break; // Give up if all polygons have been attempted without success.
						else
						{
							pruningIsSuccessful = attemptEdgeSwap(v, vertices[ve[v]].poly[highestPruneScoreIndex]); // Try to swap either of the two edges of the polygon.
							pruneScore[highestPruneScoreIndex] = 0.0f; // Reset this polygon - we've tried it and if it didn't work we should not try it again.
						}
					}
					if(!pruningIsSuccessful)
					{
						std::cout << " Mesh::pruneExcessiveConnections() : NOTE: None of the "<<vertices[ve[v]].nPolys()
							<<" links of vertex "<<v<<" could be pruned. This may happen incidentally. "<<std::endl;
					}
				}

				/** Balance a mesh by swapping edges, in order to avoid situations where a vertex is a part of many
				  * long and thin polygons. */
				void rebalanceVertexConnections(void)
				{
					for(unsigned int i = 1; i < vertices.size(); i++)
					{
						unsigned int num_retries = 0;
						while(vertices[i].nPolys() > STRATA_VERTEX_LINK_THRESHOLD && num_retries < STRATA_VERTEX_MAX_LINKS - STRATA_VERTEX_LINK_THRESHOLD)
						{
							pruneExcessiveConnections(vertices[i].index);
							++num_retries;
						}
					}
				}

				/** Duplicate this mesh into another mesh. This is done by copying all vertices and polygons as-is
				  * into the target mesh.
				  * Not copied is the parent layer (whoever is duplicating should set what layer the new Mesh belongs to).
				  */
				template <typename MeshType>
				void duplicateMesh(MeshType * m) const
				{
					if(m->vertices.size() != 1 || m->polygons.size() != 1 || m->ve.size() != 1 || m->po.size() != 1)
					{
						std::cout << " Mesh::duplicateMesh() : ERROR: Mesh seems to be already initialized, cannot duplicate! "<<std::endl;
						std::cout << "                       : Sizes are: vertices="<<vertices.size()<<", polygons="<<polygons.size()
							<< ", ve="<<ve.size()<<", po="<<po.size()<<std::endl;
						return;
					}
					m->vertices.reserve(vertices.size()*1.05);
					m->ve.reserve(ve.size()*1.05);
					m->polygons.reserve(polygons.size()*1.05);
					m->po.reserve(po.size()*1.05);
					for(unsigned int i = 1; i < vertices.size(); i++)
						m->duplicateVertex(vertices[i]); // Copy vertices in order.
					for(unsigned int i = 1; i < ve.size(); i++)
						m->ve.push_back(ve[i]);
					for(unsigned int i = 1; i < polygons.size(); i++)
						m->duplicatePolygon(polygons[i]); // Copy polygons in order.
					for(unsigned int i = 1; i < po.size(); i++)
						m->po.push_back(po[i]);
					m->setScaleFactor(scaleTexture);
				}

				/** Split the edge between vertices 'b' and 'c'. Assume the following layout and splitting:
				  *   a *                a *
				  *    / \                /|\
				  * b *---* c  ---->   b *-*-* c     (the new vertex in the center will be called 'v')
				  *    \ /                \|/
				  *     * d                * d
				  * The vertices 'a' and 'd' do not need to exist, since bc could be at the edge of the mesh.
				  */
//				void splitEdge(const xVert &b, const xVert &c) <--- Don't do that! xVert may change while adding polys and then everything gets fucked!
				void splitEdge(xVert b, xVert c)
				{
					// Get the polygons next to bc. If bc is on the edge of the mesh, one of the xPoly's will be 0 and be ignored.
					xPoly cba = findPolygon(c, b, false);
					xPoly bcd = findPolygon(b, c, false);
					xVert a = 0;
					xVert d = 0;
					if(cba > 0)
					{
						a = findPolyNeighborFromIndex(cba, b, true);
						if(a == 0) std::cout << " Mesh::splitEdge() : WARNING: Couldn't find vertex 'a'! "<<std::endl;
						deletePolygon(cba);
					}
					if(bcd > 0)
					{
						d = findPolyNeighborFromIndex(bcd, c, true);
						if(d == 0) std::cout << " Mesh::splitEdge() : WARNING: Couldn't find vertex 'd'! "<<std::endl;
						deletePolygon(bcd);
					}
					tiny::vec3 pos = (vertices[ve[b]].pos + vertices[ve[c]].pos)*0.5; // Take the midpoint
					xVert v = addVertex(pos);
					vertices[ve[v]].thickness = (vertices[ve[b]].thickness + vertices[ve[c]].thickness)*0.5;
					if(a>0)
					{
//						std::cout << " Mesh::splitEdge() : Adding polygons for 'a' using new vertex v="<<v<<"..."<<std::endl;
						// Issue a warning if poly not added: the mesh cannot already have the polygon.
						if(!addPolygonFromVertexIndices(v,b,a)) std::cout << " Mesh::splitEdge() : WARNING: Polygon vba not added! "<<std::endl;
						if(!addPolygonFromVertexIndices(v,a,c)) std::cout << " Mesh::splitEdge() : WARNING: Polygon vac not added! "<<std::endl;
					}
					if(d>0)
					{
//						std::cout << " Mesh::splitEdge() : Adding polygons for 'd'..."<<std::endl;
						if(!addPolygonFromVertexIndices(v,d,b)) std::cout << " Mesh::splitEdge() : WARNING: Polygon vdb not added! "<<std::endl;
						if(!addPolygonFromVertexIndices(v,c,d)) std::cout << " Mesh::splitEdge() : WARNING: Polygon vcd not added! "<<std::endl;
					}
				}

				/** Split an edge opposite to vertex a across a's i-th polygon, by creating a new vertex at its midpoint.
				  * Redirects to splitEdge() called on only the edge vertices. */
				void splitEdgeFromPolygonIndex(unsigned int i, const xVert &a)
				{
					splitEdge(findPolyNeighbor(i, a, false), findPolyNeighbor(i, a, true));
				}

				/** Assign unassigned vertices to either f or g, whenever possible.
				  * This function supplements the primary assignment function (splitMesh()) by providing a way to add
				  * vertices to meshes in cases where splitMesh() was unable to add the vertex to either f or g because doing so would
				  * result in an invalid topology. Therefore, this function should be used after a topological correction
				  * has taken place (otherwise it would be unable to add it as well).
				  * This function should not be used in general for splitting meshes, since it does not make any efforts
				  * to split the mesh into two equal halves.
				  *
				  * Usage:
				  * To be used for assigning vertices not yet in 'f' or 'g' to either of these two. Assignment is done
				  * only if a polygon is found, containing the orphan vertex, for which both other vertices belong to the
				  * same mesh (i.e. f or g).
				  * The function returns 'true' if all vertices were assigned to either f or g.
				  */
				template <typename MeshType>
				bool splitAssignOrphanVertices(MeshType * f, MeshType * g, std::map<xVert,xVert> &fvert, std::map<xVert,xVert> &gvert)
				{
					bool retryAssignment = true; // This flag can be used to keep trying. Often, several iterations are required to assign all.
					bool allVerticesAreAssigned = true;
					while(retryAssignment)
					{
						retryAssignment = false;
						for(unsigned int i = 1; i < vertices.size(); i++)
						{
							if(fvert.find(vertices[i].index) == fvert.end() && gvert.find(vertices[i].index) == gvert.end())
							{
								for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++)
								{
									if(vertices[i].poly[j] == 0)
									{
										allVerticesAreAssigned = false; // At least one vertex remains unassigned.
										break;
									}
									if( fvert.find(findPolyNeighbor(j, vertices[i].index, true)) != fvert.end() &&
										fvert.find(findPolyNeighbor(j, vertices[i].index, false)) != fvert.end() )
									{
										retryAssignment = true; // Added a new vertex so we can try another iteration to add even more vertices
										fvert.insert( std::make_pair(vertices[i].index, f->addVertex(vertices[i])) );
										break;
									}
									else if( gvert.find(findPolyNeighbor(j, vertices[i].index, true)) != gvert.end() &&
										gvert.find(findPolyNeighbor(j, vertices[i].index, false)) != gvert.end() )
									{
										retryAssignment = true;
										gvert.insert( std::make_pair(vertices[i].index, g->addVertex(vertices[i])) );
										break;
									}
								}
							}
						}
						if(allVerticesAreAssigned) retryAssignment = false; // Stop assignment loop if all vertices are done.
					}
					return allVerticesAreAssigned;
				}

				/** Merge unallocated vertices with some neighbor that is allocated. This operation may be necessary
				  * to avoid situations where a vertex cannot be assigned either of the two split parts because doing
				  * so would result in a not-well-connected mesh. The solution is to merge it with a neighbor. */
				template <typename MeshType>
				void splitMergeOrphanVertices(MeshType * & /*f*/, MeshType * & /*g*/, std::map<xVert,xVert> &fvert, std::map<xVert,xVert> &gvert)
				{
					for(unsigned int i = 1; i < vertices.size(); i++)
						if(fvert.find(vertices[i].index) == fvert.end() && gvert.find(vertices[i].index) == gvert.end())
						{
							std::cout << " Mesh::splitMergeOrphanVertices() : Vertex "<<vertices[i].index<<" was not allocated, merging it with a neighbor... "<<std::endl;
							for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++)
							{
								if(vertices[i].poly[j] == 0)
								{
									std::cout << " Mesh::splitMergeOrphanVertices() : ERROR: Vertex "<<vertices[i].index<<" FAILED to be merged with a neighbor! "<<std::endl;
									break;
								}
								if(fvert.find(findPolyNeighbor(j, vertices[i].index, true)) != fvert.end())
								{
									std::cout << " Mesh::splitMergeOrphanVertices() : Vertex "<<vertices[i].index<<" to be merged with "<<findPolyNeighbor(j, vertices[i].index, true)<<"... "<<std::endl;
									mergeVertices(vertices[i].index, findPolyNeighbor(j, vertices[i].index, true));
									break;
								}
							}
						}
				}

				/** Add a new vertex provided that it has not yet been assigned a new TopologicalMesh.
				  *
				  * Parameters:
				  * - w             : the Vertex whose neighbours are to be considered as new members of Bundle 'b'
				  * - b             : the Bundle to which this function should add new vertices
				  * - newVertices   : list of vertices newly added to Bundle 'b', to be filled by this function
				  * - addedVertices : map of vertices from the original Bundle that already became members of the new Bundle 'b' (every vertex only should have 1 new Bundle)
				  * - otherVertices : list of vertices from the original Bundle that became members of a new Bundle other than 'b'
				  */
				template <typename MeshType>
				void splitAddIfNewVertex(const xVert & w, MeshType * m, std::vector<xVert> & newVertices,
						std::map<xVert, xVert> & addedVertices, const std::map<xVert, xVert> & otherVertices)
				{
					if( addedVertices.count(w) == 0 && otherVertices.count(w) == 0)
					{
						newVertices.push_back(w);
						addedVertices.insert( std::make_pair(w, m->addVertex(vertices[ve[w]]) ) ); // add vertex to the mapping of m's vertices
					}
				}

				/** Check whether a Vertex has at least one polygon for which both neighbours are already in a post-split Bundle. */
				bool splitVertexHasConnectedPolygon(const xVert &w, const std::map<xVert, xVert> & addedVertices) const
				{
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(vertices[ve[w]].poly[i] == 0) break;
						if(	addedVertices.find(findPolyNeighbor(polygons[po[vertices[ve[w]].poly[i]]], w, true)) != addedVertices.end() &&
							addedVertices.find(findPolyNeighbor(polygons[po[vertices[ve[w]].poly[i]]], w, false)) != addedVertices.end())
							return true; // Both poly neighbors are found in the mapping
					}
					return false;
				}

				/** Find new vertices to be added to a Bundle under construction, during the process of splitting an existing Bundle.
				  *
				  * Parameters:
				  * - oldVertices   : list of vertices of the original Bundle that were most recently added to Bundle 'b'
				  * - newVertices   : list of vertices newly added to Bundle 'b', to be filled by this function
				  * - addedVertices : map of vertices from the original Bundle that already became members of the new Bundle 'b' (every vertex only should have 1 new Bundle)
				  * - otherVertices : list of vertices from the original Bundle that became members of a new Bundle other than 'b'
				  * - b             : the Bundle to which this function should add new vertices
				  */
				template <typename MeshType>
				void splitAddNewVertices(const std::vector<xVert> & oldVertices, std::vector<xVert> & newVertices,
						std::map<xVert, xVert> & addedVertices, const std::map<xVert, xVert> & otherVertices, MeshType * m)
				{
					for(unsigned int i = 0; i < oldVertices.size(); i++)
					{
						VertexType & v = vertices[ve[oldVertices[i]]];
						xVert w = 0;
						for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++)
						{
							if(v.poly[j] == 0) break;
							assert(v.poly[j] < po.size());
							assert(po[v.poly[j]] < polygons.size());
							w = findPolyNeighbor(polygons[po[v.poly[j]]], oldVertices[i], true);
							if(splitVertexHasConnectedPolygon(w, addedVertices))
								splitAddIfNewVertex(w, m, newVertices, addedVertices, otherVertices);
							assert(v.poly[j] < po.size());
							assert(po[v.poly[j]] < polygons.size());
							w = findPolyNeighbor(polygons[po[v.poly[j]]], oldVertices[i], false);
							if(splitVertexHasConnectedPolygon(w, addedVertices))
								splitAddIfNewVertex(w, m, newVertices, addedVertices, otherVertices);
						}
					}
				}

				/** Split the 'this' Mesh object by distributing its vertices among the meshes 'f' and 'g'.
				  * This function should be called only once since it creates the new mesh objects 'f' and 'g'.
				  */
				template <typename MeshType>
				void splitMesh(std::function<MeshType * (void)> makeNewMesh, MeshType * & f, MeshType * & g,
						std::map<xVert,xVert> &fvert, std::map<xVert,xVert> &gvert)
				{
					VertPair farthestPair(0,0);
					findFarthestPair(farthestPair);

					// Check that the farthest pair vertices are not part of the same polygon (by checking that b is not part of any of a's polygons),
					// and that they also are not connected to the same vertex.
					if( verticesHaveCommonNeighbor(farthestPair.a, farthestPair.b) )
					{
						std::cout << " Mesh::splitMesh() : There exists a vertex that is a neighbor to both of the farthestPair vertices. Cannot split! "<<std::endl;
						return;
					}

					f = makeNewMesh(); f->setParentLayer(parentLayer); // Parent layer is shared among Bundles of the same Layer
					g = makeNewMesh(); g->setParentLayer(parentLayer);

					xVert v;

					v = f->addVertex(vertices[ve[farthestPair.a]]); fvert.emplace(farthestPair.a, v);
					v = g->addVertex(vertices[ve[farthestPair.b]]); gvert.emplace(farthestPair.b, v);

					std::vector<xVert> fOldVertices, fNewVertices, gOldVertices, gNewVertices;
					fOldVertices.push_back(farthestPair.a);
					gOldVertices.push_back(farthestPair.b);
					// First add all the neighbours of the initial vertex, while avoiding the usual check that it is well-connected to the Bundle.
					// That check only works well if there is at least 1 edge already present in the Bundle.
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(vertices[ve[farthestPair.a]].poly[i] != 0)
						{
							splitAddIfNewVertex(polygons[po[vertices[ve[farthestPair.a]].poly[i]]].a, f, fNewVertices, fvert, gvert);
							splitAddIfNewVertex(polygons[po[vertices[ve[farthestPair.a]].poly[i]]].b, f, fNewVertices, fvert, gvert);
							splitAddIfNewVertex(polygons[po[vertices[ve[farthestPair.a]].poly[i]]].c, f, fNewVertices, fvert, gvert);
						}
						if(vertices[ve[farthestPair.b]].poly[i] != 0)
						{
							splitAddIfNewVertex(polygons[po[vertices[ve[farthestPair.b]].poly[i]]].a, g, gNewVertices, gvert, fvert);
							splitAddIfNewVertex(polygons[po[vertices[ve[farthestPair.b]].poly[i]]].b, g, gNewVertices, gvert, fvert);
							splitAddIfNewVertex(polygons[po[vertices[ve[farthestPair.b]].poly[i]]].c, g, gNewVertices, gvert, fvert);
						}
					}
					fOldVertices.swap(fNewVertices);
					gOldVertices.swap(gNewVertices);
					fNewVertices.clear();
					gNewVertices.clear();
					// Now add all other vertices using splitAddNewVertices() which looks for well-connected neighbors.
					while(fOldVertices.size() > 0 || gOldVertices.size() > 0)
					{
						splitAddNewVertices(fOldVertices, fNewVertices, fvert, gvert, f);
						splitAddNewVertices(gOldVertices, gNewVertices, gvert, fvert, g);
						fOldVertices.swap(fNewVertices);
						gOldVertices.swap(gNewVertices);
						fNewVertices.clear();
						gNewVertices.clear();
					}
				}

				/** Assign all polygons of the Mesh to the three Mesh objects passed to this function.
				  * If all vertices of a polygon are in the 'fvert' mapping, the polygon is added to 'f'.
				  * If all vertices of a polygon are in the 'gvert' mapping, the polygon is added to 'g'.
				  * In all other cases (where the vertices are split) the polygon is added to 's'.
				  * The latter object is typically a Strip since it doesn't make sense to add
				  * polygons to Bundles if the vertices are not in a single Mesh object.
				  *
				  * Note that the index mappings fvert and gvert map the vertices of 'this' into the
				  * Mesh objects 'f' and 'g'. Therefore one needs to use the mapping to access the
				  * proper indexation for the polygons when adding them to their new mesh.
				  */
//				template <typename MeshTypeA, typename MeshTypeB>
				template <typename MeshType, typename StripMeshType>
				void splitAssignPolygonsToConstituentMeshes(MeshType * f, MeshType * g, StripMeshType * s,
						std::map<xVert,xVert> &fvert, std::map<xVert,xVert> &gvert)
				{
					for(unsigned int i = 1; i < polygons.size(); i++)
					{
						xVert a = polygons[i].a;
						xVert b = polygons[i].b;
						xVert c = polygons[i].c;
						// Add polygon to the correct object.
						if(gvert.find(a) == gvert.end() && gvert.find(b) == gvert.end() && gvert.find(c) == gvert.end()) // None of the vertices are in Bundle g? Then this polygon is in f.
						{
							assert(fvert.at(a) < f->ve.size());
							assert(fvert.at(b) < f->ve.size());
							assert(fvert.at(c) < f->ve.size());
							assert(f->ve[fvert.at(a)] < f->vertices.size());
							assert(f->ve[fvert.at(b)] < f->vertices.size());
							assert(f->ve[fvert.at(c)] < f->vertices.size());
							f->addPolygonFromVertexIndices(fvert.at(a), fvert.at(b), fvert.at(c));
						}
						else if(fvert.find(a) == fvert.end() && fvert.find(b) == fvert.end() && fvert.find(c) == fvert.end()) // None of the vertices are in Bundle f? Then this polygon is in g.
						{
							assert(gvert.at(a) < g->ve.size());
							assert(gvert.at(b) < g->ve.size());
							assert(gvert.at(c) < g->ve.size());
							assert(g->ve[gvert.at(a)] < g->vertices.size());
							assert(g->ve[gvert.at(b)] < g->vertices.size());
							assert(g->ve[gvert.at(c)] < g->vertices.size());
							g->addPolygonFromVertexIndices(gvert.at(a), gvert.at(b), gvert.at(c));
						}
						else
						{
							assert(fvert.find(a) != fvert.end() || gvert.find(a) != gvert.end()); // Make sure a/b/c are at least somewhere
							assert(fvert.find(b) != fvert.end() || gvert.find(b) != gvert.end());
							assert(fvert.find(c) != fvert.end() || gvert.find(c) != gvert.end());
							Bundle * _abundle = (fvert.find(a) == fvert.end() ? g->getVertexOwner(gvert.at(a)) : f->getVertexOwner(fvert.at(a)));
							Bundle * _bbundle = (fvert.find(b) == fvert.end() ? g->getVertexOwner(gvert.at(b)) : f->getVertexOwner(fvert.at(b)));
							Bundle * _cbundle = (fvert.find(c) == fvert.end() ? g->getVertexOwner(gvert.at(c)) : f->getVertexOwner(fvert.at(c)));
							xVert _a = (fvert.find(a) == fvert.end() ? g->getRemoteVertexIndex(gvert.at(a)) : f->getRemoteVertexIndex(fvert.at(a)));
							xVert _b = (fvert.find(b) == fvert.end() ? g->getRemoteVertexIndex(gvert.at(b)) : f->getRemoteVertexIndex(fvert.at(b)));
							xVert _c = (fvert.find(c) == fvert.end() ? g->getRemoteVertexIndex(gvert.at(c)) : f->getRemoteVertexIndex(fvert.at(c)));
							tiny::vec3 ap = vertices[ve[a]].pos;
							tiny::vec3 bp = vertices[ve[b]].pos;
							tiny::vec3 cp = vertices[ve[c]].pos;
							// Add to Stitch, and specify which vertices from which meshes it is using
							s->addPolygonWithVertices(RemoteVertex(_abundle, _a, ap), RemoteVertex(_bbundle, _b, bp), RemoteVertex(_cbundle, _c, cp));
						}
					}
				}
			private:
				/** While merging, adjust polygon indices such that all references to 'v' become references to 'w' instead. */
				void mergeAdjustPolygonIndices(Polygon &p, const xVert &v, const xVert &w)
				{
						if(p.a == v) { p.a = w; cleanupIfDegeneratePolygon(p); }
						if(p.b == v) { p.b = w; cleanupIfDegeneratePolygon(p); }
						if(p.c == v) { p.c = w; cleanupIfDegeneratePolygon(p); }
				}

				/** Cleanup polygons with two identical vertex indices (i.e. with zero area). */
				void cleanupIfDegeneratePolygon(Polygon &p)
				{
					if(p.a == p.b || p.a == p.c || p.b == p.c) deletePolygon(p);
				}

				/** Delete a polygon from the array of polygons. */
				void deletePolygonFromArray(const xPoly &p)
				{
					po[polygons.back().index] = po[p]; // Change indexing of last polygon to p's location in 'polygons'
					po[p] = 0; // Refer to nowhere for the to-be-deleted polygon. (No one should be using 'p.index' anymore.)
					polygons[po[polygons.back().index]] = polygons.back(); // Move last polygon in the list to p's position
					polygons.pop_back(); // Delete the (now unreferenced, duplicate) polygon at the end of the list.
				}

				/** Delete a vertex from the vertices array. */
				void deleteVertexFromArray(const xVert &v)
				{
					ve[vertices.back().index] = ve[v];
					vertices[ve[v]] = vertices.back();
					ve[v] = 0;
					vertices.pop_back();
				}

				/** Delete the xPoly reference to a Polygon from a Vertex. */
				inline void deletePolygonFromVertex(Polygon &p, Vertex &v)
				{
					for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++)
					{
						if(v.poly[j] == 0) break;
						else if(v.poly[j] == p.index)
						{
							for(unsigned int k = j; k < STRATA_VERTEX_MAX_LINKS-1; k++)
							{
								v.poly[k] = v.poly[k+1];
							}
							v.poly[STRATA_VERTEX_MAX_LINKS-1] = 0;
						}
					}
				}

				/** Delete a polygon from the Mesh, and clean up all references to it.
				  * This is a subtle function and careless use could result in meshes that are not well-connected. */
				void deletePolygon(Polygon &p)
				{
					assert(p.a < ve.size()); assert( ve[p.a] < vertices.size());
					assert(p.b < ve.size()); assert( ve[p.b] < vertices.size());
					assert(p.c < ve.size()); assert( ve[p.c] < vertices.size());
					deletePolygonFromVertex(p, vertices[ve[p.a]]);
					deletePolygonFromVertex(p, vertices[ve[p.b]]);
					deletePolygonFromVertex(p, vertices[ve[p.c]]);
					deletePolygonFromArray(p.index);
				}

				/** Delete a polygon from the Mesh by its xPoly reference. */
				void deletePolygon(const xPoly &p)
				{
					deletePolygon(polygons[po[p]]);
				}
		};
	} // end namespace mesh
} // end namespace strata
