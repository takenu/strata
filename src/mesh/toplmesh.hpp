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

namespace strata
{
	namespace mesh
	{
		template <typename VertexType> class Mesh;

		/** The TopologicalMesh defines the Mesh's basic topology as well as functions for analysing the topology.
		  * It does not define functions for manipulating the topology, this is deferred to the Mesh class which
		  * derives from it. */
		template <typename VertexType>
		class TopologicalMesh : public MeshInterface
		{
			friend class Mesh<VertexType>;
			public:
				/** Implement pure virtual function convertToMesh, originally from the DrawableMesh. */
				virtual tiny::mesh::StaticMesh convertToMesh(void) const
				{
					tiny::mesh::StaticMesh mesh;
					for(unsigned int i = 1; i < vertices.size(); i++) mesh.vertices.push_back( tiny::mesh::StaticMeshVertex(
								tiny::vec2(vertices[i].pos.z/scaleTexture + 0.5, vertices[i].pos.x/scaleTexture + 0.5), // texture coordinate
								tiny::vec3(1.0f,0.0f,0.0f), // tangent (appears to do nothing)
								(vertices[i].poly[0] > 0 ? computeNormal(vertices[i].poly[0]) : tiny::vec3(0.0f,1.0f,0.0f)),
								vertices[i].pos ) ); // position
					for(unsigned int i = 1; i < polygons.size(); i++)
					{
						mesh.indices.push_back( ve[polygons[i].c] - 1 ); // -1 because vertices[0] is the error value and the mesh doesn't have that so it's shifted by 1
						mesh.indices.push_back( ve[polygons[i].b] - 1 ); // Note that we add polygons in reverse order because OpenGL likes them counterclockwise while we store them clockwise
						mesh.indices.push_back( ve[polygons[i].a] - 1 );
					}

					return mesh;
				}

				virtual float meshSize(void) 
				{
					VertPair farthestPair(0,0);
					identifyEdgeVertices();
					return findFarthestPair(farthestPair);
				}

				/** Function is virtual: derived classes may improve upon this function by rewriting it (e.g. through not calling the expensive analyseShape() function).  */
				virtual float findFarthestPair(VertPair &farthestPair) const
				{
					return analyseShape(farthestPair);
				}

				tiny::vec3 getVertexPosition(xVert v) { return vertices[ve[v]].pos; }

				/** Set the scale multiplier for the terrain's texture coordinates. */
				void setScaleFactor(float _scale) { scaleTexture = _scale; }

				// Re-define pure virtual function for splitting a mesh, first defined in MeshInterface.
				virtual bool split(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip) = 0;
			protected:
				std::vector<VertexType> vertices;
				std::vector<Polygon> polygons;

				std::vector<xVert> ve;
				std::vector<xPoly> po;

				float scaleTexture; /**< Scale factor - coordinates should range from -scaleTexture/2 to scaleTexture/2 (used for texture coords) */

				/** Declare a function for adding vertices, which must be overloaded in the end-using class. */
				virtual xVert addVertex(const VertexType &v) = 0;

				/** Analyse the shape of the mesh, and return the pair of most distant vertices in the set. This only considers edge vertices (such
				  * that the calculation is easiest, also because for most sane meshes edge vertices are most distant, and because it is easier to
				  * work with edge vertices when e.g. splitting meshes). */
				float analyseShape(VertPair &farthestPair) const
				{
					std::vector<xVert> edgeVertices;
					xVert edgeStart = findRandomEdgeVertex();
					xVert edgeVertex = vertices[ve[edgeStart]].nextEdgeVertex;
					while(edgeVertex != edgeStart)
					{
//						edgeVertex = findAdjacentEdgeVertex(edgeVertex, true); // move by one edge vertex, clockwise
//						std::cout << " TopologicalMesh::analyseShape() : edgevertex = "<<edgeVertex<<std::endl;
						edgeVertex = vertices[ve[edgeVertex]].nextEdgeVertex;
						edgeVertices.push_back(edgeVertex); // this will add all edge vertices, finishing with edgeStart, after which the loop exits
					}
					return findFarthestPairFromList(edgeVertices, farthestPair);
				}

