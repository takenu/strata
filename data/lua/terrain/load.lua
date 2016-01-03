-- Load Terrain
function load_terrain()
	loadobj("terrain/terrain.lua")
end

TerrainBaseLayerSpecs = {
	size = 300.0,
	maxmeshsize = 70.0,
	divisions = 200,
	height = 0.0,
}

function TerrainBaseLayerSpecs:new(o)
	o = o or {}
	setmetatable(o,self)
	self.__index = self
	return o
end

