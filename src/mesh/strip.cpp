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

#include "strip.hpp"
#include "bundle.hpp"
#include "layer.hpp"

using namespace strata::mesh;

/** Split the Strip object. This function is roughly similar to the Bundle implementation for splitting Bundles,
  * but it is simpler since the Strip is only split into two Strips that are each roughly half as long as the
  * original Strip. */
bool Strip::split(std::function<Bundle * (void)>, std::function<Strip * (void)> makeNewStrip)
{
	if(isStitch)
	{
		// TODO: It should be possible to split Stitches, but it won't work right now because
		// we would need to access makeNewStitch(), not makeNewStrip(). There are several
		// solutions possible (e.g. passing that function from Terrain to here, or using a
		// specialized split function to be called by the Terrain directly).
		// Perhaps better, one could call this with makeNewStitch() from the Terrain, but then
		// that function would need to have 0 arguments, and isTransverseStitch would have
		// to become mutable.
		std::cout << " Strip::split() : Stitch Strips cannot be split yet. "<<std::endl;
		return false;
	}

	Strip * f = 0;
	Strip * g = 0;

	std::map<xVert,xVert> fvert,gvert;
	
	splitMesh(makeNewStrip, f, g, fvert, gvert);
	if(f==0 || g==0) { std::cout << " Strip::split() : New Strips do not exist, splitting aborted. "<<std::endl; return false; }

	if(vertices.size()+1 > f->vertices.size() + g->vertices.size())
	{
		std::cout << " Strip::split() : ERROR: Not all vertices were assigned a Bundle! "<<std::endl;
		std::cout << " Mappings fvert = "; for(std::map<xVert, xVert>::iterator it = fvert.begin(); it != fvert.end(); it++) std::cout <<" "<<it->first<<"->"<<it->second; std::cout << std::endl;
		std::cout << " Mappings gvert = "; for(std::map<xVert, xVert>::iterator it = gvert.begin(); it != gvert.end(); it++) std::cout <<" "<<it->first<<"->"<<it->second; std::cout << std::endl;
	}

	splitAssignPolygonsToConstituentMeshes(f,g,g,fvert,gvert);

	f->setScaleFactor(scaleTexture);
	g->setScaleFactor(scaleTexture);

	f->setParentLayer(parentLayer);
	g->setParentLayer(parentLayer);

	f->resetTexture(parentLayer->getStripTexture());
	g->resetTexture(parentLayer->getStripTexture());

	// Copy the references to all adjacent meshes of 'this', when required.
	// This copying is done both ways: the adjacent mesh is added to the newly added one,
	// and the newly added one is added to the adjacent mesh.
	for(unsigned int i = 0; i < adjacentBundles.size(); i++)
	{
		// Copy adjacent bundles where required.
		if(f->isAdjacentToVertices(adjacentBundles[i]))
		{
			f->addAdjacentBundle(adjacentBundles[i]);
			adjacentBundles[i]->addAdjacentStrip(f);
		}
		if(g->isAdjacentToVertices(adjacentBundles[i]))
		{
			g->addAdjacentBundle(adjacentBundles[i]);
			adjacentBundles[i]->addAdjacentStrip(g);
		}
	}

	if(f->vertices.size() < 3) std::cout << " Strip::split() : WARNING: Strip "<<f<<" cannot have polygons! "<<std::endl;
	if(g->vertices.size() < 3) std::cout << " Strip::split() : WARNING: Strip "<<g<<" cannot have polygons! "<<std::endl;

	return true;
}

tiny::mesh::StaticMesh Strip::convertToMesh(void) const
{
	tiny::mesh::StaticMesh mesh;
	for(unsigned int i = 1; i < vertices.size(); i++)
		mesh.vertices.push_back( tiny::mesh::StaticMeshVertex(
				tiny::vec2(vertices[i].pos.z/scaleTexture + 0.5, vertices[i].pos.x/scaleTexture + 0.5), // texture coordinate
				tiny::vec3(1.0f,0.0f,0.0f), // tangent (appears to do nothing)
				(vertices[i].poly[0] > 0 ? computeNormal(vertices[i].poly[0]) : tiny::vec3(0.0f,1.0f,0.0f)),
				vertices[i].getPosition() ) ); // position
	for(unsigned int i = 1; i < polygons.size(); i++)
	{
		mesh.indices.push_back( ve[polygons[i].c] - 1 ); // -1 because vertices[0] is the error value and the mesh doesn't have that so it's shifted by 1
		mesh.indices.push_back( ve[polygons[i].b] - 1 ); // Note that we add polygons in reverse order because OpenGL likes them counterclockwise while we store them clockwise
		mesh.indices.push_back( ve[polygons[i].a] - 1 );
	}

	return mesh;
}

void Strip::recalculateVertexPositions(void)
{
	for(unsigned int i = 1; i < vertices.size(); i++)
	{
		vertices[i].pos = vertices[i].getOwningBundle()->getVertexPositionFromIndex(vertices[i].getRemoteIndex());
		if(vertices[i].isStitchVertex())
			vertices[i].setSecondaryPos(vertices[i].getSecondaryBundle()->getVertexPositionFromIndex(vertices[i].getSecondaryIndex()));
	}
}

