//
//  Interpreter.cpp
//  interpreter
//
//  Created by 李了 on 14/11/1.
//  Copyright (c) 2014年 李了. All rights reserved.
//

#include "Interpreter.h"
#include "myMacro.h"
#include "PublicClass.h"
#include "APIManager.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>
using namespace std;
APIManager API;


void Interpreter:: init()
{
    static bool ini = false;
    if(ini==false)
        cout<<"Welcome to MiniSQL"<<endl<<"Sever version 0.0.1"<<endl<<endl<<"Copyright (c) 2014, 2014, Crab, Liliao, Starclam. All rights reserved."<<endl<<endl;
    ini=true;
    quitflag = false;
    currentPosition = 0;
    primaryKeyPosition = -1;
    uniqueKeyPostion = -1;
    originalInput.clear();
    column.clear();
    condition.clear();
    insert.clear();
    currentCommand.operation = -1;
    currentCommand.objectType = -1;
    currentCommand.objectName.clear();
    currentTable.attributes.clear();
    currentTable.attriNum = -1;
    currentTable.blockNum = -1;
    currentTable.eachRecordLength = -1;
    currentTable.freeNum = 0;//
    
}

bool Interpreter::quit()
{
    return quitflag;
}

//总长度不超过11位
bool Interpreter:: isValidInt(const string s)
{
    if(s.size() == 0 || s.size() > 11)
    {
        return false;
    }
    else
    {
        if((s[0] != '-') &&(!isdigit(s[0])))
        {
            return false;
        }
        else
        {
            for(size_t i = 1; i < s.size(); ++i)
            {
                if(!isdigit(s[i]))
                    return false;
            }
        }
    }
    return true;
}

//判断字符串中是否有数字之外字符（首位负号除外）；判断字符串中小数点数量是否超过1；判断小数点之前是否超过33位
bool Interpreter:: isValidFloat(const string s)
{
    int count = 0;
    if(s.size() == 0)
    {
        return false;
    }
    else
    {
        if((s[0] != '-') &&(!isdigit(s[0])))
        {
            return false;
        }
        else
        {
            for(size_t i = 1; i < s.size(); ++i)
            {
                if((!isdigit(s[i]) && (s[i]!= '.')))
                    return false;
                else
                {
                    if(s[i] == '.')
                        count++;
                }
            }
            if(count > 1)
                return false;
            else
            {
                for(size_t i = 1; i < s.size(); ++i)
                {
                    if(s[i] == '.')
                    {
                        string beforeDot = s.substr(0, i);
                        //string afterDot = s.substr(i+1, s.size() - i - 1);
                        if(beforeDot.size()>33)
                            return false;
                    }
                }
            }
        }
    }
    return true;
}

void Interpreter:: getInput()
{
    cout<<"minisql> ";
    originalInput.clear();
    string tmpSentence;
    getline(cin,tmpSentence);
    while(tmpSentence.find(';')== string::npos)
    {
        originalInput.append(tmpSentence);
        originalInput.append(" ");
        tmpSentence.clear();
        getline(cin,tmpSentence);
    }
    originalInput.append(tmpSentence);
    for(size_t i = 0; i < originalInput.size(); ++i)
    {
        if(originalInput[i] == '\t' || originalInput[i] == '\n' || originalInput[i] == ';')
            originalInput[i] = ' ';
    }
}

vector<string> Interpreter:: readFile(string filename)
{
    ifstream inputfile;
    vector<string> fileCommands;
    string command = "";
    fileCommands.clear();
    inputfile.open(filename);
    if(!inputfile)
    {
        outputHelp(EXEFILERR);
        return fileCommands;
    }
    else
    {
        while(!inputfile.eof())
        {
            getline(inputfile, command, ';');
            //将command中的空白字符转化为空格
            for(size_t i = 0; i < command.size(); ++i)
            {
                if(command[i] == '\n'||command[i] == '\t')
                    command[i] = ' ';
            }
            fileCommands.push_back(command);
        }
        if(fileCommands.back() == "")
        {
            fileCommands.pop_back();
        }
    }
    return fileCommands;
}

