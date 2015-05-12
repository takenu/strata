function sky_basicsky(path)
	local v = SkyBox:new{
		name = "BasicSkyBox",
		gradtex = "sky.png",
	}
	sky.loadSkyBox(v:collectArgs(path))
end

