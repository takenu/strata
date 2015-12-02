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

using namespace strata::mesh;

/** Split the Strip object. This function is roughly similar to the Bundle implementation for splitting Bundles,
  * but it is simpler since the Strip is only split into two Strips that are each roughly half as long as the
  * original Strip. */
bool Strip::split(std::function<Bundle * (void)>, std::function<Strip * (void)> makeNewStrip)
{
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
//	std::cout << " Split mesh into strips with "<<f->polygons.size()<<" and "<<g->polygons.size()<<" polys. "<<std::endl;

	f->setScaleFactor(scaleTexture);
	g->setScaleFactor(scaleTexture);

	f->initMesh();
	g->initMesh();
	f->resetTexture(scaleTexture, 250, 100, 0);
	g->resetTexture(scaleTexture, 250, 200, 100);

	// Copy the references to all adjacent meshes of 'this', when required.
	// This copying is done both ways: the adjacent mesh is added to the newly added one,
	// and the newly added one is added to the adjacent mesh.
//	duplicateAdjacentMeshes(f);
//	duplicateAdjacentMeshes(g);
	std::cout << " Strip::split() : Copying adjacent bundles, there are "<<adjacentBundles.size()<<" of these... "<<std::endl;
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

//	f->addAdjacentMesh(g); <-- Not necessary, Strips do not use vertices from other Strips but only from Bundles
//	g->addAdjacentMesh(f);
	return true;
}

bool Strip::isAdjacentToVertices(const Bundle * b) const
{
	for(unsigned int i = 1; i < vertices.size(); i++)
		if(vertices[i].getOwningBundle() == b) return true;
	return false;
}

Strip::~Strip(void)
{
	for(unsigned int i = 0; i < adjacentBundles.size(); i++)
		assert(adjacentBundles[i]->releaseAdjacentStrip(this));
}

bool Strip::checkAdjacentMeshes(void) const
{
	bool adjacentMeshesAreComplete = true;
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
		else ++(adjacentBundleRefs.at(vertices[i].getOwningBundle()));
		if(!(vertices[i].getOwningBundle()->isValidVertexIndex(vertices[i].getRemoteIndex())))
		{
			std::cout << " Strip::checkAdjacentMeshes() :";
			std::cout << " Vertex "<<i<<" with remote index "<<vertices[i].getRemoteIndex()<<" refers to Bundle without reverse link!"<<std::endl;
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
