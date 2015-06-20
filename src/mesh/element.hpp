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
#include <deque>
#include <map>
#include <functional>

#include <tiny/math/vec.h>
#include <tiny/mesh/staticmesh.h>

#include "vecmath.hpp"
#include "drawable.hpp"

#define STRATA_VERTEX_MAX_LINKS 11

namespace strata
{
	namespace mesh
	{
		// Typedefs. These link into an std::vector using two-layer indexation. Index '0' is reserved for uninitialized data.
		typedef unsigned int xVert;
		typedef unsigned int xPoly;

		/** A vertex, for being part of a mesh. */
		struct Vertex
		{
			tiny::vec3 pos;
			xVert index; /**< The index of this vertex in the 've' array of the MeshBundle. */
			xVert nextEdgeVertex; /**< The next edge vertex, if this vertex itself is on the edge. Otherwise 0. */
			xPoly poly[STRATA_VERTEX_MAX_LINKS]; /**< Enforce max number of links (to avoid having to (de)allocate memory when creating a Vertex). */

			Vertex(const tiny::vec3 &p) : pos(p), index(0), nextEdgeVertex(0)
			{
				for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					poly[i] = 0;
			}

			Vertex(float x, float y, float z) : Vertex(tiny::vec3(x,y,z)) {}

			Vertex & operator= (const Vertex &v) { pos = v.pos; index = v.index; for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) poly[i] = v.poly[i]; return *this; }
		};

		/** A polygon, for being part of a mesh. */
		struct Polygon
		{
			xVert a; /**< Vertices for the polygon, in a-b-c clockwise order. (The normal is upwards from this direction.) */
			xVert b;
			xVert c;

			xPoly index; /**< The index of this vertex in the 'po' array of the MeshBundle. */

			Polygon(xVert _a, xVert _b, xVert _c) : a(_a), b(_b), c(_c) {}
		};

		inline std::ostream & operator<< (std::ostream &s, const Vertex &v) { s << v.index; return s; }
		inline std::ostream & operator<< (std::ostream &s, const Polygon &p) { s << "("<<p.a<<","<<p.b<<","<<p.c<<")"; return s; }

		/** A class for containing a pair of vertices. */
		struct VertPair
		{
			xVert a;
			xVert b;

			VertPair(xVert _a, xVert _b) : a(_a), b(_b) {}
		};

		class Bundle;
		class Strip;

		/** An interface for the Mesh class. */
		class MeshInterface : public DrawableMesh
		{
			private:
			protected:
				/** A list of meshes adjacent to the current one. If a vertex is deleted, all adjacent meshes are notified such that
				  * they can update their mapping. */
				std::vector<MeshInterface*> adjacentMeshes;

				/** Purge a vertex, cleaning it from all references. After this operation, the derived class should consider the vertex with
				  * index "oldVert" belonging to the mesh fragment with id "mfid" as no longer existing. The vertex with index "newVert" is the
				  * suggested replacement of the old vertex. */
				virtual void purgeVertex(long unsigned int mfid, xVert oldVert, xVert newVert) = 0;

				/** Purge an adjacent mesh, removing it from the adjacentMeshes list. After this action is performed, the adjacent mesh
				  * removed from the list will no longer be informed of changes and communication (e.g. regarding vertices being deleted
				  * or added) becomes impossible. In general, this function should be used only immediately before deleting a mesh. */
				void purgeAdjacentMesh(MeshInterface * _mesh)
				{
					for(unsigned int i = 0; i < adjacentMeshes.size(); i++)
						if(adjacentMeshes[i] == _mesh)
						{
							adjacentMeshes[i] = adjacentMeshes.back();
							adjacentMeshes.pop_back();
						}
				}

				/** Check whether a given mesh is already listed as adjacent to this mesh. */
				bool hasAdjacentMesh(MeshInterface * _mesh)
				{
					for(unsigned int i = 0; i < adjacentMeshes.size(); i++)
						if(adjacentMeshes[i] == _mesh) return true;
					return false;
				}

