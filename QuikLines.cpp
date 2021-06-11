﻿#include <iostream>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <map>
#include <vector>
#include <windows.h>
#include <shlobj.h>
#include "boost/filesystem.hpp"

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

int main()
{
    setlocale(LC_ALL, "ru");
    SetProgramDataDirectory(); 

    while (true) 
    {
        const std::string addMethod= "ADD"; 
        const std::string deleteMethod = "DELETE"; 
        const std::string quitMethod = "QUIT"; 

        bool outter = false; 

        std::string args; 
        std::vector<std::string> argsVector; 
        std::cout << "Выберите метод: ADD, DELETE, QUIT.\nВведите название метода и нажмите ENTER" << std::endl; 
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