void Strip::duplicateStrip(Strip * s) const
{
	if(s->vertices.size() != 1 || s->polygons.size() != 1)
	{
		std::cout << " Strip::duplicateStrip() : ERROR: Cannot duplicate Strip, target is not ready for copying vertices and polygons!"<<std::endl;
		return;
	}
	duplicateMesh(s);
	for(unsigned int i = 0; i < adjacentBundles.size(); i++)
		s->addAdjacentBundle(adjacentBundles[i]);
}

bool Strip::isAdjacentToVertices(const Bundle * b) const
{
	for(unsigned int i = 1; i < vertices.size(); i++)
		if(vertices[i].getOwningBundle() == b
				|| vertices[i].getSecondaryBundle() == b) return true;
	return false;
}

/** Find 'a' in the following situation (clockwise=true)
  * w---a
  *  \ /
  *   v
  * where 'v' and 'w' are remote indices rather than normal indices.
  * If not found, return 0.
  * This function doesn't make sense for Stitch Strips so there is no attempt
  * to include secondary Bundles.
  */
xVert Strip::findRemoteVertexPolyNeighbor(Bundle * &neighborBundle, xVert v, xVert w,
		const Bundle * vBundle, const Bundle * wBundle, bool clockwise)
{
	xVert remoteNeighborIndex = 0;
	xVert vLocal = 0;
	xVert wLocal = 0;
	for(unsigned int i = 1; i < vertices.size(); i++)
		if(vertices[i].getRemoteIndex() == v && vertices[i].getOwningBundle() == vBundle)
		{
			vLocal = vertices[i].index;
			break;
		}
	for(unsigned int i = 1; i < vertices.size(); i++)
		if(vertices[i].getRemoteIndex() == w && vertices[i].getOwningBundle() == wBundle)
		{
			wLocal = vertices[i].index;
			break;
		}
	if(vLocal == 0 || wLocal == 0)
	{
		// This can happen naturally since this function is called a lot on Strips that don't
		// have the desired vertices. So don't print.
//		std::cout << " Strip::findRemoteVertexPolyNeighbor() : Either 'v' or 'w' is not"
//			<< " represented in the Strip mesh, cannot search for neighbors! "<<std::endl;
	}
	else
	{
		xVert localNeighbor = findPolyNeighborFromVertexPair(vLocal, wLocal);
		if(localNeighbor > 0)
		{
//			std::cout << " Strip::fRVPN() : Found poly "<<vertices[ve[vLocal]].pos<<", "
//				<< vertices[ve[wLocal]].pos<<", "<<vertices[ve[localNeighbor]].pos<<std::endl;
			remoteNeighborIndex = vertices[ve[localNeighbor]].getRemoteIndex();
			neighborBundle = vertices[ve[localNeighbor]].getOwningBundle();
		}
		// This also may happen, if the neighbor is not in this Strip but in another Strip that
		// also contains the vertex 'v'.
//		else std::cout << " Strip::findRemoteVertexPolyNeighbor() : Not found! "<<std::endl;
	}
	return remoteNeighborIndex;
}

Strip::~Strip(void)
{
	for(unsigned int i = 0; i < adjacentBundles.size(); i++)
		assert(adjacentBundles[i]->releaseAdjacentStrip(this));
}