void Interpreter:: converseCase()
{
    for(size_t i = 0; i < originalInput.size(); ++i)
    {
        originalInput[i] = toupper(originalInput[i]);
    }
}

string Interpreter:: findNextToken(int firstpos, string input)
{
    int j = 0;
    string token;
    while(input[firstpos] == ' ')
    {
        ++firstpos;               //firstpos指向初始位置后第一个非空格
    }
    for(j = firstpos; j < input.size(); ++j)
    {
        if(input[j] == ' ')
        {
            break;               //j标记关键字后第一个空格的位置
        }
    }
    currentPosition = j;
    token = input.substr(firstpos,j-firstpos);
    //cout<<"test find next token: "<< token<<endl;
    return token;
}

//去除input中的（）和两端空格
string Interpreter::deleteBrackets(string input)
{
    string result;
    result = input.substr(input.find("(")+1, input.find(")") - input.find("(") - 1);
    if(result.size() > 0)
    {
        result.erase(0,result.find_first_not_of(" "));
        result.erase(result.find_last_not_of(" ") + 1);
    }
    return result;
}

vector<string> Interpreter:: splitConditions(int firstpos, string input)
{
    string substring;
    if(input.empty())
    {
        substring = input;
    }
    else
    {
        substring = input.substr(firstpos, input.size() - 1 -firstpos);
        substring = substring.erase(0, substring.find_first_not_of(" "));//去掉开头空格
    }
    vector<string> conditions;
    if(substring.find("AND")== string::npos)//单个条件
    {
        conditions.push_back(substring);
    }
    else //多个条件
    {
        size_t last = 0;
        size_t index = substring.find("AND",last);//查找从last开始第一个AND,index纪录A的位置
        while(index!=string::npos)
        {
            conditions.push_back(substring.substr(last, index - last));
            last = substring.find_first_not_of(" ", index+3);//AND之后第一个非空格
            index = substring.find("AND",last);
        }
        if(index - last > 0)
        {
            conditions.push_back(substring.substr(last, index - last));
        }
    }
//    for(size_t i = 0; i < conditions.size(); ++i)
//    {
//        cout<<"condition: "<<conditions[i]<<endl;
//    }
    return conditions;
}

//condition之间用and连接
bool Interpreter:: parseCondition(vector<string> conditions)
{
    vector<string> subConditions;//每一条条件中的子字符串
    for(size_t i = 0; i < conditions.size(); ++i)
    {
        conditions[i].append(" ");
        subConditions.clear();
        size_t last = 0;
        size_t index = conditions[i].find_first_of(" ", last);
        while(index!=string::npos)
        {
            subConditions.push_back(conditions[i].substr(last, index - last));
            last = conditions[i].find_first_not_of(" ", index);
            index = conditions[i].find_first_of(" ", last);
        }
        if(subConditions.size() > 3 )
        {
            outputHelp(WHEREERROR);//不只拆分出三个子字符串
            return false;
        }
        Conditions tmpCondition;
        tmpCondition.attribute = subConditions[0];
//        cout<<subConditions[0]<<endl;
        tmpCondition.condition_type = judgeConditionType(subConditions[1]);
        //去掉字符串两端的引号
        if(subConditions[2].size()>2)
        {
            size_t j = subConditions[2].size() - 1;
            if(subConditions[2][0]== '\'' && subConditions[2][j]== '\'')
               {
                   subConditions[2] = subConditions[2].substr(1,j-1);
               }
        }
        tmpCondition.attributeValue = subConditions[2];
//        cout<<"test delete \':"<<tmpCondition.attributeValue<<endl;
        condition.push_back(tmpCondition);
    }
    return true;
}

