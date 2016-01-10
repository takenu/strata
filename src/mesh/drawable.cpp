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

#include "interface.hpp"

#include "drawable.hpp"

using namespace strata::mesh;

void DrawableMesh::initMesh(void)
{
	if(renderMesh)
	{
		std::cout << " DrawableMesh::initMesh() : WARNING: Attempt to re-initialize mesh! "<<std::endl;
		return;
	}
	else if(!texture)
	{
		std::cout << " DrawableMesh::initMesh() : ERROR: Cannot initialize Mesh without Texture! "<<std::endl;
		return;
	}
	renderMesh = new tiny::draw::StaticMesh( convertToMesh() );
	renderMesh->setDiffuseTexture(*texture);
	renderer->addWorldRenderable(renderMesh);
}

void DrawableMesh::resetTexture(tiny::draw::RGBTexture2D * _texture)
{
	texture = _texture;
	if(renderMesh)
	{
		renderMesh->setDiffuseTexture(*texture);
	}
	else initMesh();
}

void DrawableMesh::resetMesh(void)
{
	if(!renderMesh)
		std::cout << " Drawable::initMesh() : No mesh yet, use initMesh() instead! "<<std::endl;
	else if(!texture)
		std::cout << " Drawable::initMesh() : No texture yet, cannot reset! "<<std::endl;
	else
	{
		// TODO: Instead of deleting and re-adding the mesh, we should be able to update its buffers.
		renderer->freeWorldRenderable(renderMesh);
		delete renderMesh;
		renderMesh = 0;
		initMesh();
	}
}

DrawableMesh::~DrawableMesh(void)
{
	if(renderMesh)
		renderer->freeWorldRenderable(renderMesh);
}