				/** Directly compare all vertex pairs to find the pair of vertices with the maximal separation. Typically slower than analyseShape() but
				  * faster for very thin, very long meshes (e.g. stitches). */
				float analyseShapeDirect(VertPair &farthestPair) const
				{
					return findFarthestPairFromList(ve, farthestPair);
				}

				float findFarthestPairFromList(const std::vector<xVert> &edgeVertices, VertPair &farthestPair) const
				{
					float maxDistance = 0.0f;
					for(unsigned int i = 0; i < edgeVertices.size(); i++)
					{
						for(unsigned int j = i+1; j < edgeVertices.size(); j++)
						{
							float dist = tiny::length( vertices[ve[edgeVertices[i]]].pos - vertices[ve[edgeVertices[j]]].pos );
							if(dist > maxDistance)
							{
								maxDistance = dist;
								farthestPair = VertPair(edgeVertices[i],edgeVertices[j]);
							}
						}
					}
					return maxDistance;
				}

				/** Compare two polygons. Return 'true' if they contain the same vertices in the same (clockwise) order. */
				inline bool comparePolygons(xVert a, xVert b, Polygon & k) const
				{
					if(a == k.a) return (b == k.b); // if a and b are the same vertices for both polygons, c must be as well
					else if(a == k.b) return (b == k.c);
					else if(a == k.c) return (b == k.a);
					else return false;
				}

				/** Allow the Strip to print the remoteIndex and origin of a borrowed vertex. */
				virtual std::string printVertexInfo(const VertexType &) const { return std::string(""); }

				void printLists(void) const
				{
					std::cout << " Printing TopologicalMesh lists: "<<std::endl;
					std::cout << " vertices: "; for(unsigned int i = 0; i < vertices.size(); i++) std::cout << i << ":"<<vertices[i].pos<<" (E="<<findEdgeVertex(vertices[i].index)<<"), "; std::cout << std::endl;
					std::cout << " vertex index: "; for(unsigned int i = 0; i < ve.size(); i++) std::cout << i << ":"<<ve[i]<<" @ "<<&vertices[ve[i]]<<", "; std::cout << std::endl;
					std::cout << " vertex check: "; for(unsigned int i = 0; i < ve.size(); i++) std::cout << i << ":"<<vertices[ve[i]].index<<", "; std::cout << std::endl;
					std::cout << " vertex polys: "<<std::endl;
					for(unsigned int i = 0; i < vertices.size(); i++)
					{
						std::cout << " vertex "<<i<<": index = "<<vertices[i].index<<", polys = ";
						for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++) std::cout << vertices[i].poly[j] << ", ";
						std::cout <<(isEdgeVertex(vertices[i].index)?"(E)":"")<< printVertexInfo(vertices[i])<<std::endl;
					}
				}

				void printPolygons(void) const
				{
					std::cout << " Printing TopologicalMesh polygons: ";
					for(unsigned int i = 0; i < polygons.size(); i++)
					{
						std::cout << i << ": "<<polygons[i].a<<","<<polygons[i].b<<","<<polygons[i].c<<std::endl;
					}
				}

				/** A function to compute how skewed (or stretched, or non-equilateral) a polygon is.
				  * Higher values reflect more deformed (less equilateral) polygons. Size is irrelevant, only the
				  * shape is taken into consideration.
				  * The test chosen is to compare the cumulative length of each pair of sides to the length of the
				  * third side. If all sides are roughly comparable in length, two sides will have a total length
				  * that is roughly twice the length of the third side. In extreme cases, two sides could have a length
				  * that is only slightly larger than that of the third side.
				  *
				  * For polygon edges a, b and c, the ratio c/(a+b-c) is 1 for an equilateral triangle, and can be
				  * extremely big (or small) for very long and thin polygons. The minimum of all three ratios for a polygon
				  * also cannot be lower than 1, such that we have a reasonably continuous way to measure the
				  * 'equilateral-ness' of polygons.
				  */
				inline float computePolygonSkew(const Polygon &p) const
				{
					return computePolygonSkew(p.a, p.b, p.c);
				}

