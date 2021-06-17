#include <iostream>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <map>
#include <vector>
#include <windows.h>
#include <atlimage.h>
#include <sstream>
#include <codecvt>
#include <shlobj.h>
#include "boost/filesystem.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/locale.hpp"

using json = nlohmann::json;

std::string PROGRAM_DATA_PATH;

std::wstring StringToWSTRING(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

static std::string fromLocale(const std::string& localeStr) {
    boost::locale::generator g;
    g.locale_cache_enabled(true);
    std::locale loc = g(boost::locale::util::get_system_locale());
    return boost::locale::conv::to_utf<char>(localeStr, loc);
}

static std::string toLocale(const std::string& utf8Str) {
    boost::locale::generator g;
    g.locale_cache_enabled(true);
    std::locale loc = g(boost::locale::util::get_system_locale());
    return boost::locale::conv::from_utf<char>(utf8Str, loc);
}

bool IsDir(const TCHAR* dir) {
    DWORD flag = GetFileAttributes(dir);
    if (flag == 0xFFFFFFFFUL) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
            return false;
    }
    if (!(flag & FILE_ATTRIBUTE_DIRECTORY))
        return false;
    return true;
}

bool IsDir(const std::string dir) {
    DWORD flag = GetFileAttributes(StringToWSTRING(dir).c_str());
    if (flag == 0xFFFFFFFFUL) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
            return false;
    }
    if (!(flag & FILE_ATTRIBUTE_DIRECTORY))
        return false;
    return true;
}

std::string GetAppData()
{
    // обычно это 'C:\Users\<user>\AppData\Roaming'
    WCHAR path[MAX_PATH];
    HRESULT res = SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, path);
    std::wstring ws(path);
    std::string result(ws.begin(), ws.end()); 

    return result;
}

void ClearCin() 
{
    if (char(std::cin.peek()) == '\n')
        std::cin.ignore();

    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(32767, '\n');
    }
}

bool CheckDirectoryAndCreate(std::string path) 
{
    boost::filesystem::path dir(path);

    if (!boost::filesystem::exists(dir)) 
    {
        if (boost::filesystem::create_directory(dir)) {
            return true;
        }

        else 
        {
            return false; 
        }
    
    }

    return true; 
}

BOOL SetProgramDataDirectory() 
{
    std::string path = GetAppData(); 

    if (path.size() != 0) 
    {
        PROGRAM_DATA_PATH = std::move(path) + "\\QuikLines"; 
        CheckDirectoryAndCreate(PROGRAM_DATA_PATH); 

        PROGRAM_DATA_PATH = PROGRAM_DATA_PATH + "\\storage";
        CheckDirectoryAndCreate(PROGRAM_DATA_PATH);

        return TRUE;
    }

    return FALSE; 
}

size_t split(const std::string& txt, std::vector<std::string>& strs, char ch)
{
    size_t pos = txt.find(ch);
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while (pos != std::string::npos) {
        strs.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;

        pos = txt.find(ch, initialPos);
    }

    // Add the last one
    strs.push_back(txt.substr(initialPos, min(pos, txt.size()) - initialPos + 1));

    return strs.size();
}

std::vector<std::string> ReadLineData(bool modeTwo)
{
    while (true)
    {
        std::vector<std::string> inputVector;
        std::string inputString;
        
        if (modeTwo) 
        {
            std::cout << "Введите аргументы через пробел. Пример:\t MAGN SBER ... GAZP" << std::endl;
            ClearCin();
        }

        else 
        {
            std::cout << "Введите аргументы через пробел. Пример:\t MAGN 65,23 61,39 67,56 55,71" << std::endl;
            ClearCin();
        }
           
        std::getline(std::cin, inputString);
        size_t size = split(inputString, inputVector, ' ');
        
        if (modeTwo && size < 1)
        {
            continue; 
        }

        if (!modeTwo && size != 5)
        {
            std::cout << "ОШИБКА! Количество аргументов не равно 5!\n\n" << std::endl;
            continue;
        }

        return inputVector;
    }
}

