-- Load all vegetation.
function load_sky()
	loadobj("sky/basicsky.lua")
end

-- Define a basic SkyBox object
SkyBox = {
	cubemeshsize = 1.0e6,
	boxtexsize = 16,
	gradtex = ""
}

function SkyBox:new(o)
	o = o or {}
	setmetatable(o,self)
	self.__index = self
	return o
end
function SkyBox:collectArgs(path)
	local prefix = findprefix(path)
	return self.cubemeshsize, self.boxtexsize, self.gradtex == "" and "" or prefix .. self.gradtex
end