//将create table语句按照','分割
vector<string> Interpreter:: splitCreateTable(string input)
{
    size_t fleft = input.find('(');
    size_t lright = input.find_last_of(')');
    if(lright>fleft+2)
    input = input.substr(fleft+1, lright -fleft-1);
    
    vector<string> sentences;
    if(input.find(',')==string::npos)//没有‘，’，说明只有一条语句
    {
        sentences.push_back(input);
    }
    else
    {
        size_t last = 0;
        size_t index = input.find_first_of(',', last);
        while(index!=string::npos)
        {
            sentences.push_back(input.substr(last, index - last));
            sentences.back() = (sentences.back()).erase(0, (sentences.back()).find_first_not_of(" "));
            last = input.find_first_not_of(',', index);
            index = input.find_first_of(',', last);
        }
        if(last< input.size() - 1)
        {
            sentences.push_back(input.substr(last, input.size() - last -1));
            sentences.back() = (sentences.back()).erase(0, (sentences.back()).find_first_not_of(" "));
        }
        else
        {
            sentences.clear();//格式错误，返回空的vector
        }
    }
    return sentences;

}

bool Interpreter:: parseCreateTable(vector<string> createtable)
{
    if(!createtable.size() || createtable.size() > 32)//空值或者超过限制数量
        return false;
    else
    {
        vector<string> tokens;
        vector<string> attrname;
        for(size_t i = 0; i < createtable.size(); ++i)
        {
            Attribute tmpattribute;
            tokens.clear();
            //按照空格拆分
            size_t last = 0;
            size_t index = createtable[i].find_first_of(' ', last);
            while(index!=string::npos)
            {
                tokens.push_back(trim(createtable[i].substr(last, index - last)));
                last = createtable[i].find_first_not_of(' ', index);
                index = createtable[i].find_first_of(' ', last);
            }
            if(last< createtable[i].size() - 1)
            {
                tokens.push_back(trim(createtable[i].substr(last, createtable[i].size() - last)));
            }
            if(tokens.size() < 2)
                return false;
            
            for(size_t j = 0; j < tokens.size(); ++j)
            {
//                cout<<j<<" "<<tokens[j]<<endl;
                if(tokens[0] != "PRIMARY")
                {
                    tmpattribute.name= tokens[0];
                    attrname.push_back(tokens[0]);
                    switch(tokens[1][0])
                    {
                        case 'C':
                            if(!(tokens[1][1]=='H' && tokens[1][2]=='A' &&tokens[1][3]=='R'))
                            {
                                outputHelp(TYPEERROR);
                                return false;
                            }
                            else
                            {
                                tmpattribute.type = CHAR;
                                size_t left = tokens[1].find('(');
                                size_t right = tokens[1].find(')');
                                string sub = trim(tokens[1].substr(left+1, right - left - 1));
                                stringstream ss;
                                ss << sub;
                                ss >> tmpattribute.length;
                                if(tmpattribute.length > 255)
                                {
                                    outputHelp(CHARBOUD);
                                    return false;
                                }
                            }
                            break;
                        case 'I':
                            if(!(tokens[1][1]=='N' && tokens[1][2]=='T'))
                            {
                                outputHelp(TYPEERROR);
                                return false;
                            }
                            else
                                tmpattribute.type = INT;
                            break;
                        case 'F':
                            if(!(tokens[1][1]=='L' && tokens[1][2]=='O' &&tokens[1][3]=='A'&& tokens[1][4] == 'T'))
                            {
                                outputHelp(TYPEERROR);
                                return false;
                            }
                            else
                                tmpattribute.type = FLOAT;
                            break;
                    }
                    if(tokens.size() == 3)//unique
                    {
                        if(trim(tokens[2]) == "UNIQUE")
                        {
                            uniqueKeyPostion = (int)i;
                        }
                        else
                        {
                            return false;
                        }
                    }
                }
                else
                {
                    if(tokens[1] == "KEY" && tokens.size() == 3)
                    {
                        tokens[2] = trim(deleteBrackets(tokens[2]));
                        int k = 0;
                        for(k = 0; k < attrname.size(); ++k)//判断primarykey的位置 默认这一句在最后
                        {
                            if(attrname[k] == tokens[2])
                            {
                                primaryKeyPosition = k;
                                break;
                            }
                            if(k == attrname.size())//没有找到对应的name
                            {
                                outputHelp(VOIDPRI);
                                return false;
                            }
                        }
                    }
                    else
                        return false;
                }
            }
            if(tmpattribute.name != "")
            column.push_back(tmpattribute);
        }
        if(attrname.size() < createtable.size() - 1)//不止一个primary key
            return false;
    }
    return true;
}

