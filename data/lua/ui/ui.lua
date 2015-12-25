function ui_ui(path)
	local v = UIFont:new{
		name = "UserInterface",
		fonttex = "OpenBaskerville-0.0.75.ttf",
	}
	ui.loadFont(v:collectArgs(path))
	local w = UIMonitorWindow:new{
		title = "Chathran Strata",
		fps = true,
	}
	ui.loadMonitorWindow(w:collectArgs(path))
end

