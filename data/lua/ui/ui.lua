function ui_ui(path)
	ui.loadAttribute("closeKey", "ESCAPE")
	local v = UIFont:new{
		id = "UserInterface",
		fonttex = "OpenBaskerville-0.0.75.ttf",
	}
	ui.loadFont(v:collectArgs(path))
	loadobj("ui/monitor.lua")
	loadobj("ui/mainmenu.lua", v)
	loadobj("ui/console.lua")
end

