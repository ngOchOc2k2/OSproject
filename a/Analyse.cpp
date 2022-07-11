#define UNICODE
#pragma once
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <windows.h>
#include <time.h>
#include <sys/types.h>
#include <psapi.h>
#include <tlhelp32.h>

#define MAX_CWD_LENS 512
#define MAX_BUFFER_SIZE 256
#define MAX_TOK_BUFSIZE 256
#define TOKEN_DELIMETERS " \t\r\n\a"

using namespace std;

/////////////////////////////////////
////////// Xử lý câu lệnh ///////////
/////////////////////////////////////
 wchar_t *GetWC( char *c)
{
    /**
     * Chuyển char* sang wchar_t*
     * 
     * 
     */
    const size_t cSize = strlen(c)+1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs (wc, c, cSize);

    return wc;
}
char* getChar(wchar_t* c){
    /**
     * @brief Chuyển wchar_t* sang char*
     * 
     */
    const int wSize = wcslen(c) + 1;
    char* s = new char[wSize];
    wcstombs(s,c,wSize);
    return s;
}
/**
 * Sửa lỗi gõ cách câu lệnh
 * VD: C:\Users\Admin> $           cd E:\ 
 * ==> C:\Users\Admin> $ cd E:\
 * 
 **/
void fixCmd(char* cmd){
    /**
     * Sửa lỗi gõ cách câu lệnh
     * VD: C:\Users\Admin> $           cd E:\ 
     * ==> C:\Users\Admin> $ cd E:\
     * 
     **/
    int length = strlen(cmd);
    int first;
    for(int i=0; i<length; i++){
        if((cmd)[i] != ' '){
            first = i;
            break;
        }
    }
    int real_len = length - first, i=0;
//			"     Ta Ton" -> "Ta Tona Ton"
    while(real_len--){
        (cmd)[i] = (cmd)[i+first];
        i++;
    }
    (cmd)[length - first] = '\0'; //"Ta Tona Ton" -> "Ta Ton"
}

/**
 * In ra màn hình console đường dẫn 
 * (VD: C:\Users\Admin\.....> $ *phần tiếp này là lệnh*)
 **/
int printPrompt(TCHAR* cur_directory){
    /**
     * In ra màn hình console đường dẫn 
     * (VD: C:\Users\Admin\.....> $ *phần tiếp này là lệnh*)
     **/
    if(GetCurrentDirectory(300,cur_directory) == 0){
        cout << "Reading of current working directory failed.\n";
        return -1;
    }
    printf("%S> $", cur_directory);   
    return 1; 
}

/**
 * Chia câu lệnh thành các đoạn ngăn cách bởi ký tự ngăn (" ","\t","\r","\n","\a")
 * 
 **/
char **separate_line(char *line){
    /**
     * Chia câu lệnh thành các đoạn ngăn cách bởi ký tự ngăn (" ","\t","\r","\n","\a")
     * 
     **/
    int bufsize = MAX_TOK_BUFSIZE;
    int position = 0;
    char **tokens = (char**)malloc(bufsize*sizeof(char*));
    char *token;

    if(!tokens){
        cout << "Allocation Failed" << endl; /* Không đủ bộ nhớ cấp phát */
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOKEN_DELIMETERS); /*Con trỏ trỏ tới args[0] của lệnh cmd VD: cd, dir*/
    while(token != NULL){
        tokens[position] = token; /* Lưu các con trỏ chứa thành phần của lệnh cmd */
        // cout << tokens[position] << endl;
        position++; 
        if(position >= bufsize){ /* số thành phần args[i] trong lệnh cmd lớn hơn số bufsize dự tính*/
            bufsize += MAX_TOK_BUFSIZE; /* Tăng số bufsize */
            tokens = (char**)realloc(tokens, bufsize); /* Cấp phát thêm bộ nhớ cho tokens */
            if(!tokens){
                cout << "Allocation Failed" << endl; /* Không đủ bộ nhớ cấp phát */
                exit(EXIT_FAILURE);
            } 
        }
        token = strtok(NULL,TOKEN_DELIMETERS); /* Trỏ token tới thành phần args tiếp theo trong của cmd*/
    }
    tokens[position] = NULL; /*Kết thúc danh sách mã*/
    return tokens;
}

/**
 * Chuẩn hóa đường dẫn [path], tránh trường hợp như:
 * VD: cd E:\New folder\ => [path] = 'E:\New'
 **/
char* combinePath (char **args, int start){ // wchar_t*
    /**
     * Chuẩn hóa đường dẫn [path], tránh trường hợp có dấu ngăn cách như:
     * VD: cd E:\New folder\ => [path] = 'E:\New'
     **/
    char *path = (char *)calloc(MAX_CWD_LENS,sizeof(char)); /*Các thành phần không bị ngăn cách*/
    char *pivot = (char*)" "; /* Dấu ngăn cách */
    /* Kết hợp từng thành phần trong path */
    path = strcat(path, args[start]);
    int i = start + 1;
    while(args[i] != NULL){
        path = strcat(path,pivot);
        path = strcat(path, args[i]);
        ++i;
    }
    return path;
}

/**
 * Đổi màu chữ (Có thể có hoặc k cơ bản là cũng k có ảnh hưởng gì chỉ là trang trí )
 * 
 **/
void setColor(char* color) {
    /*
    * Đổi màu lời gọi đầu 
    */
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    WORD saved_attributes;

    /* Lưu các thuộc tính */
    // GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    // saved_attributes = consoleInfo.wAttributes;

    if (strcmp(color,"green")==0) {
      SetConsoleTextAttribute(hConsole,FOREGROUND_GREEN | FOREGROUND_INTENSITY);
     
    }
    if (strcmp(color,"blue")==0) {
      SetConsoleTextAttribute(hConsole,FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    }
    if (strcmp(color,"red")==0) {
      SetConsoleTextAttribute(hConsole,FOREGROUND_RED | FOREGROUND_INTENSITY);
    }
}