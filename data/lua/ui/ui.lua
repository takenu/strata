function ui_ui(path)
	local v = UIFont:new{
		name = "UserInterface",
		fonttex = "OpenBaskerville-0.0.75.ttf",
	}
	ui.loadFont(v:collectArgs(path))
	local w = UIMonitorWindow:new{
		red = 40,
		green = 100,
		blue = 40,
		title = "Chathran Strata Monitor",
		id = "Chathran Strata Monitor",
		fps = "true",
		memusage = "true",
	}
	ui.loadMonitorWindow(w.id)
	ui.loadWindowAttribute(w.id, "fontsize", v.fontsize)
	ui.loadWindowAttribute(w.id, "fontaspectratio", v.fontaspectratio)
	ui.loadWindowAttribute(w.id, "fps", w.fps)
	ui.loadWindowAttribute(w.id, "memusage", w.memusage)
	local wt = UIFlatTexture:new{
		red = 50,
		green = 50,
		blue = 150,
		alpha = 60,
	}
	ui.loadFlatTexture(w.id, wt:collectArgs(path))
	ui.loadWindowDimensions(w.id, "box", w:getWindowBox())
	ui.loadWindowFontColour(w.id, "fontcolour", w:getFontColour())
	local x = UIMainMenuWindow:new{
		red2 = 40,
		green2 = 200,
		blue2 = 200,
		title = "Strata Main Menu",
		id = "Strata Main Menu",
	}
	ui.loadMainMenuWindow(x.id)
	ui.loadWindowAttribute(x.id, "fontsize", v.fontsize)
	ui.loadWindowAttribute(x.id, "fontaspectratio", v.fontaspectratio)
	local xt = UIFlatTexture:new{
		red = 100,
		green = 50,
		blue = 50,
		alpha = 150,
	}
	ui.loadFlatTexture(x.id, xt:collectArgs(path))
	ui.loadWindowDimensions(x.id, "box", x:getWindowBox())
	ui.loadWindowFontColour(x.id, "fontcolour", x:getFontColour())
	ui.loadWindowFontColour(x.id, "fontcolour2", x:getFontColour2())
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
	ui.loadFlatTexture(y.id, yt:collectArgs(path))
	ui.loadWindowDimensions(y.id, "box", y:getWindowBox())
	ui.loadWindowFontColour(y.id, "fontcolour", y:getFontColour())
end

