-- Load UI elements.
function load_ui()
	loadobj("ui/ui.lua")
end

UIFont = {
	fonttex = "",
	fontsize = 0.08,
	fontaspectratio = 2,
	fontpixels = 160,
	fontresolution = 2048,
	id = "Default",
}

UIFlatTexture = {
	size = 64,
	red = 50,
	green = 50,
	blue = 50,
	alpha = 100,
}

UIWindow = {
	left = -1.0,
	top = 1.0,
	right = 1.0,
	bottom = -1.0,
	red = 40, -- Text colour (RGB)
	green = 40,
	blue = 40,
	red2 = 100, -- Text colour 2 (RGB)
	green2 = 40,
	blue2 = 40,
	triggerKey = "",
	id = "Default"
}

UIColour = {
	red = 150,
	green = 150,
	blue = 150,
}

function UIFont:new(o)
	o = o or {}
	setmetatable(o,self)
	self.__index = self
	return o
end
function UIFlatTexture:new(o)
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
function UIColour:new(o)
	o = o or {}
	setmetatable(o,self)
	self.__index = self
	return o
end

UIConsoleWindow = UIWindow:new{
	title = "",
	right = 0.0,
	top = -0.4,
	id = "Default Console",
	logFontColour = UIColour:new{}
}

UIMonitorWindow = UIWindow:new{
	title = "",
	fps = false,
	right = 0.0,
	bottom = 0.4,
	id = "Default Monitor",
}

UIMainMenuWindow = UIWindow:new{
	title = "",
	left = -0.3,
	top = 0.5,
	right = 0.3,
	bottom = -0.5,
	id = "Default Main Menu",
}

UIButton = UIWindow:new{
	buttonText = "Button",
	parentWindow = "",
	functionTarget = "",
	functionArgs = "",
	id = "Default Button",
}

function UIFont:collectArgs(path)
	local prefix = findprefix(path)
	return self.fonttex == "" and "" or prefix .. self.fonttex, self.fontsize, self.fontaspectratio,self.fontpixels, self.fontresolution
end

function UIFlatTexture:collectArgs(path)
	return self.size, self.red, self.green, self.blue, self.alpha
end

function UIWindow:getWindowBox()
	return self.left, self.top, self.right, self.bottom
end

function UIWindow:getFontColour()
	return self.red, self.green, self.blue
end

function UIWindow:getFontHighlight()
	return self.red2, self.green2, self.blue2
end

function UIColour:collectArgs()
	return self.red, self.green, self.blue
end

