APPDATA = os.getenv('APPDATA')
JSON = (loadfile "LuaIndicators\\JSON.lua")()

Settings=
{
    Name = "Line2",
    period = 5, 
    line = {
             {
                Name = "Line2", 
                Color = RGB(255, 255, 0),
                Type = TYPE_LINE,
                Width = 2
             }
          }
}

ticker = nil

function Init()
	ticker = getDataSourceInfo().sec_code
    return 1
end

function OnCalculate(index)
	local FilePath = APPDATA .. "\\QuikLines\\storage\\line2.json"
	local FileReader = io.open(FilePath, "r")
	local Content = FileReader:read("*all")
	FileReader:close()
	local Data = JSON:decode(Content)

	if Data[ticker] then
		return Data[ticker]
	else
		return 0
	end
end