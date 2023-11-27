#include <iostream>
#include <Windows.h>
#include <Wininet.h>
#include <fstream>
#pragma comment(lib,"Wininet.lib")

using namespace std;

void ftpClient()
{
    HINTERNET hInternet = InternetOpen(L"FTP Client", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0); // подключение к интернету
    if (hInternet == NULL) 
    {
        cerr << "Unable to initialize WinINet!" << endl;
        return;
    }

  //  HINTERNET hFtpSession = InternetConnect(hInternet, L"test.rebex.net", 21, L"demo", L"password", INTERNET_SERVICE_FTP, 0, 0);
    HINTERNET hFtpSession = InternetConnect(hInternet, L"ftp.slackware.com", 21, L"anonymous", L"anonymous", INTERNET_SERVICE_FTP, 0, 0);
    if (hFtpSession == NULL) 
    {
        cerr << "Unable to connect to FTP server!" << endl;
        InternetCloseHandle(hInternet);
        return;
    }
    // Получение списка файлов на FTP-сервере и выводит информацию о каждом файле
    WIN32_FIND_DATA findFileData;  
    HINTERNET hFind = FtpFindFirstFile(hFtpSession, L"*", &findFileData, 0, 0);
    if (hFind != NULL) 
    {
        do {
            wcout << "File: " << findFileData.cFileName << ", Size: " << findFileData.nFileSizeLow << " bytes" << endl;
        } while (InternetFindNextFile(hFind, &findFileData) != 0);
        InternetCloseHandle(hFind);
    }
    // -----------------------------------------
    // переход каталог
    wstring directoryName;
    wcout << "Enter the directory: ";
    wcin >> directoryName;

    if (!FtpSetCurrentDirectory(hFtpSession, directoryName.c_str()))
    {
        cerr << "Unable to change directory!" << endl;
        InternetCloseHandle(hFtpSession);
        InternetCloseHandle(hInternet);
        return;
    }
    // Получение списка файлов в текущем каталоге
    WIN32_FIND_DATA findFileData1;
    HINTERNET hFind1 = FtpFindFirstFile(hFtpSession, L"*", &findFileData1, 0, 0);
    if (hFind != NULL)
    {
        do 
        {
            wcout << "File: " << findFileData1.cFileName << ", Size: " << findFileData1.nFileSizeLow << " bytes" << endl;
        } while (InternetFindNextFile(hFind1, &findFileData1) != 0);
        InternetCloseHandle(hFind1);
    }
    // ------------------------------------------
    // Запрос имени файла для скачивания у пользователя
    wstring fileNameToDownload;
    wcout << "Enter the name of the file you want to download: ";
    wcin >> fileNameToDownload;

    // Скачивание файла
    HINTERNET hFile = FtpOpenFile(hFtpSession, fileNameToDownload.c_str(), GENERIC_READ, FTP_TRANSFER_TYPE_BINARY, 0); //открывает файл на FTP-сервере для чтения
    if (hFile != NULL) 
    {
        // Определяем путь для сохранения файла в папке проекта
        wstring filePath = L".\\" + fileNameToDownload;
        HANDLE hLocalFile = CreateFile(filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hLocalFile != INVALID_HANDLE_VALUE) 
        {
            DWORD bytesRead;
            const DWORD bufferSize = 1024;
            BYTE buffer[bufferSize];
            while (InternetReadFile(hFile, buffer, bufferSize, &bytesRead) && bytesRead > 0) 
            {
                // Запись данных в локальный файл
                DWORD bytesWritten;
                WriteFile(hLocalFile, buffer, bytesRead, &bytesWritten, NULL);
            }
            CloseHandle(hLocalFile);
        }
        InternetCloseHandle(hFile);
    }
    else 
    {
        cerr << "Unable to download file!" << endl;
    }
    InternetCloseHandle(hFtpSession);
    InternetCloseHandle(hInternet);
}
void httpClient()
{
    HINTERNET hInternet = InternetOpen(L"HTTP Client", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet == NULL)
    {
        cerr << "Unable to initialize WinINet!" << endl;
        return;
    }
    //отвечает за HTTP-соединение и загрузку изображения по указанному URL-адресу

    HINTERNET hHttpSession = InternetOpenUrl(hInternet, L"https://w.forfun.com/fetch/da/daf8eb568fea522f6701fb9c66378cdc.jpeg", NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (hHttpSession == NULL)
    {
        cerr << "Unable to connect to HTTP server!" << endl;
        InternetCloseHandle(hInternet);
        return;    
    }

    // Получение заголовков HTTP запроса 
    DWORD bufferSize = 0;
    HttpQueryInfo(hHttpSession, HTTP_QUERY_RAW_HEADERS_CRLF, NULL, &bufferSize, NULL); 
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        wchar_t* headers = new wchar_t[bufferSize / sizeof(wchar_t)];
        if (HttpQueryInfo(hHttpSession, HTTP_QUERY_RAW_HEADERS_CRLF, headers, &bufferSize, NULL))
        {
            wcout << "HTTP Headers:\n" << headers << endl;
        }
        delete[] headers;
    }

    // Создание файла для сохранения изображения
    wstring filePath = L"image.jpeg";
    ofstream outputFile(filePath, ios::binary); //Открывается файл для записи данных в бинарном режиме
    if (!outputFile.is_open())
    {
        cerr << "Unable to create file for image!" << endl;
        InternetCloseHandle(hHttpSession);
        InternetCloseHandle(hInternet);
        return;
    }
    
    const DWORD bufferSizeForDownload = 1024;
    BYTE* buffer = new BYTE[bufferSizeForDownload];
    DWORD bytesRead;
    while (InternetReadFile(hHttpSession, buffer, bufferSizeForDownload, &bytesRead) && bytesRead > 0) //читает данные из HTTP-сессии и записывает их в файл
    {
        // Запись данных в файл
        outputFile.write(reinterpret_cast<char*>(buffer), bytesRead);
    }

    delete[] buffer;
    outputFile.close();
    InternetCloseHandle(hHttpSession);
    InternetCloseHandle(hInternet);
}

int main() 
{
    ftpClient();
    httpClient();
    return 0;
}
