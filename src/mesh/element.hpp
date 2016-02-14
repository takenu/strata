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

#include <tiny/math/vec.h>

#define STRATA_VERTEX_MAX_LINKS 10 /**< The maximal number of links allowed for any vertex. */
#define STRATA_VERTEX_LINK_THRESHOLD 8 /** The threshold after which attempts should be made to reduce the number of links of a vertex. */

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
			xVert index; /**< The index of this vertex in the 've' array of the Mesh. Uses '0' as an error value (valid vertices should not have index==0). */
			xVert nextEdgeVertex; /**< The next edge vertex, if this vertex itself is on the edge of a mesh. Otherwise 0. */
			float thickness; /**< The thickness of the layer, between 0 and 1, as a fraction of the original thickness of the layer. */
			xPoly poly[STRATA_VERTEX_MAX_LINKS]; /**< Enforce max number of links (to avoid having to (de)allocate memory when creating a Vertex). */

			Vertex(const tiny::vec3 &p) : pos(p), index(0), nextEdgeVertex(0), thickness(1.0f)
			{
				clearPolys();
			}

			Vertex(float x, float y, float z) : Vertex(tiny::vec3(x,y,z)) {}

			Vertex & operator= (const Vertex &v) { pos = v.pos; index = v.index; thickness = v.thickness; for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++) poly[i] = v.poly[i]; return *this; }

			/** Remove all polygon memberships from the Vertex (required e.g. when creating a duplicate of a Vertex) */
			void clearPolys(void)
			{
				for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					poly[i] = 0;
			}

			/** Count the number of polygons this Vertex is part of. */
			inline unsigned int nPolys(void) const
			{
				for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					if(poly[i] == 0) return i;
				return STRATA_VERTEX_MAX_LINKS;
			}

			/** Set the position of the Vertex. (The variable 'pos' is public at this time,
			  * but it may still be desirable to use a function rather than simple assignment.) */
			inline void setPosition( tiny::vec3 _p )
			{
				pos = _p;
			}
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
	}
}
