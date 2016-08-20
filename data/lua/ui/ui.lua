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
		title = "Chathran Strata",
		fps = "true",
		memusage = "true",
	}
	ui.loadWindowBase(w:collectArgs(path))
	ui.loadMonitorWindow()
	ui.loadMonitorWindowAttribute("fps", w.fps)
	ui.loadMonitorWindowAttribute("memusage", w.memusage)
	local x = UIMainMenuWindow:new{
		red2 = 40,
		green2 = 200,
		blue2 = 200,
		title = "Strata Main Menu",
	}
	ui.loadWindowBase(x:collectArgs(path))
	ui.loadMainMenuWindow()
end

