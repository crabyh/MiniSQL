//
//  PublicClass.h
//  MiniSQL
//
//  Created by Myh on 11/1/14.
//  Copyright (c) 2014 Myh. All rights reserved.
//

#ifndef MiniSQL_PublicClass_h
#define MiniSQL_PublicClass_h

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdio.h>
#define MAXBUFFERNUM 20 //BUFFER中的块数
#define BLOCKSIZE 4096//在MAIN.H中定义？
#define FILEPTR long
using namespace std;
enum type {INT, CHAR, STRING};
enum CONDITION_TYPE {EQUAL, NOT_EQUAL, GREATER, GREATER_EQUAL, SMALLER, SMALLER_EQUAL, EXIST};



class Conditions
{
public:
    string attribute;
    CONDITION_TYPE condition_type;
    string attributeValue;
};

class Data;

class Attribute
{
public:
    friend class CatalogManager;
    friend class Table;
    string name = "";
    string indexName = "";      //index名
    int type = INT;
    int length = 0;
    bool isPrimaryKey = false;
    bool isUnique = false;

    Attribute(){}
    Attribute(string name, int type, int length, bool isPrimaryKey = false, bool isUnique = false): name(name), type(type), length(length), isPrimaryKey(isPrimaryKey), isUnique(isUnique){}
};

class Row
{
public:
    string value;
    FILEPTR ptr;
};

class Table
{
public:
    string name;     //表名
    int blockNum = 0;       //在name.table中占用的block数
    int recordNum = 0;      //记录条数
    int attriNum = 0;       //属性数
    int eachRecordLength = 0;       //每条记录的长度
    string primaryKey;     //主键名字
    int freeNum = 0;       //有几条被删除的记录
    vector<Attribute> attributes;       //指向元数据链表的指针
    vector<Attribute>::iterator AttriIt;     //Attribute的iterator
    FILEPTR firstRow;     //指向数据链表的指针
    long dataBlockInFile = -1;       //data开头在file中的块的位置（每张表的数据一定是从一块的开头开始）
    FILEPTR freeList;        //指向等待删除链表的指针（这东西到底干吗用）

    Table(){}
    //带参数的初始化函数
    Table(string name, string primaryKey)
    :name(name), primaryKey(primaryKey){}
};

inline string format(int num)
{
    string result;
    stringstream ss;
    ss << num;
    ss >> result;
    ss.clear();
    result.insert(0, 11 - result.length(), '0');
    return result;
}

inline string format(float num)
{
    char c[100];
    string s;
    sprintf(c, "%40.7f", num);
    s = c;
    return  s;
}


#endif
