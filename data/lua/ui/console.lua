function ui_console(path)
	local y = UIConsoleWindow:new{
		red = 210,
		green = 210,
		blue = 210,
		id = "Strata Console",
		triggerKey = "RETURN",
		title = "Strata Console",
	}
	ui.loadConsoleWindow(y.id)
	ui.loadWindowAttribute(y.id, "triggerKey", y.triggerKey)
	local yt = UIFlatTexture:new{
		red = 50,
		green = 50,
		blue = 50,
		alpha = 200,
	}
	ui.loadFlatTexture(y.id, "background", yt:collectArgs(path))
	ui.loadWindowDimensions(y.id, "box", y:getWindowBox())
	ui.loadWindowFontColour(y.id, "fontcolour", y:getFontColour())
	ui.loadWindowFontColour(y.id, "logFontColour", y.logFontColour:collectArgs())
	ui.loadWindowFunction(y.id, "a", "a")
	ui.loadWindowFunction(y.id, "b", "b")
	ui.loadWindowFunction(y.id, "c", "c")
	ui.loadWindowFunction(y.id, "d", "d")
	ui.loadWindowFunction(y.id, "e", "e")
	ui.loadWindowFunction(y.id, "f", "f")
	ui.loadWindowFunction(y.id, "g", "g")
	ui.loadWindowFunction(y.id, "h", "h")
	ui.loadWindowFunction(y.id, "i", "i")
	ui.loadWindowFunction(y.id, "j", "j")
	ui.loadWindowFunction(y.id, "k", "k")
	ui.loadWindowFunction(y.id, "l", "l")
	ui.loadWindowFunction(y.id, "m", "m")
	ui.loadWindowFunction(y.id, "n", "n")
	ui.loadWindowFunction(y.id, "o", "o")
	ui.loadWindowFunction(y.id, "p", "p")
	ui.loadWindowFunction(y.id, "q", "q")
	ui.loadWindowFunction(y.id, "r", "r")
	ui.loadWindowFunction(y.id, "s", "s")
	ui.loadWindowFunction(y.id, "t", "t")
	ui.loadWindowFunction(y.id, "u", "u")
	ui.loadWindowFunction(y.id, "v", "v")
	ui.loadWindowFunction(y.id, "w", "w")
	ui.loadWindowFunction(y.id, "x", "x")
	ui.loadWindowFunction(y.id, "y", "y")
	ui.loadWindowFunction(y.id, "z", "z")
	ui.loadWindowFunction(y.id, "0", "0")
	ui.loadWindowFunction(y.id, "1", "1")
	ui.loadWindowFunction(y.id, "2", "2")
	ui.loadWindowFunction(y.id, "3", "3")
	ui.loadWindowFunction(y.id, "4", "4")
	ui.loadWindowFunction(y.id, "5", "5")
	ui.loadWindowFunction(y.id, "6", "6")
	ui.loadWindowFunction(y.id, "7", "7")
	ui.loadWindowFunction(y.id, "8", "8")
	ui.loadWindowFunction(y.id, "9", "9")
	ui.loadWindowFunction(y.id, "`", "`")
	ui.loadWindowFunction(y.id, ";", ";")
	ui.loadWindowFunction(y.id, "'", "'")
	ui.loadWindowFunction(y.id, ",", ",")
	ui.loadWindowFunction(y.id, ".", ".")
	ui.loadWindowFunction(y.id, "/", "/")
	ui.loadWindowFunction(y.id, "\\", "\\")
	ui.loadWindowFunction(y.id, "[", "[")
	ui.loadWindowFunction(y.id, "]", "]")
	ui.loadWindowFunction(y.id, "-", "-")
	ui.loadWindowFunction(y.id, "=", "=")
	ui.loadWindowFunction(y.id, " ", " ")
	ui.loadWindowFunction(y.id, "BACKSPACE", "BACKSPACE")
	ui.loadWindowFunction(y.id, "RETURN", "Execute")
end

