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

#include <functional>

#include "terrain.hpp"

using namespace strata::mesh;
using namespace std::placeholders; // for using std::bind

Bundle * Terrain::makeNewBundle(void)
{
	return new Bundle(++bundleCounter, bundles, renderer);
}

Strip * Terrain::makeNewStrip(void)
{
	return new Strip(++stripCounter, strips, renderer, false, false);
}

Strip * Terrain::makeNewStitch(bool isTransverseStitch)
{
	return new Strip(++stripCounter, strips, renderer, true, isTransverseStitch);
}

/** Duplicate an existing layer, resulting in a new layer at a given height above the old one.
  * The positioning of the vertices of the new layer is using the normals from the old layer's vertices.
  * The Bundle/Strip structure of the new layer will mirror the structure of the underlying layer. Note that
  * this similarity is not strictly necessary but merely convenient, one should not be concerned about these
  * properties continuing to be similar. In principle it is expected that the structure will diverge during
  * terrain manipulation.
  */
void Terrain::duplicateLayer(const Layer * baseLayer, float thickness)
{
	layers.push_back(new Layer());
	layers.back()->setBundleTexture(new tiny::draw::RGBTexture2D(
				*(masterLayer->getBundleTexture())));
	layers.back()->setStripTexture(new tiny::draw::RGBTexture2D(
				*(masterLayer->getStripTexture())));
	layers.back()->setStitchTexture(new tiny::draw::RGBTexture2D(
				*(masterLayer->getStitchTexture())));
	std::vector<const Bundle *> baseBundles;
	std::vector<const Strip *> baseStrips;
	// First collect bundles and strips of the base layer. Do not add Bundles and Strips yet - that would mess up the std::map.
	for(std::map<long unsigned int, Bundle*>::const_iterator it = bundles.begin(); it != bundles.end(); it++)
		if(it->second->getParentLayer() == baseLayer)
			baseBundles.push_back(it->second);
	for(std::map<long unsigned int, Strip*>::const_iterator it = strips.begin(); it != strips.end(); it++)
		if(it->second->getParentLayer() == baseLayer && !(it->second->isStitchMesh())) // skip Stitches - they are re-made separately.
			baseStrips.push_back(it->second);
	// Now duplicate all bundles and strips of the base layer.
	std::map<const Bundle*, Bundle*> bmap;
	std::map<const Strip*, Strip*> smap;
	for(unsigned int i = 0; i < baseBundles.size(); i++)
	{
		Bundle * bundle = makeNewBundle();
		bundle->setParentLayer(layers.back());
		layers.back()->addBundle(bundle);
		baseBundles[i]->duplicateBundle(bundle);
		bmap.emplace(baseBundles[i],bundle);
	}
	for(unsigned int i = 0; i < baseStrips.size(); i++)
	{
		Strip * strip = makeNewStrip();
		strip->setParentLayer(layers.back());
		baseStrips[i]->duplicateStrip(strip);
		smap.emplace(baseStrips[i],strip);
	}
	// Move all vertices of the new Mesh a fixed distance along the direction of their respective normals.
	layers.back()->increaseThickness(thickness);
	// Update all cross references: adjust Strip owningBundle, and adjust adjacentBundles/adjacentStrips
	for(unsigned int i = 0; i < baseBundles.size(); i++)
		bmap.at(baseBundles[i])->duplicateAdjustAdjacentStrips(smap);
	for(unsigned int i = 0; i < baseStrips.size(); i++)
	{
		smap.at(baseStrips[i])->duplicateAdjustAdjacentBundles(bmap);
		smap.at(baseStrips[i])->duplicateAdjustOwningBundles(bmap);
	}
	for(std::map<const Strip*, Strip*>::iterator it = smap.begin(); it != smap.end(); it++)
		it->second->recalculateVertexPositions(); // Strip positions are not updated by the Layer and need to be re-set
	// Copy all other attributes, and initialize meshes.
	for(std::map<const Bundle*, Bundle*>::iterator it = bmap.begin(); it != bmap.end(); it++)
	{
		it->second->setScaleFactor(it->first->getScaleFactor());
		it->second->resetTexture(layers.back()->getBundleTexture());
	}
	for(std::map<const Strip*, Strip*>::iterator it = smap.begin(); it != smap.end(); it++)
	{
		it->second->setScaleFactor(it->first->getScaleFactor());
		it->second->resetTexture(layers.back()->getStripTexture());
	}
	// Collect layer edge vertices and connect them to the underlying layer. Since
	// layer duplication is normally done on flat terrains, extending Layers along
	// their surface is not an option and we force all Stitches to be transversal
	// (i.e. cutting through the Layer).
	stitchLayer(layers.back(), true);
}

/** Stitch a floating Layer to the layers underneath it. Stitching is performed
  * such that the stitch width is as narrow as possible.
  * Note that stitching only works on floating Layers that are not yet connected
  * to the Layers under them. After stitching, the Layer should always be
  * well-defined and have no more holes or openings at its edges.
  */
void Terrain::stitchLayer(Layer * layer, bool stitchTransverse)
{
//	std::cout << " Stitch layer "<<layer<<"..."<<std::endl;
	xVert startVertex = 0;
//	Bundle * startBundle = 0;
	std::vector<StripVertex> edgeVertices;
	// Find an initial vertex to start stitching.
	// For every Bundle such a vertex is found. We will stitch until all of
	// these vertices are stitched. Using a vertex per Bundle (rather than
	// per Layer) ensures that stitching works even on Layers that consist of
	// multiple non-mutually-connected meshes.
	for(std::map<long unsigned int, Bundle*>::iterator it = bundles.begin(); it != bundles.end(); it++)
		if(it->second->getParentLayer() == layer)
			if(it->second->findVertexAtLayerEdge(startVertex))
			{
//				startBundle = it->second;
				if(layers.size()==1) std::cout << " stitchLayer() : Found vertex at layer edge at "<<it->second->getVertexPositionFromIndex(startVertex)<<"..."<<std::endl;
				edgeVertices.push_back( it->second->getVertexPositionFromIndex(startVertex) );
				edgeVertices.back().setOwningBundle(it->second);
				edgeVertices.back().setRemoteIndex(startVertex);
//				break;
			}
	// Make a Stitch Strip object.
/*	Strip * stitch = 0;
	while(edgeVertices.size() > 0)
	{
		StripVertex startVertex = edgeVertices.size();
		stitch = makeNewStitch();
	}
	// Set Stitch texture and parent.
	stitch->resetTexture(layer->getStitchTexture());
	stitch->setParentLayer(layer);*/
//	stitch->setScaleFactor(startBundle->getScaleFactor());
}

void Terrain::getUnderlyingVertex(Bundle * & bundle, xVert & index, const Layer * baseLayer, tiny::vec3 v)
{
}
