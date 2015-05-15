/*
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

		inline std::ostream & operator<< (std::ostream &s, const Polygon &p) { s << "("<<p.a<<","<<p.b<<","<<p.c<<")"; return s; }

		/** A helper struct for keeping a list of vertex pairs. */
		struct VertPair
		{
			xVert a;
			xVert b;

			VertPair(xVert _a, xVert _b) : a(_a), b(_b) {}
		};

		/** A mesh, consisting of vertices and polygons that link the vertices together. */
		class MeshBundle
		{
			public:
				std::vector<Vertex> vertices;
				std::vector<Polygon> polygons;

				std::vector<xVert> ve;
				std::vector<xPoly> po;

				long unsigned int polyAttempts;

				/** At construction, add error values for polygons and vertices. */
				MeshBundle(void) : polyAttempts(0) { vertices.push_back( Vertex(0.0f, 0.0f, 0.0f) ); polygons.push_back( Polygon(0,0,0) ); }

				void createFlatLayer(float size, unsigned int ndivs, float height = 0.0f);
				void createFlatLayerPolygon(std::deque<VertPair> &plist, xVert _a, xVert _b, float limit, float step);

				tiny::mesh::StaticMesh convertToMesh(float size);

				/** Add a vertex and return the xVert reference to that vertex. */
				xVert addVertex(const Vertex &v) { ve.push_back( vertices.size() ); vertices.push_back(v); vertices.back().index = ve.size()-1; return ve.size()-1; }
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

				/** Find a neighbouring vertex based on an edge.
				  *
				  * This function returns the vertex on the other side of the first edge radiating away from 'v' in a clockwise fashion, measured starting from j.
				  */
/*				inline Vertex & findNeighborVertex(const tiny::vec3 &p, const Vertex & v, bool clockwise)
				{
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(v.poly[i]==0) break;
						else
						{
				}*/

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
//					std::cout << " Created polygon: vertices="<<a.pos<<"->"<<b.pos<<"->"<<c.pos<<std::endl;
					return true;
				}

				void printLists(void)
				{
					std::cout << " Printing MeshBundle lists: "<<std::endl;
					std::cout << " vertices: "; for(unsigned int i = 0; i < vertices.size(); i++) std::cout << i << ":"<<vertices[i].pos<<", "; std::cout << std::endl;
					std::cout << " vertex index: "; for(unsigned int i = 0; i < ve.size(); i++) std::cout << i << ":"<<ve[i]<<", "; std::cout << std::endl;
					std::cout << " vertex check: "; for(unsigned int i = 0; i < ve.size(); i++) std::cout << i << ":"<<vertices[ve[i]].index<<", "; std::cout << std::endl;
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
						printDifferentials(vertices[ve[polygons[i].b]].pos,div); printDifferentials(vertices[ve[polygons[i].c]].pos,div);
						std::cout << std::endl;
					}
				}
		};
	}
}
