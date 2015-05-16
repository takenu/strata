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

#include <tiny/mesh/io/staticmesh.h>
#include <tiny/img/io/image.h>

#include <config.h>

#include <selene.h>

#include "sky.hpp"

using namespace strata::core;

void SkyManager::loadSkyBox(float cubeMeshSize, int boxTexSize, std::string gradTex)
{
	if(cubeMeshSize < 100 || boxTexSize < 1 || gradTex == "") { std::cerr << " loadSkyBox called from Lua with invalid args: "<<cubeMeshSize<<", "<<boxTexSize<<", "<<gradTex<<"."<<std::endl; return;}
	//Create sky (a simple cube containing the world).
	skyBox = new tiny::draw::StaticMesh(tiny::mesh::StaticMesh::createCubeMesh(-1.0*cubeMeshSize));
	skyBoxTexture = new tiny::draw::RGBTexture2D(tiny::img::Image::createSolidImage(boxTexSize), tiny::draw::tf::filter);
	skyBox->setDiffuseTexture(*skyBoxTexture);
	
	//Render using a more advanced shading model.
	sunSky = new tiny::draw::effects::SunSky();
	skyGradientTexture = new tiny::draw::RGBATexture2D(tiny::img::io::readImage(DATA_DIRECTORY + "img/" + gradTex));
	sunSky->setSkyTexture(*skyGradientTexture);

	renderer->addWorldRenderable(skyBox);
	renderer->addScreenRenderable(sunSky,false,false);
}

void SkyManager::registerLuaFunctions(sel::State & luaState)
{
	luaState["sky"].SetObj(*this,
			"loadSkyBox", &SkyManager::loadSkyBox);
}

void SkyManager::cleanup(void)
{
	delete sunSky;
	delete skyBox;
	delete skyBoxTexture;
	delete skyGradientTexture;	
}
