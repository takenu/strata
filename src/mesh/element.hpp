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
			xVert a; /**< Vertices for the polygon, in a-b-c clockwise order. */
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

		/** A mesh, consisting of vertices and polygons that link the vertices together. It is used intensely as a container object so data is public.
		  * However, since some helper functions are never required as outside functions, they are hidden (as opposed to the data).
		  */
		class MeshBundle
		{
			public:
				std::vector<Vertex> vertices;
				std::vector<Polygon> polygons;

				std::vector<xVert> ve;
				std::vector<xPoly> po;

				long unsigned int polyAttempts;

				/** At construction, add error values for polygons and vertices. */
				MeshBundle(void) : polyAttempts(0) { vertices.push_back( Vertex(0.0f, 0.0f, 0.0f) ); polygons.push_back( Polygon(0,0,0) ); po.push_back(0); }

				void createFlatLayer(float size, unsigned int ndivs, float height = 0.0f);
				void createFlatLayerPolygon(std::deque<VertPair> &plist, xVert _a, xVert _b, float limit, float step);

				tiny::mesh::StaticMesh convertToMesh(float size);

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

				/** Delete a vertex. */
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
//					std::cout << " new polygon at vertices "<<&a<<", "<<&b<<", "<<&c<<std::endl; printLists();
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) if(a.poly[i] == 0) { a.poly[i] = po.size()-1; break; }
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) if(b.poly[i] == 0) { b.poly[i] = po.size()-1; break; }
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) if(c.poly[i] == 0) { c.poly[i] = po.size()-1; break; }
//					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) if(a.poly[i] == 0) { std::cout << " put poly["<<i<<"] to "<<po.size()-1<<" for vertex "<<a.index<<std::endl; a.poly[i] = po.size()-1; break; }
//					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) if(b.poly[i] == 0) { std::cout << " put poly["<<i<<"] to "<<po.size()-1<<" for vertex "<<b.index<<std::endl; b.poly[i] = po.size()-1; break; }
//					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) if(c.poly[i] == 0) { std::cout << " put poly["<<i<<"] to "<<po.size()-1<<" for vertex "<<c.index<<std::endl; c.poly[i] = po.size()-1; break; }
//					std::cout << " Created polygon: vertices="<<a.pos<<"->"<<b.pos<<"->"<<c.pos<<", indices "<<polygons.back()<<std::endl;
					return true;
				}

				void printLists(void)
				{
					std::cout << " Printing MeshBundle lists: "<<std::endl;
					std::cout << " vertices: "; for(unsigned int i = 0; i < vertices.size(); i++) std::cout << i << ":"<<vertices[i].pos<<", "; std::cout << std::endl;
					std::cout << " vertex index: "; for(unsigned int i = 0; i < ve.size(); i++) std::cout << i << ":"<<ve[i]<<" @ "<<&vertices[ve[i]]<<", "; std::cout << std::endl;
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
//						printDifferentials(vertices[ve[polygons[i].b]].pos,div); printDifferentials(vertices[ve[polygons[i].c]].pos,div);
						std::cout << std::endl;
					}
				}
			private:
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
//					std::cout << " findNeighborVertex() : finding neighbor for "<<j<<" and "<<v<<" in "<<(clockwise?"":"counter")<<"clockwise direction..."<<std::endl;
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(v.poly[i]==0) break;
						else
						{
							Vertex & w = vertices[ve[ findPolyNeighbor(polygons[po[v.poly[i]]],v.index,clockwise) ]];
							float innerProd = dot(j.pos - v.pos, normalize(w.pos - v.pos));
//							std::cout << " poly "<<i<<": vertex="<<w.index<<" third="<<findPolyNeighbor(polygons[po[v.poly[i]]],v.index,!clockwise)<<" dotpr="<<innerProd
//								<<" dotcr="<<dot(cross( w.pos - v.pos, j.pos - v.pos ),polyNormal(polygons[po[v.poly[i]]]) )<<std::endl;
							if(innerProd > bestInnerProd && w.index != j.index) // skip j itself, it can show up if another polygon already exists on the other side
							{
								if( (dot(cross( w.pos - v.pos, j.pos - v.pos ),polyNormal(polygons[po[v.poly[i]]]) ) < 0.0f) != clockwise ) // note the inequality on two bools to generate XOR-like behavior
								{
//									std::cout << " j="<<j.index<<", v="<<v.index<<", w="<<w.index<<", innerprod="<<innerProd<<" cross="<<cross(w.pos-v.pos, j.pos-v.pos)<<"norm="<<polyNormal(polygons[po[v.poly[i]]])<<std::endl;
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
