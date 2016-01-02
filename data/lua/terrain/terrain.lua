function terrain_terrain(path)
	local v = TerrainBaseLayerSpecs:new{
	}
	-- Make the base layer for the terrain, which underlies it.
	terrain.makeFlatLayer(v.size, v.maxmeshsize, v.divisions, v.height)
	-- Add layers one by one. Start with thicker ones and finish with
	-- thinner layers for more detail.
	terrain.addLayer(20.0)
	terrain.addLayer(10.0)
	terrain.addLayer(10.0)
	terrain.addLayer(10.0)
	terrain.addLayer(10.0)
	terrain.addLayer(5.0)
	terrain.addLayer(5.0)
	terrain.addLayer(5.0)
	terrain.addLayer(5.0)
	terrain.addLayer(5.0)
	--
	terrain.addLayer(3.0)
	terrain.addLayer(3.0)
	terrain.addLayer(3.0)
	terrain.addLayer(3.0)
	terrain.addLayer(3.0)
	terrain.addLayer(3.0)
	terrain.addLayer(3.0)
	terrain.addLayer(3.0)
	terrain.addLayer(3.0)
	terrain.addLayer(3.0)
	--
	terrain.addLayer(2.0)
	terrain.addLayer(2.0)
	terrain.addLayer(2.0)
	terrain.addLayer(2.0)
	terrain.addLayer(2.0)
	terrain.addLayer(2.0)
	terrain.addLayer(2.0)
	terrain.addLayer(2.0)
	terrain.addLayer(2.0)
	terrain.addLayer(2.0)
end

