function ui_ui(path)
	local v = UIFont:new{
		name = "UserInterface",
		fonttex = "OpenBaskerville-0.0.75.ttf",
	}
	ui.loadFont(v:collectArgs(path))
	local w = UIMonitorWindow:new{
		title = "Chathran Strata",
		fps = "true",
		memusage = "true",
	}
	ui.loadWindowBase(w:collectArgs(path))
	ui.loadMonitorWindow()
	ui.loadMonitorWindowAttribute("fps", w.fps)
	ui.loadMonitorWindowAttribute("memusage", w.memusage)
end