bool Interpreter:: judgeCommandType(string input)
{
    string operation;
    string object;
    operation = findNextToken(0, input);//到第一个空格为止  分出操作符
    if (operation == "CREATE")
    {
        currentCommand.operation = CREATE;
        return true;
    }
    if (operation == "DROP")
    {
        currentCommand.operation = DROP;
        return true;
    }
    if (operation == "DELETE")
    {
        currentCommand.operation = DELETE;
        return true;
    }
    if (operation == "INSERT")
    {
        currentCommand.operation = INSERT;
        return true;
    }
    if (operation == "EXECFILE")
    {
        currentCommand.operation = EXECFILE;
        return true;
    }
    if (operation ==  "QUIT")
    {
        currentCommand.operation = QUIT;
        return true;
    }
    if (operation == "USE")
    {
        currentCommand.operation = USE;
        return true;
    }
    if(operation == "SELECT")
    {
        currentCommand.operation = SELECT;
        return true;
    }
    return false;
}

CONDITION_TYPE Interpreter:: judgeConditionType(string conditionType)
{
    if(conditionType == "=")
        return EQUAL;
    if(conditionType == "<>")
        return NOT_EQUAL;
    if(conditionType == ">")
        return GREATER;
    if(conditionType == ">=")
        return GREATER_EQUAL;
    if(conditionType == "<")
        return SMALLER;
    if(conditionType == "<=")
        return SMALLER_EQUAL;
    if(conditionType == "EXIST")
        return EXIST;
    return ERROR;
}

bool Interpreter::checkObjectName(string objectName)
{

        if (objectName[0]>'Z'||objectName[0]<'A')
        {
            return false;
        }
        else
        {
            for(size_t i = 1; i < objectName.size(); ++i)
            {
                if((objectName[i]>='0' && objectName[i]<='9') || objectName[i] == '_' ||(objectName[i]>='A' && objectName[i]<='Z') || (objectName[i] == '-'))
                {
                    continue;
                }
                else
                {
                    return false;
                }
            }
        }
    return true;
}

