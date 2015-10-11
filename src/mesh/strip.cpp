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

using namespace strata::mesh;

/** Split the Strip object. This function is roughly similar to the Bundle implementation for splitting Bundles,
  * but it is simpler since the Strip is only split into two Strips that are each roughly half as long as the
  * original Strip. */
void Strip::split(std::function<Bundle * (void)>, std::function<Strip * (void)> makeNewStrip)
{
	Strip * f = 0;
	Strip * g = 0;

	std::map<xVert,xVert> fvert,gvert;
	
	splitMesh(makeNewStrip, f, g, fvert, gvert);
	if(f==0 || g==0) { std::cout << " Strip::split() : New Strips do not exist, splitting aborted. "<<std::endl; return; }

	if(vertices.size()+1 > f->vertices.size() + g->vertices.size())
	{
		std::cout << " Strip::split() : ERROR: Not all vertices were assigned a Bundle! "<<std::endl;
		std::cout << " Mappings fvert = "; for(std::map<xVert, xVert>::iterator it = fvert.begin(); it != fvert.end(); it++) std::cout <<" "<<it->first<<"->"<<it->second; std::cout << std::endl;
		std::cout << " Mappings gvert = "; for(std::map<xVert, xVert>::iterator it = gvert.begin(); it != gvert.end(); it++) std::cout <<" "<<it->first<<"->"<<it->second; std::cout << std::endl;
	}

	splitAssignPolygonsToConstituentMeshes(f,g,g,fvert,gvert);
	std::cout << " Split mesh into strips with "<<f->polygons.size()<<" and "<<g->polygons.size()<<" polys. "<<std::endl;

	f->setScaleFactor(scaleTexture);
	g->setScaleFactor(scaleTexture);

	f->initMesh();
	g->initMesh();
	f->resetTexture(scaleTexture, 250, 100, 0);
	g->resetTexture(scaleTexture, 250, 200, 100);
}