				/** The direct version of computePolygonSkew which calculates the skew of a polygon of vertices
				  * a, b and c. The polygon does not need to exist in this TopologicalMesh. */
				inline float computePolygonSkew(const xVert &a, const xVert &b, const xVert &c) const
				{
					float x = tiny::length( vertices[ve[a]].pos - vertices[ve[b]].pos );
					float y = tiny::length( vertices[ve[b]].pos - vertices[ve[c]].pos );
					float z = tiny::length( vertices[ve[c]].pos - vertices[ve[a]].pos );
					if(x/y < 0.0001 || y/z < 0.0001 || z/x < 0.0001) return 1000000.0f; // Avoid numerical precision issues - sides could have relative length ~0
					return std::max( z/(x+y-z), std::max( y/(x+z-y), x/(y+z-x)) );
				}

				/** Compute the skew of a xVert's i-th polygon. */
				inline float computePolygonSkew(unsigned int i, const xVert &v) const
				{
					return computePolygonSkew(polygons[po[vertices[ve[v]].poly[i]]]);
				}

				/** Compute the skew of the xPoly's polygon. */
				inline float computePolygonSkew(const xPoly &p)
				{
					return computePolygonSkew(polygons[po[p]]);
				}

				TopologicalMesh(core::intf::RenderInterface * _renderer) :
					MeshInterface(_renderer),
					scaleTexture(1.0f),
					hasDesignatedEdgeVertices(false)
				{
					polygons.push_back( Polygon(0,0,0) );
					po.push_back(0); // po[0] shouldn't be used as a polygon because 0 is the "N/A" value for the Vertex's poly[] array
					vertices.push_back( tiny::vec3(0.0f, 0.0f, 0.0f) );
					ve.push_back(0); // ve[0] shouldn't be used either because 0 is the "N/A" value for the Vertex's nextEdgeVertex variable.
				}

				virtual ~TopologicalMesh(void) { polygons.clear(); vertices.clear(); ve.clear(); po.clear(); }

				/** Find a neighbouring vertex based on an edge.
				  *
				  * This function returns the vertex on the other side of the first edge radiating away from 'v' in a (counter)clockwise fashion, measured starting from j.
				  * It does this by trying to find the highest dot (inner) product among all vectors with a positive (or negative, for counterclockwise) cross product.
				  * If no neighbor is found with an angle less than 90 degrees (measured from the v-j vector) then 0 is returned (the non-existing vertex).
				  *
				  * Note that length is ignored, the algorithm doesn't care much how far it has to look to find a vertex in the right direction. However, it's guaranteed that
				  * it's already connected by a polygon also connecting to j, so the resulting polygon won't be much bigger than one already existing.
				  */
				inline xVert findNeighborVertex(const Vertex &j, const Vertex & v, bool clockwise) const
				{
					float bestInnerProd = 0.0f;
					xVert vert = 0;
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(v.poly[i]==0) break;
						else
						{
							const Vertex & w = vertices[ve[ findPolyNeighbor(polygons[po[v.poly[i]]],v.index,clockwise) ]];
							float innerProd = dot(j.pos - v.pos, normalize(w.pos - v.pos));
							if(innerProd > bestInnerProd && w.index != j.index) // skip j itself, it can show up if another polygon already exists on the other side
							{
								if( (dot(cross( w.pos - v.pos, j.pos - v.pos ),polyNormal(polygons[po[v.poly[i]]]) ) < 0.0f) != clockwise ) // note the inequality on two bools to generate XOR-like behavior
								{
									bestInnerProd = innerProd;
									vert = w.index;
								}
							}
						}
					}
					return vert;
				}