bool Interpreter::parseCommand(string input)
{
    string object;
    string token;
    string objname;
    if (currentCommand.operation < 0)
    {
        outputHelp(UNKNOWN);
        return false;
    }
    else
    {
        switch (currentCommand.operation)
        {
            case USE:
                objname = findNextToken(currentPosition, input);
                if(checkObjectName(objname))
                {
                    currentCommand.objectName.push_back(objname);//表名
                    currentCommand.objectType = TABLE;
                    currentTable.name = objname;
                }
                else
                {
                    outputHelp(INVALIDNAME);
                    return false;
                }
                break;
                
            case CREATE:
                object = findNextToken(currentPosition, input);//找出第二个关键字并去除开头的空格
                if ((object == "INDEX")||(object == "TABLE"))
                {
                    if(object == "INDEX")
                    {
                        if(input.find("ON") && (input.find("(")!=string::npos) && (input.find(")")!= string::npos))
                        {
                            currentCommand.objectType = INDEX;
                            objname = findNextToken(currentPosition, input);
                            if(checkObjectName(objname))
                                currentCommand.objectName.push_back(objname);//索引名
                            else
                            {
                                outputHelp(INVALIDNAME);
                                return false;
                            }
                            if(findNextToken(currentPosition, input) != "ON")//如果索引名之后不是紧跟着on
                            {
                                outputHelp(CREINDERR);
                                return false;
                            }
                            objname = findNextToken(currentPosition, input);
                            if(checkObjectName(objname))
                            {
                                currentCommand.objectName.push_back(objname);//表名
                            }
                            else
                            {
                                outputHelp(INVALIDNAME);
                                return false;
                            }
                            objname = deleteBrackets(input.substr(currentPosition,input.size() - 1));//列名
                            if(checkObjectName(objname))
                            {
                                 currentCommand.objectName.push_back(objname);//列名
                                 currentTable.name = currentCommand.objectName[1];
                            }
                            else
                            {
                                outputHelp(INVALIDNAME);
                                return false;
                            }
                        }
                        else//如果没有on和（）
                        {
                            outputHelp(CREINDERR);
                            return false;
                        }
                    }
                    
                    if (object == "TABLE")
                    {
                        currentCommand.objectType = TABLE;
                        objname = findNextToken(currentPosition, input);//表名
                        if(checkObjectName(objname))
                        {
                            currentCommand.objectName.push_back(objname);
                            if(!parseCreateTable(splitCreateTable(input)))//createtable输入出错
                            {
                                outputHelp(CRETABERR);
                                return false;
                            }
                            else
                            {
                                currentTable.name = objname;//记录当前表格信息
                                currentTable.attriNum = (int)column.size();
                                for(size_t ii = 0; ii < column.size(); ++ii)
                                {
                                    Attribute tmpattr;
                                    tmpattr.name = column[ii].name;
                                    tmpattr.type = column[ii].type;
                                    tmpattr.length = column[ii].length;
                                    currentTable.attributes.push_back(tmpattr);
                                }
                                if(primaryKeyPosition != -1)
                                {
                                    currentTable.primaryKey = column[primaryKeyPosition].name;
                                    currentTable.attributes[primaryKeyPosition].isUnique = true;
                                    currentTable.attributes[primaryKeyPosition].isPrimaryKey = true;
                                }
                                if(uniqueKeyPostion != -1)
                                {
                                    currentTable.attributes[uniqueKeyPostion].isUnique = true;
                                }
                                
                            }
                        }
                        else
                        {
                            outputHelp(INVALIDNAME);
                            return false;
                        }
                        
                    }
                }
                else //INDEX TABLE 以外的词
                {
                    outputHelp(UNKNOWN);
                    return false;
                }
                break;
                
            case DROP://完成
                object = findNextToken(currentPosition, input);//找出第二个关键字并去除开头的空格
                if ((object == "INDEX")||(object == "TABLE"))
                {
                    if(object == "INDEX")
                    {
                        currentCommand.objectType = INDEX;
                    }
                    if (object == "TABLE")
                    {
                        currentCommand.objectType = TABLE;
                    }
                    objname = findNextToken(currentPosition, input);
                    if(checkObjectName(objname))
                    {
                        currentCommand.objectName.push_back(objname);
                    }
                    else
                    {
                        outputHelp(INVALIDNAME);
                        return false;
                    }
                }
                else //INDEX TABLE 以外的词
                {
                    outputHelp(UNKNOWN);
                    return false;
                }
                break;
            case DELETE:
                token = findNextToken(currentPosition, input);
                if (token != "FROM")
                {
                    outputHelp(DELETEERR);
                    return false;
                }
                else
                {
                    objname = findNextToken(currentPosition, input);//表名
                    if (checkObjectName(objname))
                    {
                        currentCommand.objectName.push_back(objname);
                        currentCommand.objectType = TABLE;
                        //currentTable.name = currentCommand.objectName[0];
                    }
                    else
                    {
                        outputHelp(INVALIDNAME);
                        return false;
                    }
                    token = findNextToken(currentPosition, input);
                    if (token == "")//不带where从句
                    {
                        return true;
                    }
                    else
                    {
                        if (token == "WHERE")//带条件语句，用and或&连接
                        {                            
                            if(!parseCondition(splitConditions(currentPosition, input)))
                            {
                                outputHelp(WHEREERROR);
                            }
                        }
                    }
                }
                break;
            case INSERT:
                token = findNextToken(currentPosition, input);
                if (token != "INTO")
                {
                    outputHelp(INSERTERR);
                    return false;
                }
                else
                {
                    objname = findNextToken(currentPosition, input);//表名
                    if(checkObjectName(objname))
                    {
                        currentTable.name = objname;
                        currentCommand.objectType = TABLE;
                        currentCommand.objectName.push_back(objname);
                        if(input.find('(')&& input.find(')'))
                        {
                            string sub = input.substr(input.find('(')+1,input.find(')')-input.find('(')-1);
                            size_t last = 0;
                            size_t index = sub.find_first_of(',',last);//查找从last开始第一个','
                            while(index!=string::npos)
                            {
                                insert.push_back(sub.substr(last, index - last));
                                last = sub.find_first_not_of(',', index);
                                index = sub.find_first_of(',',last);
                            }
                            if(index - last > 0)
                            {
                                insert.push_back(sub.substr(last, index - last));
                                if(insert.back().size()>257)
                                {
                                    outputHelp(CHARBOUD);
                                    return false;
                                }
                            }

                        }
                        else
                        {
                            outputHelp(INSERTERR);
                            return false;
                        }
                    }
                    else
                    {
                        outputHelp(INVALIDNAME);
                        return false;
                    }
                }
                break;
            case SELECT:
                token = findNextToken(currentPosition, input);
                if (token != "*")
                {
                    outputHelp(SELERR);
                    return false;
                }
                else
                {
                    token = findNextToken(currentPosition, input);
                    if (token != "FROM")
                    {
                        outputHelp(SELERR);
                        return false;
                    }
                    else
                    {
                        objname = findNextToken(currentPosition, input);
                        if (checkObjectName(objname) == true)
                        {
                            currentCommand.objectName.push_back(objname);
                            currentCommand.objectType = TABLE;
                            currentTable.name = objname;
                        }
                        else
                        {
                            outputHelp(INVALIDNAME);
                            return false;
                        }
                        if(findNextToken(currentPosition, input) == "WHERE")
                        {
                            if(parseCondition(splitConditions(currentPosition, input)) == false)
                            {
                                outputHelp(WHEREERROR);
                                return false;
                            }
                        }
                    }
                }
                break;
            case EXECFILE://完成
            objname= findNextToken(currentPosition, input);
                    currentCommand.objectName.push_back(objname);
                    currentCommand.objectType = FILE;            
            break;
            case QUIT://检查quit后是否还有其他字符
                if(findNextToken(currentPosition, input).size())
                return false;
                quitflag = true;
                break;
        }
    }
    return true;
}

