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
								tiny::vec2(vertices[i].pos.z/scale + 0.5, vertices[i].pos.x/scale + 0.5), // texture coordinate
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

				// Re-define pure virtual function for splitting a mesh, first defined in MeshInterface.
				virtual void split(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip) = 0;
			protected:
				std::vector<VertexType> vertices;
				std::vector<Polygon> polygons;

				std::vector<xVert> ve;
				std::vector<xPoly> po;

				float scale; /**< Scale factor - coordinates should range from -scale/2 to scale/2 (used for texture coords) */

				/** Re-declare pure virtual function purgeVertex, originally from the MeshInterface. */
				virtual void purgeVertex(long unsigned int mfid, xVert oldVert, xVert newVert) = 0;

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

				void printLists(void) const
				{
					std::cout << " Printing MeshBundle lists: "<<std::endl;
					std::cout << " vertices: "; for(unsigned int i = 1; i < vertices.size(); i++) std::cout << i << ":"<<vertices[i].pos<<" (E="<<findEdgeVertex(vertices[i].index)<<"), "; std::cout << std::endl;
					std::cout << " vertex index: "; for(unsigned int i = 1; i < ve.size(); i++) std::cout << i << ":"<<ve[i]<<" @ "<<&vertices[ve[i]]<<", "; std::cout << std::endl;
					std::cout << " vertex check: "; for(unsigned int i = 1; i < ve.size(); i++) std::cout << i << ":"<<vertices[ve[i]].index<<", "; std::cout << std::endl;
					std::cout << " vertex polys: "<<std::endl;
					for(unsigned int i = 0; i < vertices.size(); i++)
					{
						std::cout << " vertex "<<i<<": index = "<<vertices[i].index<<", polys = ";
						for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++) std::cout << vertices[i].poly[j] << ", ";
						std::cout <<(isEdgeVertex(vertices[i].index)?"(E)":"")<< std::endl;
					}
				}

				void printPolygons(float step, unsigned int iterstep = 1) const
				{
					float div = 0.5f/step;
					std::cout << " polygons: ";
					for(unsigned int i = 0; i < polygons.size(); i+=iterstep)
					{
						std::cout << i << ":"<<polygons[i]<<" at "<<vertices[ve[polygons[i].a]].pos*div<<", "<<vertices[ve[polygons[i].b]].pos*div<<", "<<vertices[ve[polygons[i].c]].pos*div;
						std::cout << " diffs at "; printDifferentials(vertices[ve[polygons[i].a]].pos,div);
						std::cout << std::endl;
					}
				}

				TopologicalMesh(core::intf::RenderInterface * _renderer) :
					MeshInterface(_renderer),
					hasDesignatedEdgeVertices(false)
				{
					polygons.push_back( Polygon(0,0,0) );
					po.push_back(0); // po[0] shouldn't be used as a polygon because 0 is the "N/A" value for the Vertex's poly[] array
					vertices.push_back( VertexType(0.0f, 0.0f, 0.0f) );
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

				bool checkVertexIndices(void) const
				{
					bool vertexIndicesAreValid = true;
					for(unsigned int i = 1; i < vertices.size(); i++)
					{
						if(vertices[i].index < 1) vertexIndicesAreValid = false;
					}
					return vertexIndicesAreValid;
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
					return edgeVerticesAreValid;
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
					if(v<=0) printLists();
					assert(v>0);
					xVert result = 0;
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(vertices[ve[v]].poly[i] == 0) { std::cout << " findAdjacentEdgeVertex() : Failed to find next edge vertex! "<<std::endl; break; }
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
