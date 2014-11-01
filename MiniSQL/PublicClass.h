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
using namespace std;

class Data;

class Attribute
{
    friend class Table;
    string name;
    int type;
    int length;
    bool isPrimaryKey;
    bool isUnique;
    Attribute()
    {
        isPrimaryKey = false;
        isUnique = false;
        name = "";
        type = 1;
        length = 0;
    }
    Attribute(string name, int type, int length, bool isPrimaryKey = false, bool isUnique = false): name(name), type(type), length(length), isPrimaryKey(isPrimaryKey), isUnique(isUnique){}
};

class Table
{
    string name;     //表名
    int blockNum = 0;       //在name.table中占用的block数
    int recordNum = 0;      //记录条数
    int attriNum;       //属性数
    int eachRecordLength = 0;       //每条记录的长度
    int primaryKey;     //主键是第几个
    int freeNum = 0;       //有几条被删除的记录
    vector<Attribute>* attributes;       //指向元数据链表的指针
    vector<string>* data = NULL;     //指向数据链表的指针
    vector<string>* emptyList = NULL;        //指向等待删除链表的指针
public:
    //带参数的初始化函数（未完成）
    Table(string name,int attriNum, vector<Attribute>* attributes, int primaryKey)
    :name(name),attriNum(attriNum), attributes(attributes),primaryKey(primaryKey)
    {
        if(primaryKey>=attriNum)
            throw ("primaryKeyError");
        for(int i=0;i<attributes->size();i++)
        {
            eachRecordLength += attributes->at(i).length;
            attriNum+=1;
        }
    }
};

class Index
{
    string indexName;       //索引名
    Table* tableAddr;       //所在表的地址
    int OnAttriNum;     //在第几个属性上建立的索引
public:
    //带参数的初始化函数
    Index(string indexName, Table* tableAddr, int OnAttriNum):indexName(indexName),tableAddr(tableAddr),OnAttriNum(OnAttriNum){}
};

class Data
{
    friend class Table;
    vector<vector<string>> data;
};

#endif