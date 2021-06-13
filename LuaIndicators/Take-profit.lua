APPDATA = os.getenv('APPDATA')
JSON = (loadfile "LuaIndicators\\JSON.lua")()

Settings=
{
    Name = "Take-profit",
    period = 5, 
    line = {
             {
                Name = "Take-profit", 
                Color = RGB(38, 166, 154),
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
	local FilePath = APPDATA .. "\\QuikLines\\storage\\line4.json"
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