				/** Add an adjacent mesh, if it doesn't exist yet. */
				void addAdjacentMesh(MeshInterface * _mesh)
				{
					if(!hasAdjacentMesh(_mesh))
						adjacentMeshes.push_back(_mesh);
				}

				MeshInterface(core::intf::RenderInterface * _renderer) : DrawableMesh(_renderer) {}

				virtual ~MeshInterface(void)
				{
					for(unsigned int i = 0; i < adjacentMeshes.size(); i++)
						adjacentMeshes[i]->purgeAdjacentMesh(this);
				}
			public:
				/** Split the Mesh into two parts. This operation should always reduce the size() of the Mesh. */
				virtual void split(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip) = 0;

				/** Determine the size of the fragment, defined as the maximal end-to-end distance between two edge vertices. */
				virtual float meshSize(void) = 0;

				virtual tiny::mesh::StaticMesh convertToMesh(void) = 0;
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
		class Mesh : public MeshInterface
		{
			public:
				/** Convert the Mesh to a StaticMesh object. */
				virtual tiny::mesh::StaticMesh convertToMesh(void)
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

				float size(void)
				{
					VertPair farthestPair(0,0);
					return findFarthestPair(farthestPair);
				}

				/** Function is virtual: derived classes may improve upon this function by rewriting it (e.g. through not calling the expensive analyseShape() function).  */
				virtual float findFarthestPair(VertPair &farthestPair)
				{
					return analyseShape(farthestPair);
				}

				/** Add a vertex and return the xVert reference to that vertex. Note that careless construction of meshes will likely
				  * result in invalid meshes, this function  should only be used if one ensures that all vertices end up being properly
				  * linked into a mesh (without holes or bottlenecks) by polygons.*/
				xVert addVertex(const Vertex &v)
				{
					ve.push_back( vertices.size() );
					vertices.push_back(v);
					vertices.back().index = ve.size()-1;
					return ve.size()-1;
				}
				xVert addVertex(tiny::vec3 &p) { return addVertex( Vertex(p) ); }
				xVert addVertex(float x, float y, float z) { return addVertex( Vertex(tiny::vec3(x,y,z)) ); }

				tiny::vec3 getVertexPosition(xVert v) { return vertices[ve[v]].pos; }

				// Re-define pure virtual functions from MeshInterface.
				virtual void split(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip) = 0;
				virtual float meshSize(void) = 0;
			protected:
				std::vector<VertexType> vertices;
				std::vector<Polygon> polygons;

				std::vector<xVert> ve;
				std::vector<xPoly> po;

				float scale; /**< Scale factor - coordinates should range from -scale/2 to scale/2 (used for texture coords) */

				virtual void purgeVertex(long unsigned int mfid, xVert oldVert, xVert newVert) = 0;

				/** Analyse the shape of the mesh, and return the pair of most distant vertices in the set. This only considers edge vertices (such
				  * that the calculation is easiest, also because for most sane meshes edge vertices are most distant, and because it is easier to
				  * work with edge vertices when e.g. splitting meshes). */
				float analyseShape(VertPair &farthestPair)
				{
					std::vector<xVert> edgeVertices;
					xVert edgeStart = findRandomEdgeVertex();
					xVert edgeVertex = 0;
					while(edgeVertex != edgeStart)
					{
						edgeVertex = findAdjacentEdgeVertex(edgeVertex, true); // move by one edge vertex, clockwise
						edgeVertices.push_back(edgeVertex); // this will add all edge vertices, finishing with edgeStart, after which the loop exits
					}
					return findFarthestPairFromList(edgeVertices, farthestPair);
				}

				/** Directly compare all vertex pairs to find the pair of vertices with the maximal separation. Typically slower than analyseShape() but
				  * faster for very thin, very long meshes (e.g. stitches). */
				float analyseShapeDirect(VertPair &farthestPair)
				{
					return findFarthestPairFromList(ve, farthestPair);
				}

				float findFarthestPairFromList(std::vector<xVert> &edgeVertices, VertPair &farthestPair)
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

