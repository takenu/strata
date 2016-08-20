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
	ui.loadWindowBase(w:collectArgs(path))
	ui.loadMonitorWindow(w.id)
--	ui.loadMonitorWindowAttribute("fps", w.fps)
--	ui.loadMonitorWindowAttribute("memusage", w.memusage)
	ui.loadWindowAttribute("Chathran Strata Monitor", "fps", w.fps)
	ui.loadWindowAttribute("Chathran Strata Monitor", "memusage", w.memusage)
	local wt = UIFlatTexture:new{
		red = 50,
		green = 50,
		blue = 150,
		alpha = 60,
	}
	ui.loadFlatTexture(w.title, wt:collectArgs(path))
	local x = UIMainMenuWindow:new{
		red2 = 40,
		green2 = 200,
		blue2 = 200,
		title = "Strata Main Menu",
		id = "Strata Main Menu",
	}
	ui.loadWindowBase(x:collectArgs(path))
	ui.loadMainMenuWindow(x.id)
	local xt = UIFlatTexture:new{
		red = 100,
		green = 50,
		blue = 50,
		alpha = 150,
	}
	ui.loadFlatTexture(x.title, xt:collectArgs(path))
end

