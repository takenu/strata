function ui_tergen(path, v)
	local x = UIWindow:new{
		left = 0.2,
		top = 1.0,
		right = 1.0,
		bottom = -1.0,
		red2 = 40,
		green2 = 200,
		blue2 = 200,
		title = "Strata Terrain Generator",
		triggerKey = "t",
		id = "Strata Terrain Generator",
	}
	ui.loadBaseWindow(x.id)
	ui.loadWindowAttribute(x.id, "triggerKey", x.triggerKey)
	ui.loadWindowFunction(x.id, "c", "compress")
	ui.loadWindowFunction(x.id, "e", "erode")
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
	local xTitle = UIButton:new{
		buttonText = "Strata Terrain Generator",
		parentWindow = x.id,
		functionTarget = x.id,
		functionArgs = "",
		id = "Strata Terrain Generator button",
		left = x.left,
		top = x.top,
		right = x.right,
		bottom = x.top - buttonHeight,
	}
	local xTex = UIFlatTexture:new{
		red = 150,
		green = 150,
		blue = 150,
		alpha = 180,
	}
	ui.loadButton(xTitle.parentWindow, xTitle.id);
	ui.loadFlatTexture(x.id, xTitle.id, xTex:collectArgs(path))
	ui.loadWindowDimensions(x.id, xTitle.id, xTitle:getWindowBox())
	ui.loadButtonAttribute(xTitle.parentWindow, xTitle.id, "text", xTitle.buttonText)
	local xCompress = UIButton:new{
		buttonText = "Compress",
		parentWindow = x.id,
		functionTarget = x.id,
		functionArgs = "compress",
		id = "Compress",
		left = x.left,
		top = xTitle.bottom,
		right = x.right,
		bottom = xTitle.bottom - buttonHeight,
	}
	ui.loadButton(xCompress.parentWindow, xCompress.id)
	ui.loadFlatTexture(x.id, xCompress.id, xTex:collectArgs(path))
	ui.loadWindowDimensions(x.id, xCompress.id, xCompress:getWindowBox())
	ui.loadButtonAttribute(xCompress.parentWindow, xCompress.id, "text", xCompress.buttonText)
	ui.loadButtonAttribute(xCompress.parentWindow, xCompress.id, "receiver", xCompress.functionTarget)
	ui.loadButtonAttribute(xCompress.parentWindow, xCompress.id, "args", xCompress.functionArgs)
end

