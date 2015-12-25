function ui_ui(path)
	local v = UserInterface:new{
		name = "UserInterface",
		fonttex = "OpenBaskerville-0.0.75.ttf",
	}
	ui.loadUI(v:collectArgs(path))
end

