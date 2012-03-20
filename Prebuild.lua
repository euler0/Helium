require "Helium"

-- Pre-build script execution.
Helium.Prebuild = function()

	local python = "python"
	
	if os.get() == "windows" then
		python = "python.exe"
		if os.pathsearch( python, os.getenv( 'PATH' ) ) == nil then
			python = "python3.bat"
		end
    else
		python = python .. "3"
    end

	local pythonPath = os.pathsearch( python, os.getenv( 'PATH' ) )
	if pythonPath == nil then
		error( "\n\nYou must have Python 3.x installed and in your PATH to continue." )
	end

	local commands =
	{
		python .. " Prebuild-JobDefParser.py EngineJobs . .",
		python .. " Prebuild-JobDefParser.py Framework . .",
		python .. " Prebuild-JobDefParser.py GraphicsJobs . .",
		python .. " Prebuild-JobDefParser.py TestJobs . .",
		python .. " Prebuild-TypeParser.py D3D9Rendering EditorSupport Engine EngineJobs Framework FrameworkWin Graphics GraphicsJobs GraphicsTypes PcSupport PreprocessingPc Rendering TestJobs WinWindowing Windowing",
		python .. " Prebuild-TypeParser.py -i Example -s Example -p EXAMPLE_ ExampleGame ExampleMain",
	}

	local result = 0

	for i, commandString in ipairs( commands ) do
		result = os.execute( commandString )
		if result ~= 0 then
			break
		end
	end

	if result ~= 0 then
		error( "An error occurred processing the pre-build scripts." )
	end

end