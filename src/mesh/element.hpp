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
		  * - The VertexType is a class that represents a point in space.
		  * - The PolygonType is a class that represents a surface in space, determined by an ordered set of vertices (normally three).
		  */
		template <typename VertexType, typename PolygonType>
		class Mesh
		{
			public:
				std::vector<VertexType> vertices;
				std::vector<PolygonType> polygons;

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
			protected:
				Mesh(void) {}
				virtual ~Mesh(void) { polygons.clear(); vertices.clear(); ve.clear(); po.clear(); }
		};
	}
}
