-- Load all vegetation.
function load_ui()
	loadobj("ui/ui.lua")
end

UserInterface = {
	fonttex = "",
	fontsize = 0.1,
	fontaspectratio = 2,
	fontpixels = 160,
	fontresolution = 2048
}

function UserInterface:new(o)
	o = o or {}
	setmetatable(o,self)
	self.__index = self
	return o
end
function UserInterface:collectArgs(path)
	local prefix = findprefix(path)
	return self.fonttex == "" and "" or prefix .. self.fonttex, self.fontsize, self.fontaspectratio,self.fontpixels, self.fontresolution
end

