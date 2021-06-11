APPDATA = os.getenv('APPDATA')
JSON = (loadfile "LuaIndicators\\JSON.lua")()

Settings=
{
    Name = "Line4",
    period = 5, 
    line = {
             {
                Name = "Line4", 
                Color = RGB(0, 128, 192),
                Type = TYPE_POINT,
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
	local FilePath = APPDATA .. "\\QuikLines\\storage\\line4.json"
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