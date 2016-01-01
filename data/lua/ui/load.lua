-- Load UI elements.
function load_ui()
	loadobj("ui/ui.lua")
end

UIFont = {
	fonttex = "",
	fontsize = 0.1,
	fontaspectratio = 2,
	fontpixels = 160,
	fontresolution = 2048,
}

UIWindow = {
	left = -1.0,
	top = 1.0,
	right = 1.0,
	bottom = -1.0,
	red = 40,
	green = 100,
	blue = 40,
}

function UIFont:new(o)
	o = o or {}
	setmetatable(o,self)
	self.__index = self
	return o
end
function UIWindow:new(o)
	o = o or {}
	setmetatable(o,self)
	self.__index = self
	return o
end

UIMonitorWindow = UIWindow:new{
	title = "",
	fps = false,
}

function UIFont:collectArgs(path)
	local prefix = findprefix(path)
	return self.fonttex == "" and "" or prefix .. self.fonttex, self.fontsize, self.fontaspectratio,self.fontpixels, self.fontresolution
end

function UIMonitorWindow:collectArgs(path)
	local prefix = findprefix(path)
	return self.left, self.top, self.right, self.bottom, self.red, self.green, self.blue, self.title
end