				/** Delete a vertex. This function is in principle unsafe, may result in invalid meshes, and does not delete its adjacent polygons. */
				void delVertex(xVert j)
				{
					assert(j < ve.size());
					vertices[ve[j]] = vertices.back(); // copy last vertex to deleted vertex
					ve[vertices.back().index] = ve[j]; // delete last vertex
					vertices.pop_back(); // remove from vertex list
					ve[j] = 0; // remove from index list
				}

				/** Compare two polygons. Return 'true' if they contain the same vertices in the same (clockwise) order. */
				inline bool comparePolygons(xVert a, xVert b, Polygon & k)
				{
					if(a == k.a) return (b == k.b); // if a and b are the same vertices for both polygons, c must be as well
					else if(a == k.b) return (b == k.c);
					else if(a == k.c) return (b == k.a);
					else return false;
				}

				bool addPolygon(Vertex &a, Vertex &b, Vertex &c)
				{
					// check whether polygon exists (by using a's list)
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
						if(a.poly[i] > 0 && comparePolygons(a.index, b.index, polygons[po[a.poly[i]]])) return false; // Polygon found
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

				void printLists(void)
				{
					std::cout << " Printing MeshBundle lists: "<<std::endl;
					std::cout << " vertices: "; for(unsigned int i = 0; i < vertices.size(); i++) std::cout << i << ":"<<vertices[i].pos<<" (E="<<findEdgeVertex(vertices[i].index)<<"), "; std::cout << std::endl;
					std::cout << " vertex index: "; for(unsigned int i = 0; i < ve.size(); i++) std::cout << i << ":"<<ve[i]<<" @ "<<&vertices[ve[i]]<<", "; std::cout << std::endl;
					std::cout << " vertex check: "; for(unsigned int i = 0; i < ve.size(); i++) std::cout << i << ":"<<vertices[ve[i]].index<<", "; std::cout << std::endl;
					std::cout << " vertex polys: "<<std::endl;
					for(unsigned int i = 0; i < vertices.size(); i++)
					{
						std::cout << " vertex "<<i<<": index = "<<vertices[i].index<<", polys = ";
						for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++) std::cout << vertices[i].poly[j] << ", ";
						std::cout <<(isEdgeVertex(vertices[i].index)?"(E)":"")<< std::endl;
					}
				}

				void printPolygons(float step, unsigned int iterstep = 1)
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

				Mesh(core::intf::RenderInterface * _renderer) : MeshInterface(_renderer)
				{
					polygons.push_back( Polygon(0,0,0) );
					po.push_back(0); // po[0] shouldn't be used as a polygon because 0 is the "N/A" value for the Vertex's poly[] array
					vertices.push_back( VertexType(0.0f, 0.0f, 0.0f) );
					ve.push_back(0); // ve[0] shouldn't be used either because 0 is the "N/A" value for the Vertex's nextEdgeVertex variable.
				}

				virtual ~Mesh(void) { polygons.clear(); vertices.clear(); ve.clear(); po.clear(); }