bool Interpreter:: executeCommand()
{
    switch (currentCommand.operation)
    {
        case CREATE:
            switch (currentCommand.objectType)
        {
            case TABLE:
            {
                if(API.existTable(currentTable.name)) //如果这张表已经存在
                {
                    outputHelp(SAMETABLE);
                    return false;
                }
                if(primaryKeyPosition == -1)
                {
                    outputHelp(NOPRIKEY);
                    return false;
                }
                //string indexName = currentTable.name + "-" + currentTable.primaryKey;
                API.creatTable(currentTable);
                //API.createIndex(indexName, currentTable.name, currentTable.primaryKey);

                cout<<"The table has been created"<<endl;
                break;}
            case INDEX:
                if(API.existTable(currentCommand.objectName[1]) == false)//表不存在
                {
                    outputHelp(TABLEERROR);
                    return false;
                }
                if(API.exsitAttrTable(currentCommand.objectName[1], currentCommand.objectName[2]) == false)
                {
                    outputHelp(NOATTRI);
                    return false;
                }
                if(API.existIndex(currentCommand.objectName[0]) == true)//索引名已经存在
                {
                    outputHelp(SAMEINDEX);
                    return false;
                }
                if (API.existIndexAttr(currentCommand.objectName[1], currentCommand.objectName[2]) == true)//如果属性上已经有索引
                {
                    outputHelp(COLUMNERROR);
                    return false;
                }
                if(API.isUnique(currentCommand.objectName[1], currentCommand.objectName[2]) == false)//如果属性不是unique
                {
                    outputHelp(VOIDUNI);
                    return false;
                }
                API.createIndex(currentCommand.objectName[0],currentCommand.objectName[1], currentCommand.objectName[2]);
                cout<<"The index has been created"<<endl;
                break;
        }
            break;
        case DROP:
            switch (currentCommand.objectType)
        {
            case TABLE:
                if(API.existTable(currentCommand.objectName[0])==false)//如果这张表不存在
                {
                    outputHelp(TABLEERROR);
                    return false;
                }
                else
                API.dropTable(currentTable.name);
                cout<<"The table has been droped" << endl;
                break;
            case INDEX:
                if(API.existIndex(currentCommand.objectName[0]))
                API.dropIndex(currentCommand.objectName[0]);
                else
                {
                    outputHelp(INDEXERROR);
                    return false;
                }
                cout<<"The index has been droped"<<endl;
                break;
        }
            break;
        case DELETE:
            if(API.existTable(currentCommand.objectName[0])==false)//如果这张表不存在
            {
                outputHelp(TABLEERROR);
                return false;
            }
            else
            {
                int total;
                if(condition.size() > 0)//带where从句

                {
                    total = API.deleteValue(currentCommand.objectName[0], condition);
                }
                else
                    total = API.deleteValue(currentCommand.objectName[0]);
                cout<<total<<" records have been deleted."<<endl;
            }
            break;
        case SELECT:
        {if(API.existTable(currentCommand.objectName[0])==false)//如果这张表不存在
            {
                outputHelp(TABLEERROR);
                return false;
            }
            vector<Row> result;
             if(condition.size()>0)
             {
                 result = API.select(currentCommand.objectName[0], condition);
             }
             else
                 result = API.select(currentCommand.objectName[0]);
            API.showResults(currentCommand.objectName[0], result);
            break;}
        case INSERT:
            if(API.existTable(currentCommand.objectName[0])==false)//如果这张表不存在
            {
                outputHelp(TABLEERROR);
                return false;
            }
            if(API.checkInsertNume(currentCommand.objectName[0], insert) == false)//如果插入数目不对
            {
                outputHelp(INSERTNUMBERERROR);
                return false;
            }
            if(API.checkInsertType(currentCommand.objectName[0], insert) == false)//如果数据类型不对
            {
                outputHelp(INSERTTYPEERROR);
                return false;
            }
            if(API.uniqueValue(currentCommand.objectName[0], insert) == false)
            {
                outputHelp(UNIQUEVALUE);
                return false;
            }
            API.insertValue(currentCommand.objectName[0], insert);
            cout<<"The data have been inserted"<<endl;
            break;
        case EXECFILE:
        {
            vector<string> commands = readFile(currentCommand.objectName[0]);
            for(size_t i = 0; i < commands.size(); ++i)
            {
                originalInput = commands[i];
                //cout<<"test originalInput after reading file"<<endl;
                //cout<<originalInput;
                converseCase();
                currentCommand.operation = -1;
                currentCommand.objectType = -1;
                currentCommand.objectName.clear();
                insert.clear();
                if(judgeCommandType(originalInput) == true)
                {
                    if(parseCommand(originalInput) == true)
                        executeCommand();
                }
            }
            break;
        }
        case QUIT:
            quitflag = true;
            break;
        default:
            break;
    }
    return true;
}

