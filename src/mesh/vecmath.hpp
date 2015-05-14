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

#include <cassert>

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
	}
}

