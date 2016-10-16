path = "../data/lua/"

function loadscr(fname)
	local f = assert(loadfile(path .. fname))
	f()
end

function loadobj(path, ...)
	loadscr(path)
--	print("function will be: ","function getfunc() return " .. string.gsub(string.sub(path,1,string.find(path,'.lua')-1),'/','_') .. " end")
	local h = assert(load("function getfunc() return " .. string.gsub(string.sub(path,1,string.find(path,'.lua')-1),'/','_') .. " end"))
	h()
	local func = getfunc()
	func(path, ...)
	getfunc = nil
end

function findprefix(path)
	local prefix = ""
	local s = path;
	while string.find(s,'/') do prefix, s = prefix .. string.sub(s,1,string.find(s,'/')), string.sub(s,string.find(s,'/')+1,-1) end
	return prefix
end

-- Keep all loaded objects in a table. Objects whose identifier (<path>::<name>)
-- is in this table will not be initialized again. This also implies that
-- any objects deleted from the C++ program should be deleted from here,
-- or otherwise they cannot be re-loaded when necessary.
loadedObjects = {}
function makeid(path,name)
	return path .. "::" .. name
end
function unload(id) -- set an object as not-yet-loaded
	loadedObjects[id] = nil
end
function finalize(id) -- set an object as loaded and ready for use
	loadedObjects[id] = true
end

-- The startup of Lua as called from the main executable.
function start()
	-- Load sky before UI, otherwise UI goes behind sky
	loadscr("sky/load.lua"); load_sky()
	loadscr("ui/load.lua"); load_ui()
	loadscr("terrain/load.lua"); load_terrain()
end

