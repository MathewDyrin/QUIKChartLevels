APPDATA = os.getenv('APPDATA')
JSON = (loadfile "LuaIndicators\\JSON.lua")()

Settings=
{
    Name = "Stop",
    period = 5, 
    line = {
             {
                Name = "Stop", 
                Color = RGB(239, 83, 83),
                Type = TYPE_DASHDOT,
                Width = 1
             }
          }
}

ticker = nil

function Init()
	ticker = getDataSourceInfo().sec_code
    return 1
end

function OnCalculate(index)
	local FilePath = APPDATA .. "\\QuikLines\\storage\\line3.json"
	local FileReader = io.open(FilePath, "r")
	local Content = FileReader:read("*all")
	FileReader:close()
	local Data = JSON:decode(Content)

	if Data[ticker] then
		return Data[ticker]
	else
		return nil
	end
end