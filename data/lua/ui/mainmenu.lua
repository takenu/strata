function ui_mainmenu(path, v)
	local x = UIMainMenuWindow:new{
		red2 = 40,
		green2 = 200,
		blue2 = 200,
		title = "Strata Main Menu",
		triggerKey = "ESCAPE",
		id = "Strata Main Menu",
	}
	ui.loadMainMenuWindow(x.id)
	ui.loadWindowAttribute(x.id, "triggerKey", x.triggerKey)
	ui.loadWindowFunction(x.id, "q", "quit")
	ui.loadWindowFunction(x.id, "r", "resume")
	local xt = UIFlatTexture:new{
		red = 100,
		green = 50,
		blue = 50,
		alpha = 150,
	}
	local xHighlight = UIFlatTexture:new{
		red = 160,
		green = 90,
		blue = 90,
		alpha = 150,
	}
	ui.loadFlatTexture(x.id, "background", xt:collectArgs(path))
	ui.loadFlatTexture(x.id, "highlight", xt:collectArgs(path))
	ui.loadWindowDimensions(x.id, "box", x:getWindowBox())
	ui.loadWindowFontColour(x.id, "fontcolour", x:getFontColour())
	ui.loadWindowFontColour(x.id, "fonthighlight", x:getFontHighlight())
	local buttonHeight = v.fontsize
	local xQuit = UIButton:new{
		buttonText = "Quit",
		parentWindow = x.id,
		functionTarget = x.id,
		functionArgs = "quit",
		id = "Quit",
		left = x.left,
		top = x.top - buttonHeight,
		right = x.right,
		bottom = x.top - 2*buttonHeight,
	}
	local xResume = UIButton:new{
		buttonText = "Resume",
		parentWindow = x.id,
		functionTarget = x.id,
		functionArgs = "resume",
		id = "Resume",
		left = x.left,
		top = xQuit.bottom,
		right = x.right,
		bottom = xQuit.bottom - buttonHeight,
	}
	local xTex = UIFlatTexture:new{
		red = 150,
		green = 150,
		blue = 150,
		alpha = 180,
	}
	ui.loadButton(xQuit.parentWindow, xQuit.id)
	ui.loadFlatTexture(x.id, xQuit.id, xTex:collectArgs(path))
	ui.loadWindowDimensions(x.id, xQuit.id, xQuit:getWindowBox())
	ui.loadButtonAttribute(xQuit.parentWindow, xQuit.id, "text", xQuit.buttonText)
	ui.loadButtonAttribute(xQuit.parentWindow, xQuit.id, "receiver", xQuit.functionTarget)
	ui.loadButtonAttribute(xQuit.parentWindow, xQuit.id, "args", xQuit.functionArgs)
	ui.loadButton(xResume.parentWindow, xResume.id)
	ui.loadFlatTexture(x.id, xResume.id, xTex:collectArgs(path))
	ui.loadWindowDimensions(x.id, xResume.id, xResume:getWindowBox())
	ui.loadButtonAttribute(xResume.parentWindow, xResume.id, "text", xResume.buttonText)
	ui.loadButtonAttribute(xResume.parentWindow, xResume.id, "receiver", xResume.functionTarget)
	ui.loadButtonAttribute(xResume.parentWindow, xResume.id, "args", xResume.functionArgs)
end

