function ui_console(path)
	local y = UIConsoleWindow:new{
		red = 210,
		green = 210,
		blue = 210,
		id = "Strata Console",
		title = "Strata Console",
	}
	ui.loadConsoleWindow(y.id)
	ui.loadWindowAttribute(y.id, "fontsize", v.fontsize)
	ui.loadWindowAttribute(y.id, "fontaspectratio", v.fontaspectratio)
	local yt = UIFlatTexture:new{
		red = 50,
		green = 50,
		blue = 50,
		alpha = 200,
	}
	ui.loadFlatTexture(y.id, "background", yt:collectArgs(path))
	ui.loadWindowDimensions(y.id, "box", y:getWindowBox())
	ui.loadWindowFontColour(y.id, "fontcolour", y:getFontColour())
end