				/** Find a neighbor vertex in a polygon. This function gives nonsensical output if you call it for a vertex not part of this polygon. */
				inline const xVert & findPolyNeighbor(const Polygon &p, const xVert &v, bool clockwise) const
				{
					return (p.a == v ? (clockwise ? p.b : p.c) :
							(p.b == v ? (clockwise ? p.c : p.a) :
							 			 (clockwise ? p.a : p.b) ) );
				}

				/** Find a neighbor vertex in a polygon using the index of the polygon in the vertex. Redirects to findPolyNeighbor. */
				inline const xVert & findPolyNeighbor(unsigned int i, const xVert &v, bool clockwise) const
				{
					return findPolyNeighbor(polygons[po[vertices[ve[v]].poly[i]]],v,clockwise);
				}

				/** Find a neighbor vertex in a polygon using the polygon's index.
				  * This function cannot be named findPolyNeighbor as its signature would be too similar to the function
				  * findPolyNeighbor(unsigned int, const xVert &, bool).
				  */
				inline const xVert & findPolyNeighborFromIndex(const xPoly &p, const xVert &v, bool clockwise) const
				{
					assert(p>0);
					assert(p<po.size());
					assert(po[p]<polygons.size());
					return findPolyNeighbor(polygons[po[p]],v,clockwise);
				}