std::string GenerateImage(std::string& text, std::string ticker)
{
    HWND hDestop = GetDesktopWindow();
    HDC hDestopDC = GetWindowDC(hDestop); // дескриптор контекста устройства 

    HBRUSH hRed = CreateSolidBrush(RGB(22, 26, 37));

    HFONT font = (HFONT)GetStockObject(DEVICE_DEFAULT_FONT);


    HDC hDC = CreateCompatibleDC(hDestopDC); // новый декриптор контекста 
    HBITMAP hBmp = CreateCompatibleBitmap(hDestopDC, 300, 300); // новый битмап 200х100 
    ReleaseDC(hDestop, hDestopDC); // освобождаем дескриптор 

    auto hDefaultBmp = SelectObject(hDC, (HGDIOBJ)hBmp); // поключаем битмап к контексту 

    RECT rectangle;
    SetRect(&rectangle, 0, 0, 300, 300);
    FillRect(hDC, &rectangle, hRed);

    /*std::string text = "Дата: 15.06.2021\nУровень: 770,0\nATR: 13,7\nСтратегия: Ложный пробой\nНаправление: Short ↓\nТВХ: 769,6 | 6,4 | 0,83% | 46,76%\nСтоп: 771,2 | 1,6 | 0,21% | 11,69%\nТейк: 764,8 | 4,8 | 0,62% | 35,07%\nСтоп/тейк: 1:3,00";*/
    
    std::vector<std::vector<char>> strings;
    strings.push_back(std::vector<char>());
    int lastIndex = 0;

    for (int i = 0; i < text.size(); i++)
    {
        if (text[i] == '\n')
        {
            strings.push_back(std::vector<char>());
            lastIndex++;
        }

        else
        {
            strings[lastIndex].push_back(text[i]);
        }
    }

    std::vector<std::string> fullStrings;

    for (auto elem : strings)
    {
        std::string tmpStr;

        for (int i = 0; i < elem.size(); i++)
        {
            tmpStr += elem[i];
        }

        fullStrings.push_back(tmpStr);
    }

    int lastYPos = 10;

    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(255, 255, 255));

    for (auto elem : fullStrings)
    {
        lastYPos += 20;
        TextOut(hDC, 30, lastYPos, StringToWSTRING(elem).c_str(), strlen(elem.c_str()));
    }

    SelectObject(hDC, (HGDIOBJ)hDefaultBmp);
    DeleteDC(hDC);

    std::string filePath = PROGRAM_DATA_PATH + '\\' + ticker + ".jpeg"; 

    CImage img;
    img.Attach(hBmp);
    img.Save(StringToWSTRING(filePath).c_str());

    return filePath; 
}

void Add() 
{
    std::vector<std::string> dataToWrite = ReadLineData(false); 
    
    int lines = 4; 
    
    for (int i = 0; i < lines; i++) 
    {
        std::string filePath = PROGRAM_DATA_PATH + "\\line" + std::to_string(i + 1) + ".json";
        std::ifstream file(filePath);
        bool has_data = false; 
        json tempJson; 


        if (file.is_open()) 
        {
            if (file.peek() != EOF) 
            {
                file >> tempJson;
                has_data = true; 
            }
        };

        if (has_data && tempJson.find(dataToWrite[0]) != tempJson.end())
        {
            tempJson.erase(dataToWrite[0]); 
            
        }

        double value = std::stod(dataToWrite[i+1]);
        tempJson[dataToWrite[0]] = value;

        std::ofstream outputFile(filePath);
        outputFile << std::setw(4) << tempJson << std::endl;
    }

    std::cout << "Успешно!\n\n" << std::endl;
}

void Delete() 
{
    std::vector<std::string> dataToDelete = ReadLineData(true);

    int lines = 4;

    for (int i = 0; i < lines; i++)
    {
        std::string filePath = PROGRAM_DATA_PATH + "\\line" + std::to_string(i + 1) + ".json";
        std::ifstream file(filePath);
        json tempJson;

        if (file.is_open())
        {
            if (file.peek() != EOF )
            {
                file >> tempJson;
                
                for (int j = 0; j < dataToDelete.size(); j++) 
                {
                    if (tempJson.find(dataToDelete[j]) != tempJson.end())
                    {
                        tempJson.erase(dataToDelete[j]);
                    }
                }

                std::ofstream outputFile(filePath);
                outputFile << std::setw(4) << tempJson << std::endl;   
            }
        };
    }
    std::cout << "Успешно!\n\n" << std::endl;
}

