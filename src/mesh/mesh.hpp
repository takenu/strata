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
#include "toplmesh.hpp"

namespace strata
{
	namespace mesh
	{
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
		class Mesh : public TopologicalMesh<VertexType>
		{
			public:
				/** Add a vertex and return the xVert reference to that vertex. Note that careless construction of meshes will likely
				  * result in invalid meshes, this function should only be used if one ensures that all vertices end up being properly
				  * linked into a mesh (without holes or bottlenecks) by polygons.*/
				xVert addVertex(const VertexType &v)
				{
					ve.push_back( vertices.size() );
					vertices.push_back(v);
					vertices.back().clearPolys(); // The vertex should not use the polygons from the original copy (if any)
					vertices.back().index = ve.size()-1;
					return ve.size()-1;
				}
			protected:
				using TopologicalMesh<VertexType>::vertices;
				using TopologicalMesh<VertexType>::polygons;
				using TopologicalMesh<VertexType>::ve;
				using TopologicalMesh<VertexType>::po;

				using TopologicalMesh<VertexType>::comparePolygons;

				Mesh(core::intf::RenderInterface * _renderer) :
					TopologicalMesh<VertexType>(_renderer)
				{
				}

				/** Re-declare pure virtual function purgeVertex, originally from the MeshInterface. */
				virtual void purgeVertex(long unsigned int mfid, xVert oldVert, xVert newVert) = 0;

				/** Delete a vertex. This function is in principle unsafe, may result in invalid meshes, and does not delete its adjacent polygons. */
				void delVertex(xVert j)
				{
					assert(j < ve.size());
					vertices[ve[j]] = vertices.back(); // copy last vertex to deleted vertex
					ve[vertices.back().index] = ve[j]; // delete last vertex
					vertices.pop_back(); // remove from vertex list
					ve[j] = 0; // remove from index list
				}

				/** Add a vertex v if it isn't added already. The tolerance determines the maximal difference between v's position
				  * and an existing vertex's position for which v is considered 'already present' in the mesh. */
				xVert addIfNewVertex(const VertexType &v, float tolerance)
				{
					for(unsigned int i = 0; i < vertices.size(); i++)
						if( tiny::length2(v.pos - vertices[i].pos) < tolerance*tolerance ) return vertices[i].index;
					return addVertex(v);
				}

				/** Add a polygon using vertex indices rather than vertex references. */
				bool addPolygonFromVertexIndices(xVert _a, xVert _b, xVert _c)
				{
					return addPolygon(vertices[ve[_a]], vertices[ve[_b]], vertices[ve[_c]]); // Add polygon using vertices from this mesh.
				}

				/** Add a polygon between vertices a, b and c to the Mesh. Returns true if and only if the polygon was added. Checks for prior
				  * existence of the polygon so that duplicates are avoided. */
				bool addPolygon(VertexType &a, VertexType &b, VertexType &c)
				{
					// check whether polygon exists (by using a's list)
					for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
					{
						if(a.poly[i] >= po.size())
						{
							std::cout << " Mesh::addPolygon() : Bad poly array for vertex "<<a.index<<" in mesh with "<<po.size()<<" polygons: " << std::endl;
							for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++) std::cout << " poly["<<j<<"] = "<<a.poly[j]<<std::endl;
						}
						assert(a.poly[i] < po.size());
						assert(po[a.poly[i]] < polygons.size());
						if(a.poly[i] > 0 && comparePolygons(a.index, b.index, polygons[po[a.poly[i]]])) return false; // Polygon found
					}
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
		};
	} // end namespace mesh
} // end namespace strata