				/** Find a neighbouring vertex based on an edge.
				  *
				  * This function returns the vertex on the other side of the first edge radiating away from 'v' in a (counter)clockwise fashion, measured starting from j.
				  * It does this by trying to find the highest dot (inner) product among all vectors with a positive (or negative, for counterclockwise) cross product.
				  * If no neighbor is found with an angle less than 90 degrees (measured from the v-j vector) then 0 is returned (the non-existing vertex).
				  *
				  * Note that length is ignored, the algorithm doesn't care much how far it has to look to find a vertex in the right direction. However, it's guaranteed that
				  * it's already connected by a polygon also connecting to j, so the resulting polygon won't be much bigger than one already existing.
				  */
				inline xVert findNeighborVertex(const Vertex &j, const Vertex & v, bool clockwise)
				{
					float bestInnerProd = 0.0f;
					xVert vert = 0;
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(v.poly[i]==0) break;
						else
						{
							Vertex & w = vertices[ve[ findPolyNeighbor(polygons[po[v.poly[i]]],v.index,clockwise) ]];
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
			private:
				/** Calculate the normal of a polygon. */
				tiny::vec3 computeNormal(xPoly _p)
				{
					return normalize(cross(vertices[ve[polygons[po[_p]].c]].pos - vertices[ve[polygons[po[_p]].a]].pos,
										   vertices[ve[polygons[po[_p]].b]].pos - vertices[ve[polygons[po[_p]].a]].pos));  // normal (use first poly's normal if available, otherwise use vertical)
				}

				inline bool isEdgeVertex(xVert _v)
				{
					Vertex & v = vertices[ve[_v]];
					if(v.poly[2] == 0) return true; // Vertices that connect to fewer than three polygons must be at the edge
					unsigned int verts[2*STRATA_VERTEX_MAX_LINKS];
					for(unsigned int i = 0; i < 2*STRATA_VERTEX_MAX_LINKS; i++) verts[i] = 0;
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(v.poly[i]==0) break;
						else
						{
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

				/** Find another edge vertex, starting from the current vertex. If the parameter 'clockwise' is true the mesh edge will be traversed in a
				  * clockwise fashion (looking from the direction of the normals, or typically, from above the terrain). */
				xVert findAdjacentEdgeVertex(xVert v, bool clockwise)
				{
					xVert result = 0;
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(vertices[ve[v]].poly[i] == 0) { std::cout << " findAdjacentEdgeVertex() : Failed to find next edge vertex! "<<std::endl; break; }
						else if( isEdgeVertex( findPolyNeighbor(polygons[po[vertices[ve[v]].poly[i]]], v, clockwise) ) )
						{
							result = findPolyNeighbor(polygons[po[vertices[ve[v]].poly[i]]], v, clockwise);
							break;
						}
					}
					return result;
				}

				xVert findRandomEdgeVertex(void)
				{
					unsigned int step = ve.size()/7 + 1;
					unsigned int startVertex = 1;
					unsigned int edgeVertex = (unsigned int)(-1);
					while(edgeVertex == (unsigned int)(-1))
					{
						startVertex = ((startVertex + step) % ve.size()); // take modulo to get next vertex
						if(startVertex != 0) edgeVertex = findEdgeVertex(startVertex);
					}
					return edgeVertex;
				}

				/** Find some vertex on the edge, starting at vertex v. This function may fail if it gets stuck in a highly deformed mesh, in this case it returns 0.
				  * This situation is best dealt with by trying to find an edge vertex but starting at another vertex randomly. */
				xVert findEdgeVertex(xVert _v)
				{
					if(_v == 0) { std::cout << " Mesh::findEdgeVertex() : Cannot find edge vertex starting from xVert error value 0! "<<std::endl; return (unsigned int)(-1); }
					if(isEdgeVertex(_v)) return _v;
					Vertex & v = vertices[ve[_v]];
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(v.poly[i] == 0) break;
						xVert w = findPolyNeighbor(polygons[po[v.poly[i]]], v.index, true); // Only need to consider one direction - the other vertex will be found in the neighbouring polygon for a non-edge vertex
						if(vertices[ve[w]].pos.x > v.pos.x) return findEdgeVertex(w);
					}
					std::cout << " Mesh::findEdgeVertex() : No edge vertex found! "<<std::endl;
					return (unsigned int)(-1);
				}

				/** Get a (non-normalized) normal vector for a polygon. */
				inline tiny::vec3 polyNormal(const Polygon &p) { return cross( vertices[ve[p.c]].pos-vertices[ve[p.a]].pos, vertices[ve[p.b]].pos-vertices[ve[p.a]].pos); }

				/** Find a neighbor vertex in a polygon. This function gives nonsensical output if you call it for a vertex not part of this polygon. */
				inline const xVert & findPolyNeighbor(const Polygon &p, const xVert &v, bool clockwise)
				{
					return (p.a == v ? (clockwise ? p.b : p.c) :
							(p.b == v ? (clockwise ? p.c : p.a) :
							 			 (clockwise ? p.a : p.b) ) );
				}
		};
	}
}
