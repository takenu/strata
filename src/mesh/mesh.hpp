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
					ve.push_back( vertices.size() );
					vertices.push_back(v);
					vertices.back().clearPolys(); // The vertex should not use the polygons from the original copy (if any)
					vertices.back().index = ve.size()-1;
					return ve.size()-1;
				}

				void setParentLayer(Layer * layer) { parentLayer = layer; }
				Layer * getParentLayer(void) const { return parentLayer; }
			protected:
				friend class Bundle; // the Bundle also must use this class's protected functions for creating Strip objects

				using TopologicalMesh<VertexType>::vertices;
				using TopologicalMesh<VertexType>::polygons;
				using TopologicalMesh<VertexType>::ve;
				using TopologicalMesh<VertexType>::po;

				using TopologicalMesh<VertexType>::comparePolygons;
				using TopologicalMesh<VertexType>::findPolyNeighbor;
				using TopologicalMesh<VertexType>::findFarthestPair;
				using TopologicalMesh<VertexType>::verticesHaveCommonNeighbor;
				using TopologicalMesh<VertexType>::getVertexPosition;

				using MeshInterface::purgeVertexFromAdjacentMeshes;

				Layer * parentLayer;

				Mesh(core::intf::RenderInterface * _renderer) :
					TopologicalMesh<VertexType>(_renderer)
				{
				}

				/** Re-declare pure virtual function purgeVertex, originally from the MeshInterface. */
				virtual void purgeVertex(long unsigned int mfid, const xVert & oldVert, const xVert & newVert) = 0;

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
					for(unsigned int i = 0; i < vertices.size(); i++)
						if( tiny::length2(v.pos - vertices[i].pos) < tolerance*tolerance ) return vertices[i].index;
					return addVertex(v);
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
					{ std::cerr << " Polygon has too many links, cannot add polygon! "<<std::endl; return false; }
					po.push_back( polygons.size() );
					polygons.push_back( Polygon(a.index, b.index, c.index) );
					polygons.back().index = po.size()-1;
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) if(a.poly[i] == 0) { a.poly[i] = po.size()-1; break; }
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) if(b.poly[i] == 0) { b.poly[i] = po.size()-1; break; }
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) if(c.poly[i] == 0) { c.poly[i] = po.size()-1; break; }
					return true;
				}

				/** Add a polygon and, if they do not exist yet, add the vertices as well. */
				template <typename AnotherVertexType>
				bool addPolygonWithVertices(const AnotherVertexType &a, long unsigned int aid, const AnotherVertexType &b, long unsigned int bid,
						const AnotherVertexType &c, long unsigned int cid, float relativeTolerance = 0.001)
				{
					// Use tolerance of (relativeTolerance) times the smallest edge of the polygon to be added.
					float tolerance = std::min( tiny::length(a.pos - b.pos), std::min( tiny::length(a.pos - c.pos), tiny::length(b.pos - c.pos) ) )*relativeTolerance;
					xVert _a = addIfNewVertex(VertexType(a,aid), tolerance);
					xVert _b = addIfNewVertex(VertexType(b,bid), tolerance);
					xVert _c = addIfNewVertex(VertexType(c,cid), tolerance);
					return addPolygonFromVertexIndices(_a, _b, _c);
				}

				/** Merge a vertex 'v' with another vertex 'w', effectively removing 'v' from the Mesh. */
				void mergeVertices(const xVert &v, const xVert &w)
				{
					// All polygons currently using 'v' should use 'w' instead
					for(unsigned int i = 1; i < polygons.size(); i++)
						mergeAdjustPolygonIndices(polygons[i], v, w);
					// All adjacent meshes should stop using the vertex 'v'.
					purgeVertexFromAdjacentMeshes(v, w);
					// Remove the vertex from the list.
					deleteVertexFromArray(v);
				}

				/** Merge unallocated vertices with some neighbor that is allocated. This operation may be necessary
				  * to avoid situations where a vertex cannot be assigned either of the two split parts because doing
				  * so would result in a not-well-connected mesh. The solution is to merge it with a neighbor. */
				template <typename MeshType>
				void splitMergeOrphanVertices(MeshType * & f, MeshType * & g, std::map<xVert,xVert> &fvert, std::map<xVert,xVert> &gvert)
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
				//	std::cout << " Mesh::splitAddIfNewVertex() : attempt to add "<<w<<"..."<<std::endl;
					if( addedVertices.count(w) == 0 && otherVertices.count(w) == 0)
					{
				//		std::cout << " Mesh::splitAddIfNewVertex() : Added vertex "<<w<<" to Mesh "<<m->getKey()<<std::endl;
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

				/** Split the 'this' Mesh object by distributing its vertices among the meshes 'f' and 'g'. */
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
						std::cout << " Mesh::splitMesh() : Farthest pair vertices seem to be members of the same (very large) polygon. Cannot split! "<<std::endl;
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
				  */
				template <typename MeshTypeA, typename MeshTypeB, typename MeshTypeC>
				void splitAssignPolygonsToConstituentMeshes(MeshTypeA * f, MeshTypeB * g, MeshTypeC * s,
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
							try
							{
								assert(fvert.at(a) < f->ve.size());
								assert(fvert.at(b) < f->ve.size());
								assert(fvert.at(c) < f->ve.size());
								assert(f->ve[fvert.at(a)] < f->vertices.size());
								assert(f->ve[fvert.at(b)] < f->vertices.size());
								assert(f->ve[fvert.at(c)] < f->vertices.size());
							}
							catch(std::exception &e)
							{
								std::cout << " Mesh::splitAssignPolygonsToConstituentMeshes() : Exception: "<<e.what()<<" with a->"<<a<<" b->"<<b<<" c->"<<c<<" on ve of size "<<ve.size()<<std::endl;
							}
				//			std::cout << " Mesh::split() : f has "<<f->polygons.size()<<" polys and an index array of size "<<f->po.size()<<std::endl;
				//			f->addPolygon(f->vertices[f->ve[fvert.at(a)]], f->vertices[f->ve[fvert.at(b)]], f->vertices[f->ve[fvert.at(c)]]);
							f->addPolygonFromVertexIndices(fvert.at(a), fvert.at(b), fvert.at(c));
						}
						else if(fvert.find(a) == fvert.end() && fvert.find(b) == fvert.end() && fvert.find(c) == fvert.end()) // None of the vertices are in Bundle f? Then this polygon is in g.
						{
							try
							{
								assert(gvert.at(a) < g->ve.size());
								assert(gvert.at(b) < g->ve.size());
								assert(gvert.at(c) < g->ve.size());
								assert(g->ve[gvert.at(a)] < g->vertices.size());
								assert(g->ve[gvert.at(b)] < g->vertices.size());
								assert(g->ve[gvert.at(c)] < g->vertices.size());
							}
							catch(std::exception &e)
							{
								std::cout << " Mesh::splitAssignPolygonsToConstituentMeshes() : Exception: "<<e.what()<<" with a->"<<a<<" b->"<<b<<" c->"<<c<<" on ve of size "<<ve.size()<<std::endl;
							}
				//			std::cout << " Mesh::split() : g has "<<g->polygons.size()<<" polys and an index array of size "<<g->po.size()<<std::endl;
				//			g->addPolygon(g->vertices[g->ve[gvert.at(a)]], g->vertices[g->ve[gvert.at(b)]], g->vertices[g->ve[gvert.at(c)]]);
							g->addPolygonFromVertexIndices(gvert.at(a), gvert.at(b), gvert.at(c));
						}
						else
						{
							VertexType & _a = (fvert.find(a) == fvert.end() ? g->vertices[g->ve[gvert.at(a)]] : f->vertices[f->ve[fvert.at(a)]]);
							VertexType & _b = (fvert.find(b) == fvert.end() ? g->vertices[g->ve[gvert.at(b)]] : f->vertices[f->ve[fvert.at(b)]]);
							VertexType & _c = (fvert.find(c) == fvert.end() ? g->vertices[g->ve[gvert.at(c)]] : f->vertices[f->ve[fvert.at(c)]]);
							long unsigned int aid = (fvert.find(a) == fvert.end() ? g->getKey() : f->getKey());
							long unsigned int bid = (fvert.find(b) == fvert.end() ? g->getKey() : f->getKey());
							long unsigned int cid = (fvert.find(c) == fvert.end() ? g->getKey() : f->getKey());
				//			std::cout << " Mesh::split() : s has "<<s->nPolys()<<" polys and an index array of size "<<s->nPolyIndices()<<std::endl;
							s->addPolygonWithVertices(_a, aid, _b, bid, _c, cid); // Add to Stitch, and specify which vertices from which meshes it is using
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
					polygons[po[p]] = polygons.back(); // Move last polygon in the list to p's position
					polygons.pop_back(); // Delete the (now unreferenced, duplicate) polygon at the end of the list.
				}

				/** Delete a vertex from the vertices array. */
				void deleteVertexFromArray(const xVert &v)
				{
					ve[vertices.back().index] = ve[v];
					ve[v] = 0;
					vertices[ve[v]] = vertices.back();
					vertices.pop_back();
				}

				/** Delete a polygon from the Mesh, and clean up all references to it.
				  * This is a subtle function and careless use could result in meshes that are not well-connected. */
				void deletePolygon(Polygon &p)
				{
					for(unsigned int i = 1; i < vertices.size(); i++)
					{
						for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++)
						{
							if(vertices[i].poly[j] == 0) break;
							else if(vertices[i].poly[j] == p.index)
							{
								std::cout << " Mesh::deletePolygon() : Cleanse reference to polygon from Vertex "<<vertices[i].index<<"..."<<std::endl;
								for(unsigned int k = j; k < STRATA_VERTEX_MAX_LINKS-1; k++)
								{
									vertices[i].poly[k] = vertices[i].poly[k+1];
								}
								vertices[i].poly[STRATA_VERTEX_MAX_LINKS-1] = 0;
							}
						}
					}
					deletePolygonFromArray(p.index);
				}
		};
	} // end namespace mesh
} // end namespace strata
