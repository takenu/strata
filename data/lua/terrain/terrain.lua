function terrain_terrain(path)
	local v = TerrainBaseLayerSpecs:new{
	}
	-- Make the base layer for the terrain, which underlies it.
	terrain.makeFlatLayer(v.size, v.maxmeshsize, v.divisions, v.height)
	-- Add layers one by one. Start with thicker ones and finish with
	-- thinner layers for more detail.
	terrain.addLayer(2.0)
	terrain.addLayer(2.0)
--[[	terrain.addLayer(1.0)
	terrain.addLayer(1.0)
	terrain.addLayer(1.0)
	terrain.addLayer(1.0)
	terrain.addLayer(0.5)
	terrain.addLayer(0.5)
	terrain.addLayer(0.5)
	terrain.addLayer(0.5)
	terrain.addLayer(0.5)
	--
	terrain.addLayer(0.3)
	terrain.addLayer(0.3)
	terrain.addLayer(0.3)
	terrain.addLayer(0.3)
	terrain.addLayer(0.3)
	terrain.addLayer(0.3)
	terrain.addLayer(0.3)
	terrain.addLayer(0.3)
	terrain.addLayer(0.3)
	terrain.addLayer(0.3)
	--
	terrain.addLayer(0.2)
	terrain.addLayer(0.2)
	terrain.addLayer(0.2)
	terrain.addLayer(0.2)
	terrain.addLayer(0.2)
	terrain.addLayer(0.2)
	terrain.addLayer(0.2)
	terrain.addLayer(0.2)
	terrain.addLayer(0.2)
	terrain.addLayer(0.2) ]]--
end