void Batch() 
{
    std::string filePath;
    std::cout << "Введите путь к файлу: напрмер C:\\Users\\DefaultUser\\Desktop\\example.txt" << std::endl;
    ClearCin();
    std::getline(std::cin, filePath);

    std::ifstream file(filePath);
    bool has_data = false;
    json fileData;

    if (file.is_open())
    {
        if (file.peek() != EOF)
        {
            file >> fileData;
            has_data = true; 
        }
    }

    int lines = 4;
    std::vector<std::string> keys{ "Level", "Entry", "Stop", "Take-profit" };

    for (int i = 0; i < lines; i++)
    {
        std::string filePath = PROGRAM_DATA_PATH + "\\line" + std::to_string(i + 1) + ".json";
        std::ifstream file(filePath);
        bool has_data = false;
        json tempJson;


        if (file.is_open())
        {
            if (file.peek() != EOF)
            {
                file >> tempJson;
                has_data = true;
            }
        };

        for (auto elem : fileData) 
        {
            if (has_data && tempJson.find(elem["Ticker"]) != tempJson.end())
            {
                std::string key(elem["Ticker"]); 
                tempJson.erase(key); 
            }

            auto val = std::move(elem[keys[i]]);
            std::string str(val);
            std::string output = boost::replace_all_copy(str, ".", ",");
          
            double value = std::stod(output);
            std::string key = elem["Ticker"]; 
            tempJson[key] = value;
        }

        std::ofstream outputFile(filePath);
        outputFile << std::setw(4) << tempJson << std::endl;
    }

    std::map<std::string, std::string> scenarios;
    std::string scenariosFilePath = PROGRAM_DATA_PATH + "\\scenarios.json"; 

    for (auto elem : fileData) 
    {
        std::string text = toLocale(elem["Smart-scenario"]);        
        std::string path = GenerateImage(text, elem["Ticker"]);
        scenarios.emplace(elem["Ticker"], path); 
    }

    json scenariosJSON(scenarios);

    std::ofstream outputFile(scenariosFilePath);
    outputFile << std::setw(4) << scenariosJSON << std::endl;

    std::cout << "Успешно!\n\n" << std::endl;
}

void Clear() 
{
    std::string result; 
    
    std::cout << "Выполнить очистку хранилища? (Введите YES и нажмите Enter): ";
    ClearCin();
    std::cin >> result; 

    if (result == "YES") 
    {
        int lines = 4;

        for (int i = 0; i < lines; i++)
        {
            std::string filePath = PROGRAM_DATA_PATH + "\\line" + std::to_string(i + 1) + ".json";
            std::ifstream file(filePath);
            json tempJson;


            if (file.is_open())
            {
                if (file.peek() != EOF)
                {
                    file >> tempJson;

                    tempJson.clear(); 

                    std::ofstream outputFile(filePath);
                    outputFile << std::setw(4) << tempJson << std::endl;
                }
            } 
        }

        std::string scenariosFilePath = PROGRAM_DATA_PATH + "\\scenarios.json";

        std::ifstream file(scenariosFilePath);
        json tempJson;

        if (file.is_open()) 
        {
            file >> tempJson;
            tempJson.clear();
            std::ofstream outputFile(scenariosFilePath);
            outputFile << std::setw(4) << tempJson << std::endl;
        }

        std::cout << "Успешно!\n\n" << std::endl;
    }

    else 
    {
        std::cout << "Вы отменили очистку хранилища.\n\n" << std::endl;
    }
}

int main()
{
    setlocale(LC_ALL, "ru");
    SetProgramDataDirectory(); 
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    while (true) 
    {
        const std::string addMethod= "ADD"; 
        const std::string deleteMethod = "DELETE";
        const std::string batchMethod = "BATCH"; 
        const std::string clearMethod = "CLEAR"; 
        const std::string quitMethod = "QUIT"; 

        bool outter = false; 

        std::string args; 
        std::vector<std::string> argsVector; 
        std::cout << "Выберите метод: ADD, BATCH, DELETE, CLEAR.\nВведите название метода и нажмите ENTER или введите QUIT и нажмите ENTER, чтобы выйти." << std::endl; 
        ClearCin(); 
        std::getline(std::cin, args);
        size_t size = split(args, argsVector, ' ');

        if (size > 1) 
        {
            std::cout << "Вы ввели некорректную клавишу\n\n";
            continue; 
        }

        std::string method = std::move(argsVector[0]); 

        if (method.size() != 0)
        {
            if (method == addMethod) 
            {
                std::cout << "\nВы выбрали метод ADD" << std::endl;
                Add();
                outter = true; 
            }

            if (method == deleteMethod)
            {
                std::cout << "\nВы выбрали метод DELETE" << std::endl;
                Delete();
                outter = true;
            }

            if (method == batchMethod)
            {
                std::cout << "\nВы выбрали метод BATCH" << std::endl;
                Batch();
                outter = true;
            }

            if (method == clearMethod) 
            {
                std::cout << "\nВы выбрали метод CLEAR" << std::endl;
                Clear();
                outter = true;
            }

            if (method == quitMethod)
            {
                exit(1);
            }

            else 
            {
                if (outter) 
                {
                    outter = false; 
                    continue; 
                }

                std::cout << "Вы ввели некорректную клавишу\n\n";
            }
        }
    }
}