				/** Find the polygon where 'w' is a clockwise neighbor of 'v'. */
				inline xPoly findPolygon(const xVert &v, const xVert &w, bool abortIfNotFound = true) const
				{
					xPoly p = 0;
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(vertices[ve[v]].poly[i] == 0) break;
						if(findPolyNeighbor(i, v, true) == w) p = vertices[ve[v]].poly[i];
					}
					if(abortIfNotFound) assert(p>0); // Check that the polygon is successfully found
					return p;
				}

				/** Find a mutual neighbor to a pair of vertices, clockwise to w and counterclockwise to w. */
				inline xVert findPolyNeighborFromVertexPair(const xVert &v, const xVert &w) const
				{
					xPoly p = findPolygon(v, w, false);
					if(p>0) return findPolyNeighbor(polygons[po[p]], v, false);
					else return 0;
				}

				/** Find a vertex opposite to another vertex across a given edge. Example:
				  *   a *
				  *    / \
				  * b *---* c
				  *    \ /
				  *     * d
				  * If this function is called with (b,a,c) or (b,c,a) it should return d's index. */
				inline xVert findOppositeVertex(const xVert &a, const xVert &b, const xVert &c) const
				{
					xVert d = 0;
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(vertices[ve[b]].poly[i] == 0) break;
						if(	findPolyNeighbor(i, b, true) == c && findPolyNeighbor(i, b, false) != a)
						{
							d = findPolyNeighbor(i, b, false);
							break;
						}
						if(	findPolyNeighbor(i, b, false) == c && findPolyNeighbor(i, b, true) != a)
						{
							d = findPolyNeighbor(i, b, true);
							break;
						}
					}
					assert(d>0); // Make sure that this function returns a valid result. Caller must ensure it makes sense.
					return d;
				}

				/** Find an opposite vertex from polygon 'a' across a's polygon with index 'i'.
				  * Redirects to the standard findOppositeVertex(). */
				inline const xVert & findOppositeVertex(unsigned int i, const xVert &a) const
				{
					return findOppositeVertex(a, findPolyNeighbor(i, a, true), findPolyNeighbor(i, a, false));
				}

				/** Find the common vertex 'v' between polygons 'p' and 'q', with the additional requirement that the
				  * vertex that is counterclockwise from v in 'p' equals the vertex that is clockwise from v in 'q'.
				  * Graphically:
				  *   *
				  *  /q\
				  * *---* v   (the 'p' and 'q' are given here, we only need to find the xVert that refers to 'v'.)
				  *  \p/
				  *   *
				  */
				inline xVert findCommonEdgeVertex(const Polygon &p, const Polygon &q) const
				{
					if(p.a == q.a && findPolyNeighbor(p, p.a, false) == findPolyNeighbor(q, p.a, true)) return p.a;
					if(p.a == q.b && findPolyNeighbor(p, p.a, false) == findPolyNeighbor(q, p.a, true)) return p.a;
					if(p.a == q.c && findPolyNeighbor(p, p.a, false) == findPolyNeighbor(q, p.a, true)) return p.a;
					if(p.b == q.a && findPolyNeighbor(p, p.b, false) == findPolyNeighbor(q, p.b, true)) return p.b;
					if(p.b == q.b && findPolyNeighbor(p, p.b, false) == findPolyNeighbor(q, p.b, true)) return p.b;
					if(p.b == q.c && findPolyNeighbor(p, p.b, false) == findPolyNeighbor(q, p.b, true)) return p.b;
					if(p.c == q.a && findPolyNeighbor(p, p.c, false) == findPolyNeighbor(q, p.c, true)) return p.c;
					if(p.c == q.b && findPolyNeighbor(p, p.c, false) == findPolyNeighbor(q, p.c, true)) return p.c;
					if(p.c == q.c && findPolyNeighbor(p, p.c, false) == findPolyNeighbor(q, p.c, true)) return p.c;
					return 0;
				}

				xVert findCommonEdgeVertex(const xPoly &p, const xPoly &q) const
				{
					return findCommonEdgeVertex(polygons[po[p]],polygons[po[q]]);
				}

				/** Check whether there exists a vertex that is connected by a direct edge to both
				  * _a and _b. This is done by checking for all polygons of _a that all vertices in the
				  * polygon are not part of any of _b's polygons.
				  * A faster implementation could be developed using a directed loop around '_a' and '_b'
				  * where every time we find the next vertex, but this would require a bit more topological
				  * programming and therefore the current approach was chosen for the time being.
				  */
				inline bool verticesHaveCommonNeighbor(const xVert & _a, const xVert & _b) const
				{
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(vertices[ve[_a]].poly[i] == 0) break;
						for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++)
						{
							if(vertices[ve[_b]].poly[i] == 0) break;
							if (vertices[ve[polygons[po[vertices[ve[_a]].poly[i]]].a]].index == vertices[ve[polygons[po[vertices[ve[_b]].poly[j]]].a]].index ||
								vertices[ve[polygons[po[vertices[ve[_a]].poly[i]]].a]].index == vertices[ve[polygons[po[vertices[ve[_b]].poly[j]]].b]].index ||
								vertices[ve[polygons[po[vertices[ve[_a]].poly[i]]].a]].index == vertices[ve[polygons[po[vertices[ve[_b]].poly[j]]].c]].index ||
								vertices[ve[polygons[po[vertices[ve[_a]].poly[i]]].b]].index == vertices[ve[polygons[po[vertices[ve[_b]].poly[j]]].a]].index ||
								vertices[ve[polygons[po[vertices[ve[_a]].poly[i]]].b]].index == vertices[ve[polygons[po[vertices[ve[_b]].poly[j]]].b]].index ||
								vertices[ve[polygons[po[vertices[ve[_a]].poly[i]]].b]].index == vertices[ve[polygons[po[vertices[ve[_b]].poly[j]]].c]].index ||
								vertices[ve[polygons[po[vertices[ve[_a]].poly[i]]].c]].index == vertices[ve[polygons[po[vertices[ve[_b]].poly[j]]].a]].index ||
								vertices[ve[polygons[po[vertices[ve[_a]].poly[i]]].c]].index == vertices[ve[polygons[po[vertices[ve[_b]].poly[j]]].b]].index ||
								vertices[ve[polygons[po[vertices[ve[_a]].poly[i]]].c]].index == vertices[ve[polygons[po[vertices[ve[_b]].poly[j]]].c]].index ) return true;
						}
					}
					return false;
				}

				/** Check whether all vertex indices refer to the correct index of 've'. */
				bool checkVertexIndices(void) const
				{
					bool vertexIndicesAreValid = true;
					for(unsigned int i = 1; i < vertices.size(); i++)
					{
						if(vertices[i].index < 1)
						{
							std::cout << " TopologicalMesh::checkVertexIndices() : Vertex "<<i<<" has index "<<vertices[i].index<<"!"<<std::endl;
							vertexIndicesAreValid = false;
						}
						else if(vertices[i].index >= ve.size())
						{
							std::cout << " TopologicalMesh::checkVertexIndices() : Vertex "<<i<<" has index "<<vertices[i].index<<" on ve array of size "<<ve.size()<<"!"<<std::endl;
							vertexIndicesAreValid = false;
						}
						else if(i != ve[vertices[i].index])
						{
							std::cout << " TopologicalMesh::checkVertexIndices() : Vertex "<<i<<" has index referring to "<<ve[vertices[i].index]<<"!"<<std::endl;
							vertexIndicesAreValid = false;
						}
					}
					return vertexIndicesAreValid;
				}

				/** Check the validity of edge vertex values in Vertex::nextEdgeVertex. A value of 'true' is returned if and only if all such
				  * values are valid. */
				bool checkEdgeVertices(void) const
				{
					bool edgeVerticesAreValid = true;
					for(unsigned int i = 1; i < vertices.size(); i++)
					{
						if(vertices[i].nextEdgeVertex == 0 && isEdgeVertex(vertices[i].index)) { std::cout << " Mesh::checkEdgeVertices() : Bad vertex "<<i<<" has zero nextEdgeVertex but is an edge vertex! "<<std::endl; edgeVerticesAreValid = false; }
						if(vertices[i].nextEdgeVertex > 0 && !isEdgeVertex(vertices[i].index)) { std::cout << " Mesh::checkEdgeVertices() : Bad vertex "<<i<<" has nonzero nextEdgeVertex but is not on the edge! "<<std::endl; edgeVerticesAreValid = false; }
						if(vertices[i].nextEdgeVertex > 0 && !isEdgeVertex(vertices[i].nextEdgeVertex)) { std::cout << " Mesh::checkEdgeVertices() : Bad vertex "<<i<<" has nonzero nextEdgeVertex but the referenced vertex is not on the edge! "<<std::endl; edgeVerticesAreValid = false; }
					}
					if(!edgeVerticesAreValid) { printPolygons(); printLists(); }
//					assert(edgeVerticesAreValid);
					return edgeVerticesAreValid;
				}

				/** Check whether polygon indices are consistent and valid.
				  * This checks the following:
				  * - Polygon indices are correctly ordered (i.e. all zeroes in the array are at the end)
				  * - Polygon indices refer to valid elements of the po array
				  */
				bool checkVertexPolyArrays(void) const
				{
					bool polyArraysAreValid = true;
					for(unsigned int i = 1; i < vertices.size(); i++)
					{
						for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++)
						{
							if(j+1 < STRATA_VERTEX_MAX_LINKS && vertices[i].poly[j] == 0 && vertices[i].poly[j+1] > 0)
							{
								std::cout << " TopologicalMesh::checkVertexPolyArrays() : Polygon array has bad ordering! "<<std::endl;
								polyArraysAreValid = false;
							}
							else if(vertices[i].poly[j] > 0)
							{
								if(vertices[i].poly[j] >= po.size())
								{
									std::cout << " TopologicalMesh::checkVertexPolyArrays() : Polygon index too high! "<<std::endl;
									polyArraysAreValid = false;
								}
								else if( po[vertices[i].poly[j]] == 0)
								{
									std::cout << " TopologicalMesh::checkVertexPolyArrays() : Polygon index references zeroth polygon! "<<std::endl;
									polyArraysAreValid = false;
								}
								else
								{
									Polygon & p = polygons[po[vertices[i].poly[j]]];
									if(p.a != vertices[i].index && p.b != vertices[i].index && p.c != vertices[i].index)
									{
										std::cout << " TopologicalMesh::checkVertexPolyArrays() : Vertex is not a member of polygon ";
										std::cout << vertices[i].poly[j]<<"! "<<std::endl;
										polyArraysAreValid = false;
									}
								}
							}
						}
					}
					return polyArraysAreValid;
				}
			private:
				/** A flag for signaling whether or not the Mesh has a consistent and valid set of edge vertices. If true, Vertex::nextEdgeVertex is
				  * reliable and can be used to follow the edge. If false, the former is not guaranteed to be correct and, in general, should not be
				  * relied upon. */
				bool hasDesignatedEdgeVertices;

				void identifyEdgeVertices(void) 
				{
					for(unsigned int i = 1; i < vertices.size(); i++) vertices[i].nextEdgeVertex = 0;
					xVert startVertex = findRandomEdgeVertex();
					xVert edgeVertex = startVertex;
					xVert nextVertex = 0;
					while(nextVertex != startVertex)
					{
						nextVertex = findAdjacentEdgeVertex(edgeVertex, true);
						vertices[ve[edgeVertex]].nextEdgeVertex = nextVertex;
//						std::cout << " TopologicalMesh::identifyEdgeVertices() : nextVertex "<<nextVertex<<" is adjacent to "<<edgeVertex<<", start= "<<startVertex<<"..."<<std::endl;
						edgeVertex = nextVertex;
					}
					if(checkEdgeVertices())
						hasDesignatedEdgeVertices = true;
				}

				/** Calculate the normal of a polygon. */
				tiny::vec3 computeNormal(xPoly _p) const
				{
					return normalize(cross(vertices[ve[polygons[po[_p]].c]].pos - vertices[ve[polygons[po[_p]].a]].pos,
										   vertices[ve[polygons[po[_p]].b]].pos - vertices[ve[polygons[po[_p]].a]].pos));  // normal (use first poly's normal if available, otherwise use vertical)
				}

				inline bool isEdgeVertex(xVert _v) const
				{
					const Vertex & v = vertices[ve[_v]];
					if(v.poly[2] == 0) return true; // Vertices that connect to fewer than three polygons must be at the edge
					unsigned int verts[2*STRATA_VERTEX_MAX_LINKS];
					for(unsigned int i = 0; i < 2*STRATA_VERTEX_MAX_LINKS; i++) verts[i] = 0;
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(v.poly[i]==0) break;
						else
						{
							assert(v.poly[i] < po.size());
							assert(po[v.poly[i]] < polygons.size());
							verts[2*i  ] = findPolyNeighbor(polygons[po[v.poly[i]]], _v, true); // find both neighbours of the present vertex
							verts[2*i+1] = findPolyNeighbor(polygons[po[v.poly[i]]], _v, false);
						}
					}