// TODO: Add secondary Bundle to below function for checking Stitch meshes as well.
bool Strip::checkAdjacentMeshes(void) const
{
	bool adjacentMeshesAreComplete = true;
	if(getParentLayer() == 0)
	{
		std::cout << " Strip::checkAdjacentMeshes() : Parent layer not set! "<<std::endl;
		adjacentMeshesAreComplete = false;
	}
	std::map<const Bundle*, unsigned int> adjacentBundleRefs;
	for(unsigned int i = 0; i < adjacentBundles.size(); i++)
		adjacentBundleRefs.emplace(adjacentBundles[i], 0);
	for(unsigned int i = 1; i < vertices.size(); i++)
	{
		if(!(isAdjacentToBundle(vertices[i].getOwningBundle())))
		{
			std::cout << " Strip::checkAdjacentMeshes() : in Strip "<<this<<",";
			std::cout << " Vertex "<<i<<" with remote index "<<vertices[i].getRemoteIndex()<<" refers to unknown Bundle "<<vertices[i].getOwningBundle()
				<<" in list of size "<<adjacentBundles.size()<<"!"<<std::endl;
			adjacentMeshesAreComplete = false;
		}
		else if(vertices[i].isStitchVertex() && !(isAdjacentToBundle(vertices[i].getSecondaryBundle())))
		{
			std::cout << " Strip::checkAdjacentMeshes() : in Strip "<<this<<",";
			std::cout << " Stitch Vertex "<<i<<" with remote index "<<vertices[i].getSecondaryIndex()<<" refers to unknown Bundle "<<vertices[i].getSecondaryBundle()
				<<" in list of size "<<adjacentBundles.size()<<"!"<<std::endl;
			adjacentMeshesAreComplete = false;
		}
		else
		{
			++(adjacentBundleRefs.at(vertices[i].getOwningBundle()));
			if(vertices[i].isStitchVertex())
					++(adjacentBundleRefs.at(vertices[i].getSecondaryBundle()));
		}
		if(!(vertices[i].getOwningBundle()->isValidVertexIndex(vertices[i].getRemoteIndex())))
		{
			std::cout << " Strip::checkAdjacentMeshes() :";
			std::cout << " Vertex "<<i<<" with remote index "<<vertices[i].getRemoteIndex()<<" refers to Bundle without reverse link!"<<std::endl;
			adjacentMeshesAreComplete = false;
		}
		else if(vertices[i].isStitchVertex() && !(vertices[i].getSecondaryBundle()->isValidVertexIndex(vertices[i].getSecondaryIndex())))
		{
			std::cout << " Strip::checkAdjacentMeshes() :";
			std::cout << " Stitch Vertex "<<i<<" with remote index "<<vertices[i].getSecondaryIndex()<<" refers to Bundle without reverse link!"<<std::endl;
			adjacentMeshesAreComplete = false;
		}
		else if( tiny::length(vertices[i].pos - vertices[i].getOwningBundle()->getVertexPositionFromIndex(vertices[i].getRemoteIndex())) > 0.01)
		{
			std::cout << " Strip::checkAdjacentMeshes() :";
			std::cout << " Vertex "<<i<<" has position "<<vertices[i].pos<<" but remote vertex "<<vertices[i].getRemoteIndex()<<" has position "
				<< vertices[i].getOwningBundle()->getVertexPositionFromIndex(vertices[i].getRemoteIndex())<<"!"<<std::endl;
			adjacentMeshesAreComplete = false;
		}
		else if(vertices[i].isStitchVertex() && tiny::length(vertices[i].getSecondaryPos() - vertices[i].getSecondaryBundle()->getVertexPositionFromIndex(vertices[i].getSecondaryIndex())) > 0.01)
		{
			std::cout << " Strip::checkAdjacentMeshes() :";
			std::cout << " Stitch Vertex "<<i<<" has position "<<vertices[i].getSecondaryPos()<<" but remote vertex "<<vertices[i].getSecondaryIndex()<<" has position "
				<< vertices[i].getSecondaryBundle()->getVertexPositionFromIndex(vertices[i].getSecondaryIndex())<<"!"<<std::endl;
			adjacentMeshesAreComplete = false;
		}
		else if(vertices[i].isStitchVertex() && (vertices[i].getOwningBundle()->getParentLayer() == getParentLayer()
					|| vertices[i].getSecondaryBundle()->getParentLayer() == getParentLayer()) )
		{
			// NOTE: This check may eventually fail 'legally' when a layer faults and parts of it are deposited on top of itself.
			// However, since this kind of topology initially is not to be expected, the check should pass. Later on
			// this can be removed, when it becomes possible for the Terrain to shift fragments of Layers on top of itself.
			std::cout << " Strip::checkAdjacentMeshes() :";
			std::cout << " Stitch Vertex "<<i<<" is stitching the Layer onto itself! "<<std::endl;
			adjacentMeshesAreComplete = false;
		}
	}
	for(unsigned int i = 1; i < polygons.size(); i++)
	{
		if(	vertices[ve[polygons[i].a]].getOwningBundle() == vertices[ve[polygons[i].b]].getOwningBundle() &&
			vertices[ve[polygons[i].a]].getOwningBundle() == vertices[ve[polygons[i].c]].getOwningBundle())
		{
			std::cout << " Strip::checkAdjacentMeshes() :";
			std::cout << " Polygon "<<i<<" has three vertices from the same Bundle!"<<std::endl;
			adjacentMeshesAreComplete = false;
		}
		else if( isStitch && !(vertices[ve[polygons[i].a]].isStitchVertex() || vertices[ve[polygons[i].b]].isStitchVertex()
					|| vertices[ve[polygons[i].c]].isStitchVertex()))
		{
			std::cout << " Strip::checkAdjacentMeshes() :";
			std::cout << " Polygon "<<i<<" is in a Stitch Strip but has no Stitch vertices!"<<std::endl;
			adjacentMeshesAreComplete = false;
		}
	}
	for(std::map<const Bundle*, unsigned int>::iterator it = adjacentBundleRefs.begin(); it != adjacentBundleRefs.end(); it++)
	{
		if(it->second == 0)
		{
			std::cout << " Strip::checkAdjacentMeshes() : Bundle is adjacent to strip but is never referenced! "<<std::endl;
			adjacentMeshesAreComplete = false;
		}
		if(!(it->first->isAdjacentToStrip(this)))
		{
			std::cout << " Strip::checkAdjacentMeshes() : Bundle does not contain a reverse reference to a Strip! "<<std::endl;
			adjacentMeshesAreComplete = false;
		}
	}
	return adjacentMeshesAreComplete;
}
