//
//  Interpreter.h
//  interpreter
//  Interpreter模块，用于读取用户指令，与api模块交互；数据结构和函数的设计在后续编码过程中还会继续修改
//  Created by 李了 on 14/11/1.
//  Copyright (c) 2014年 李了. All rights reserved.
//

#ifndef interpreter_Interpreter_h
#define interpreter_Interpreter_h

#include "PublicClass.h"

struct command
{
    int operation;                   //指令代号，见myMacro.h
    int objectType;                  //操作对象代号，见myMacro.h
    vector<string> objectName;               //操作对象名
};

class Interpreter
{
private:
    bool quitflag;
    int currentPosition;              //当前处理到input字符串的位置
    vector<Attribute> column;         //列
    vector<Conditions> condition;	  //条件语句
    vector<string> insert;		      //要插入的值链表
    Table currentTable;               //当前语句涉及到的表
    int primaryKeyPosition;           //标记primary key的位置
    int uniqueKeyPostion;             //标记unique key的位置
public:
    Interpreter();
    ~Interpreter();
    string originalInput;             //原始的输入，以；结束
    command currentCommand;           //当前执行的指令
    void init();                         //初始化
    void getInput();                     //获取输入指令
    void converseCase();                 //将输入转化为大写
    string findNextToken(int firstpos, string input); //从pos位置开始找input中的下一个关键词（用空格分隔关键词）
    bool judgeCommandType(string input);  //解析输入命令，判断命令类型，有错返回false
    vector<string> splitConditions(int firstpos, string input);//按“and”或“&”分割字符串
    bool parseCondition(vector<string> conditions);//处理条件，生成条件对象
    CONDITION_TYPE judgeConditionType(string conditionType); //判断符号，如果不在指定的符号中则返回－1
    vector<string> splitCreateTable(string input);
    bool parseCreateTable(vector<string> createtable);
    bool parseCommand(string input); //根据指令类型，生成指令结构体；判断有无语法错误
    bool checkObjectName(string objectName);//检查对象名是否合法（只包含英文字母数字下划线且以英文字母开头，
                                            //不能与保留字重复）
    string deleteBrackets(string input); //删去input两端的括号和多余空格
    bool isValidInt(const string s);          //检查是否是整数
    bool isValidFloat(const string s);        //检查是否是浮点数
    bool checkFile(string filename);     //检查文件
    void outputHelp(int errorType);      //如果命令有错，输出相应提示
    bool executeCommand(); //如果指令无误，执行指令（调用其他模块）
    vector<string> readFile(string filename);//将文件中语句按‘；’分割并返回
    bool quit();//判断quitflag的值
    void test();
};















#endif
