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

#include <tiny/math/vec.h>
#include <tiny/mesh/staticmesh.h>

#include "vecmath.hpp"

#define STRATA_VERTEX_MAX_LINKS 12

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
			xPoly poly[STRATA_VERTEX_MAX_LINKS]; /**< Enforce max number of links (to avoid having to (de)allocate memory when creating a Vertex). */

			Vertex(const tiny::vec3 &p) : pos(p), index(0) { for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) poly[i] = 0; }
			Vertex(float x, float y, float z) : pos(x,y,z), index(0) { for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) poly[i] = 0; }

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

		/** A helper struct for keeping a list of vertex pairs. */
		struct VertPair
		{
			xVert a;
			xVert b;

			VertPair(xVert _a, xVert _b) : a(_a), b(_b) {}
		};

		/** The Mesh is a base class for objects that contain parts of the terrain as a set of vertices connected via polygons.
		  * The VertexType is a type that represents a point in space. It should derive from the Vertex struct, or be a Vertex.
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
		class Mesh
		{
			public:
				std::vector<VertexType> vertices;
				std::vector<Polygon> polygons;

				std::vector<xVert> ve;
				std::vector<xPoly> po;

				float scale; /**< Scale factor - coordinates should range from -scale/2 to scale/2 (used for texture coords) */

				/** Convert the Mesh to a StaticMesh object. */
				tiny::mesh::StaticMesh convertToMesh(void)
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

				/** Find a vertex on the edge. */
				xVert findEdgeVertex(void)
				{
					xVert edgeVertex = 0;
//					for(unsigned int i = 0; i < vertices.size(); i++)
					return edgeVertex;
				}

				/** Analyse the shape of the mesh, and set the vertices _a and _b to the pair of most distant vertices in the set. This only considers edge vertices (such
				  * that the calculation is easiest, also because for most sane meshes edge vertices are most distant, and because it is easier to work with edge
				  * vertices when e.g. splitting meshes). */
				void analyseShape(VertexType &_a, VertexType &_b)
				{
				}

				/** Add a vertex and return the xVert reference to that vertex. */
				xVert addVertex(const Vertex &v)
				{
					ve.push_back( vertices.size() );
					vertices.push_back(v);
					vertices.back().index = ve.size()-1;
					return ve.size()-1;
				}
				xVert addVertex(tiny::vec3 &p) { return addVertex( Vertex(p) ); }
				xVert addVertex(float x, float y, float z) { return addVertex( Vertex(tiny::vec3(x,y,z)) ); }

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
					std::cout << " vertices: "; for(unsigned int i = 0; i < vertices.size(); i++) std::cout << i << ":"<<vertices[i].pos<<", "; std::cout << std::endl;
					std::cout << " vertex index: "; for(unsigned int i = 0; i < ve.size(); i++) std::cout << i << ":"<<ve[i]<<" @ "<<&vertices[ve[i]]<<(isEdgeVertex(i)?"(E)":"")<<", "; std::cout << std::endl;
					std::cout << " vertex check: "; for(unsigned int i = 0; i < ve.size(); i++) std::cout << i << ":"<<vertices[ve[i]].index<<", "; std::cout << std::endl;
					std::cout << " vertex polys: "<<std::endl;
					for(unsigned int i = 0; i < vertices.size(); i++)
					{
						std::cout << " vertex "<<i<<": index = "<<vertices[i].index<<", polys = ";
						for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++) std::cout << vertices[i].poly[j] << ", ";
						std::cout << std::endl;
					}
				}

				void printDifferentials(const tiny::vec3 & v, float div)
				{
					float dummy;
					std::cout << "(" << std::modf(v.x*div*sqrt(3.0f),&dummy)<<","<<std::modf(v.y*div, &dummy)<<","<<std::modf(v.z*div, &dummy)<<"), ";
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
			protected:
				Mesh(void)
				{
					polygons.push_back( Polygon(0,0,0) );
					po.push_back(0); // po[0] shouldn't be used as a polygon because 0 is the "N/A" value for the Vertex's poly[] array
				}

				virtual ~Mesh(void) { polygons.clear(); vertices.clear(); ve.clear(); po.clear(); }

				/** A function to split large meshes. */
//				virtual void splitMesh(void) = 0;
				/** Find a neighbouring vertex based on a position. If the vertex exists and a link exists between it and v, then a
				  * polygon should exist which has both v and the desired vertex. It compares vertices with a squared-diff tolerance 'eps'.
				  *
				  * This function risks having to do an average of up to 18 (6polys*3verts) deep lookups (which go through 4 large std::vector's).
				  * It remains to be seen whether it is fast enough. However, it's only to be used in constructing new layers, not in modifying
				  * existing ones.
				  */
				inline xVert findNeighbor(const tiny::vec3 &p, const Vertex & v, float eps = 0.0001f)
				{
					xVert x = 0;
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(v.poly[i]==0) break;
						else
						{
							if( tiny::length2( vertices[ve[ polygons[po[v.poly[i]]].a ]].pos - p ) < eps ) {x = polygons[po[v.poly[i]]].a; break;}
							if( tiny::length2( vertices[ve[ polygons[po[v.poly[i]]].b ]].pos - p ) < eps ) {x = polygons[po[v.poly[i]]].b; break;}
							if( tiny::length2( vertices[ve[ polygons[po[v.poly[i]]].c ]].pos - p ) < eps ) {x = polygons[po[v.poly[i]]].c; break;}
							if( tiny::length2( vertices[ve[ polygons[po[v.poly[i]]].a ]].pos - p ) < 0.01f )
							{
								std::cout << " unexpected neighbor pos at "<<vertices[ve[ polygons[po[v.poly[i]]].a ]].pos<<" far from "<<p<<std::endl;
								x = polygons[po[v.poly[i]]].a; break;
							}
						}
					}
					if(x != 0) vertices[ve[x]].pos = (vertices[ve[x]].pos + p)*0.5; // Use mixing: if position slightly deviates from expected, adjust to middle
					return x;
				}

				/** Get a (non-normalized) normal vector for a polygon. */
				inline tiny::vec3 polyNormal(const Polygon &p) { return cross( vertices[ve[p.c]].pos-vertices[ve[p.a]].pos, vertices[ve[p.b]].pos-vertices[ve[p.a]].pos); }

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
