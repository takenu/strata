-- Load Terrain
function load_terrain()
	loadobj("terrain/terrain.lua")
end

TerrainBaseLayerSpecs = {
	size = 3000.0,
	maxmeshsize = 700.0,
	divisions = 300,
	height = 0.0,
}

function TerrainBaseLayerSpecs:new(o)
	o = o or {}
	setmetatable(o,self)
	self.__index = self
	return o
end