//					std::cout << std::endl << " Vertices: "; for(unsigned int i = 0; i < 2*STRATA_VERTEX_MAX_LINKS; i++) std::cout << verts[i] << ", "; std::cout << std::endl;
					bool hasUniqueVertex = false;
					for(unsigned int i = 0; i < 2*STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(verts[i] == 0) continue; // This is a vertex for which we already found a partner
						hasUniqueVertex = true;
						for(unsigned int j = i+1; j < 2*STRATA_VERTEX_MAX_LINKS; j++)
						{
							if(verts[j] == verts[i])
							{
								verts[j] = 0;
								hasUniqueVertex = false;
							}
						}
						if(hasUniqueVertex) break; // Not found, then this is an edge vertex
					}
					return hasUniqueVertex;
				}

				/** Find another edge vertex, starting from the current vertex. If the parameter 'clockwise' is true the mesh edge will be traversed
				  * in a clockwise fashion (looking from the direction of the normals, or typically, from above the terrain). */
				xVert findAdjacentEdgeVertex(xVert v, bool clockwise) const
				{
					if(v<=0) {printPolygons(); printLists(); }
					assert(v>0);
					xVert result = 0;
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(vertices[ve[v]].poly[i] == 0) { std::cout << " findAdjacentEdgeVertex() : Failed to find next edge vertex to vertex "<<v<<"! "<<std::endl; break; }
						else if( isEdgeVertex( findPolyNeighbor(polygons[po[vertices[ve[v]].poly[i]]], v, clockwise) ) )
						{
							result = findPolyNeighbor(polygons[po[vertices[ve[v]].poly[i]]], v, clockwise);
							for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++)
							{
								if(i == j) continue;
								if(vertices[ve[v]].poly[j] == 0) break;
								if( findPolyNeighbor(polygons[po[vertices[ve[v]].poly[j]]], v, !clockwise) == result ) result = 0; // if vertex is in another polygon as well, it may be on the edge but not *along* the edge.
							}
							if(result != 0) break;
						}
					}
					return result;
				}

				/** A function guaranteed to return an edge vertex. */
				xVert findRandomEdgeVertex(bool _printSteps = false) const
				{
					unsigned int step = ve.size()/7 + 1;
					unsigned int startVertex = 1;
					unsigned int edgeVertex = (unsigned int)(-1);
					unsigned int maxAttemptsRemaining = 100;
					while(edgeVertex == (unsigned int)(-1) && (--maxAttemptsRemaining) != (unsigned int)-1)
					{
						startVertex = ((startVertex + step) % ve.size()); // take modulo to get next vertex
						if(startVertex != 0)
						{
							if(_printSteps)
							{
								std::cout << " TopologicalMesh::findRandomEdgeVertex() : start="<<startVertex;
								std::cout << " result="<<findEdgeVertex(startVertex, true)<<std::endl;
							}
							edgeVertex = findEdgeVertex(startVertex);
						}
					}
					if(edgeVertex == (unsigned int)-1 || !isEdgeVertex(edgeVertex)) { std::cout << " Mesh::findRandomEdgeVertex() : No edge vertex found! "<<std::endl; return 0; }
					return edgeVertex;
				}

				/** Find some vertex on the edge, starting at vertex v. This function may fail if it gets stuck in a highly deformed mesh, in this case it returns 0.
				  * This situation is best dealt with by trying to find an edge vertex but starting at another vertex randomly. */
				xVert findEdgeVertex(xVert _v, bool _printSteps = false) const
				{
					if(_v == 0) { std::cout << " Mesh::findEdgeVertex() : Cannot find edge vertex starting from xVert error value 0! "<<std::endl; return (unsigned int)(-1); }
					if(_printSteps) std::cout << " Trying edge vertex near xVert "<<_v<<"..."<<std::endl;
					if(isEdgeVertex(_v)) return _v;
					const Vertex & v = vertices[ve[_v]];
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(v.poly[i] == 0) break;
						if(_printSteps) std::cout << " Trying edge vertex near xVert "<<_v<<" for poly "<<v.poly[i]<<"..."<<std::endl;
						xVert w = findPolyNeighbor(polygons[po[v.poly[i]]], v.index, true); // Only need to consider one direction - the other vertex will be found in the neighbouring polygon for a non-edge vertex
						if(vertices[ve[w]].pos.x > v.pos.x) return findEdgeVertex(w);
					}
					std::cout << " Mesh::findEdgeVertex() : No edge vertex found! "<<std::endl;
					return (unsigned int)(-1);
				}

				/** Get a (non-normalized) normal vector for a polygon. */
				inline tiny::vec3 polyNormal(const Polygon &p) const
				{
					return cross( vertices[ve[p.c]].pos-vertices[ve[p.a]].pos, vertices[ve[p.b]].pos-vertices[ve[p.a]].pos);
				}
		};
	} // end namespace mesh
} // end namespace strata
