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

#include <cassert>
#include <limits>

#include <tiny/math/vec.h>

namespace strata
{
	namespace mesh
	{
		inline tiny::vec3 cross(const tiny::vec3 &a, const tiny::vec3 &b)
		{
//			return tiny::vec3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
			return tiny::cross(a,b);
		}

		inline float dot(const tiny::vec3 &a, const tiny::vec3 &b) { return tiny::dot(a,b); }

		/** Rotate a vector around another vector.
		  * Origin: http://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula */
		inline tiny::vec3 rot(const tiny::vec3 &v, tiny::vec3 axis, float angle)
		{
			if( std::fabs(length2(axis)-1.0f)>0.001f) normalize(axis);
			return (v*cos(angle) + cross(axis,v)*sin(angle) + axis*dot(axis,v)*(1.0f-cos(angle)));
		}

		/** Rotation around the y axis (special case of 'rot'), in a right-handed coordinate system. */
		inline tiny::vec3 roty(const tiny::vec3 &v, float angle)
		{
			return tiny::vec3( v.x*cos(angle) + v.z*sin(angle), v.y, v.z*cos(angle) - v.x*sin(angle) );
		}

		inline void testMathRelations(void)
		{
			tiny::vec3 v(1.0f,	0.333f,	0.71f);
			tiny::vec3 y(0.0f,	1.0f,	0.0f);
			float angle = 1.353;
			assert( length2(roty(v,angle) - rot(v,y,angle)) < 0.00001f ); // roty is special case of rot
		}

		/** Calculate the horizontal separation between two points. The convention, as usual, is that
		  * the y coordinate is the vertical coordinate - consequently this coordinate is not involved
		  * in this distance calculation. */
		inline float calcHorizontalSeparation(const tiny::vec3 &a, const tiny::vec3 &b)
		{
			return sqrt( (a.x-b.x)*(a.x-b.x) + (a.z-b.z)*(a.z-b.z) );
		}

		/** Calculate the distance between two points in space. */
		inline float dist(const tiny::vec3 &a, const tiny::vec3 &b)
		{
			return sqrt( (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y) + (a.z-b.z)*(a.z-b.z) );
		}

		/** Find the intersection between a line and a plane. In this function we use the following convention:
		  * - The line is defined as (p + x v), in other words the line formed by all points found by adding a
		  *   multiple of the vector v to the position p.
		  * - The plane is defined by the normal n and some random point z that lies on the plane.
		  *
		  * The vectors 'v' and 'n' do not need to be normalized.
		  *
		  * Note that if the line is nearly parallel to the plane the point of intersection could be ridiculously
		  * far away. In order to avoid division by zero, we then opt to return the zero vector as intersection point
		  * instead of some arbitrary point far away.
		  *
		  * The formula used is:
		  * s_I = p + ( n.(z-p) / n.v ) * v
		  * where '.' denotes the dot product.
		  * Origin: https://en.wikipedia.org/wiki/Line-plane_intersection
		  */
		inline tiny::vec3 findIntersection(tiny::vec3 p, tiny::vec3 v, tiny::vec3 z, tiny::vec3 n)
		{
			float dot1 = dot(n, z-p);
			float dot2 = dot(n, v);
			// Very small numerators mean near-parallel lines - in this case numerical precision is poor. Then return the zero vector. */
			if(dot2/dot1 < std::numeric_limits<float>::epsilon()) return tiny::vec3(0.0f, 0.0f, 0.0f);
			else return (p + v*(dot1/dot2));
		}
	}
}

