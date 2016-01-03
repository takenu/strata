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
	return new Strip(++stripCounter, strips, renderer, false);
}

Strip * Terrain::makeNewStitch(void)
{
	return new Strip(++stripCounter, strips, renderer, true);
}

/** Duplicate an existing layer, resulting in a new layer at a given height above the old one.
  * The positioning of the vertices of the new layer is using the normals from the old layer's vertices.
  * The Bundle/Strip structure of the new layer will mirror the structure of the underlying layer. Note that
  * this similarity is not strictly necessary but merely convenient, one should not be concerned about these
  * properties continuing to be similar. In principle it is expected that the structure will diverge during
  * terrain manipulation.
  */
void Terrain::duplicateLayer(Layer * baseLayer, float thickness)
{
	layers.push_back(new Layer());
	std::vector<Bundle *> baseBundles;
	std::vector<Strip *> baseStrips;
	// First collect bundles and strips of the base layer. Do not add Bundles and Strips yet - that would mess up the std::map.
	for(std::map<long unsigned int, Bundle*>::iterator it = bundles.begin(); it != bundles.end(); it++)
		if(it->second->getParentLayer() == baseLayer)
			baseBundles.push_back(it->second);
	for(std::map<long unsigned int, Strip*>::iterator it = strips.begin(); it != strips.end(); it++)
		if(it->second->getParentLayer() == baseLayer)
			baseStrips.push_back(it->second);
	// Now duplicate all bundles and strips of the base layer.
	std::map<Bundle*, Bundle*> bmap;
	std::map<Strip*, Strip*> smap;
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
	// Update all cross references: adjust Strip owningBundle, and adjust adjacentBundles/adjacentStrips
	for(unsigned int i = 0; i < baseBundles.size(); i++)
		bmap.at(baseBundles[i])->duplicateAdjustAdjacentStrips(smap);
	for(unsigned int i = 0; i < baseStrips.size(); i++)
	{
		smap.at(baseStrips[i])->duplicateAdjustAdjacentBundles(bmap);
		smap.at(baseStrips[i])->duplicateAdjustOwningBundles(bmap);
	}
	// Copy all other attributes, and initialize meshes.
	layers.back()->setBundleTexture(new tiny::draw::RGBTexture2D(
				*(masterLayer->getBundleTexture())));
	layers.back()->setStripTexture(new tiny::draw::RGBTexture2D(
				*(masterLayer->getStripTexture())));
	layers.back()->setStitchTexture(new tiny::draw::RGBTexture2D(
				*(masterLayer->getStitchTexture())));
	for(std::map<Bundle*, Bundle*>::iterator it = bmap.begin(); it != bmap.end(); it++)
	{
		it->second->setScaleFactor(it->first->getScaleFactor());
		it->second->resetTexture(layers.back()->getBundleTexture());
	}
	for(std::map<Strip*, Strip*>::iterator it = smap.begin(); it != smap.end(); it++)
	{
		it->second->setScaleFactor(it->first->getScaleFactor());
		it->second->resetTexture(layers.back()->getStripTexture());
	}
	// Move all vertices of the Mesh a fixed distance along the direction of their respective normals.
	layers.back()->increaseThickness(thickness);
	for(std::map<Strip*, Strip*>::iterator it = smap.begin(); it != smap.end(); it++)
		it->second->recalculateVertexPositions(); // Strip positions are not updated by the Layer and need to be re-set
	// Collect layer edge vertices and connect them to the underlying layer
	for(std::map<Bundle*, Bundle*>::iterator it = bmap.begin(); it != bmap.end(); it++)
		it->second->fixParameters();
	std::vector<StripPolygon> stitchPolygons;
}