void Interpreter:: outputHelp(int errorType)
{
    switch(errorType)
    {
        case EMPTY:
            cout << "ERROR: Empty query! Please enter your command!" <<endl;
            break;
        case UNKNOWN:
            cout << "ERROR: UNKNOW query! Please check your input!" << endl;
            break;
        case SELERR:
            cout << "ERROR: Incorrect usage of \"select\" query! Please check your input!" << endl;
            break;
        case CRETABERR:
            cout << "ERROR: Incorrect usage of \"create table\" ! Please check your input!" << endl;
            break;
        case CREINDERR:
            cout << "ERROR: Incorrect usage of \"create index\" ! Please check your input!" << endl;
            break;
        case DELETEERR:
            cout << "ERROR: Incorrect usage of \"delete from\" query! Please check your input!" << endl;
            break;
        case DRPTABERR:
            cout << "ERROR: Incorrect usage of \"drop table\" query! Please check your input!" << endl;
            break;
        case DRPINDERR:
            cout << "ERROR: Incorrect usage of \"drop index\" query! Please check your input!" << endl;
            break;
        case VOIDPRI:
            cout << "ERROR: Error: Invalid primary key! Please check your input!" << endl;
            break;
        case VOIDUNI:
            cout << "ERROR: Error: Invalid unique key! Please check your input!" << endl;
            break;
        case CHARBOUD:
            cout << "ERROR: Error: only 1~255 charactors is allowed! Please check your input!" << endl;
            break;
        case NOPRIKEY:
            cout << "ERROR: No primary key is defined! Please check your input!" << endl;
            break;
        case TABLEERROR:
            cout << "ERROR: Table dose not exist,please check the database." << endl;
            break;
        case INDEXERROR:
            cout << "ERROR: Index dose not exist,please check the database." << endl;
            break;
        case COLUMNERROR:
            cout << "ERROR: Index already exists on this column" << endl;
            break;
        case INSERTNUMBERERROR:
            cout << "ERROR: The column number is different from the num of the attributes." << endl;
            break;
        case INSERTERR:
            cout << "ERROR: Incorrect usage of \"insert\" command" << endl;
            break;
        case EXEFILERR:
            cout << "ERROR: No such file or directory!" << endl;
            break;
        case WHEREERROR:
            cout << "ERROR: Incorrect usage of \"where\" condition" << endl;
            break;
        case INVALIDNAME:
            cout << "ERROR: Invalid name, please check again." << endl;
            break;
        case TYPEERROR:
            cout << "ERROR: We only support char, float and int, please check the type of your data."<<endl;
            break;
        case SAMETABLE:
            cout << "ERROR: This table already exists."<<endl;
            break;
        case SAMEINDEX:
            cout << "ERROR: This index already exists."<<endl;
            break;
        case INSERTTYPEERROR:
            cout << "ERROR: Worng data type."<<endl;
            break;
        case UNIQUEVALUE:
            cout << "ERROR: Same value for the unique attribute."<<endl;
            break;
        case NOATTRI:
            cout << "ERROR: This attribute does not exist." <<endl;
            break;
    }
}

