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
	ui.loadWindowAttribute("Chathran Strata Monitor", "fontsize", v.fontsize)
	ui.loadWindowAttribute("Chathran Strata Monitor", "fontaspectratio", v.fontaspectratio)
	ui.loadWindowAttribute("Chathran Strata Monitor", "fps", w.fps)
	ui.loadWindowAttribute("Chathran Strata Monitor", "memusage", w.memusage)
	local wt = UIFlatTexture:new{
		red = 50,
		green = 50,
		blue = 150,
		alpha = 60,
	}
	ui.loadFlatTexture(w.title, wt:collectArgs(path))
	ui.loadWindowDimensions(w.title, "box", w:getWindowBox())
	ui.loadWindowFontColour(w.title, "fontcolour", w:getFontColour())
	local x = UIMainMenuWindow:new{
		red2 = 40,
		green2 = 200,
		blue2 = 200,
		title = "Strata Main Menu",
		id = "Strata Main Menu",
	}
	ui.loadMainMenuWindow(x.id)
	ui.loadWindowAttribute("Strata Main Menu", "fontsize", v.fontsize)
	ui.loadWindowAttribute("Strata Main Menu", "fontaspectratio", v.fontaspectratio)
	local xt = UIFlatTexture:new{
		red = 100,
		green = 50,
		blue = 50,
		alpha = 150,
	}
	ui.loadFlatTexture(x.title, xt:collectArgs(path))
	ui.loadWindowDimensions(x.title, "box", x:getWindowBox())
	ui.loadWindowFontColour(x.title, "fontcolour", x:getFontColour())
	ui.loadWindowFontColour(x.title, "fontcolour2", x:getFontColour2())
end

