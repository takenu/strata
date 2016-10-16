function ui_monitor(path)
	local w = UIMonitorWindow:new{
		red = 40,
		green = 100,
		blue = 40,
		title = "Chathran Strata Monitor",
		triggerKey = "m",
		id = "Chathran Strata Monitor",
		fps = "true",
		memusage = "true",
	}
	ui.loadMonitorWindow(w.id)
	ui.loadWindowAttribute(w.id, "triggerKey", w.triggerKey)
	ui.loadWindowAttribute(w.id, "fps", w.fps)
	ui.loadWindowAttribute(w.id, "memusage", w.memusage)
	local wt = UIFlatTexture:new{
		red = 50,
		green = 50,
		blue = 150,
		alpha = 60,
	}
	ui.loadFlatTexture(w.id, "background", wt:collectArgs(path))
	ui.loadWindowDimensions(w.id, "box", w:getWindowBox())
	ui.loadWindowFontColour(w.id, "fontcolour", w:getFontColour())
end