void Interpreter:: test()
{
    cout<<"test input"<<endl;
    cout<<originalInput<<endl;
    cout<<"____________________________"<<endl;
    cout<<"test command "<<endl;
    cout<<"operation: "<<currentCommand.operation<<endl;
    cout<<"objname: ";
    for(size_t i = 0; i < currentCommand.objectName.size(); ++i)
    {
        cout<<currentCommand.objectName[i]<<" ";
    }
    cout<<endl;
    cout<<"___________________________"<<endl;
    cout<<"test table"<<endl;
    cout<<"table name: "<<currentTable.name<<endl;
    cout<<"pk: "<<primaryKeyPosition<<endl;
    cout<<"uk: "<<uniqueKeyPostion<<endl;
    cout<<"test attribute"<<endl;
    cout<<"attribute size: "<<currentTable.attributes.size()<<endl;
    for(size_t i = 0; i < currentTable.attributes.size(); ++i)
    {
        cout<<currentTable.attributes[i].name<<endl;
        cout<<currentTable.attributes[i].type<<endl;
        cout<<currentTable.attributes[i].length<<endl;
    }
    cout<<"___________________________"<<endl;
    cout<<"test insert"<<endl;
    for(size_t i = 0; i < insert.size(); ++i)
    {
        cout<<insert[i]<<endl;
    }
    cout<<endl;
}
