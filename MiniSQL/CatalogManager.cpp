    //
//  CatalogManager.cpp
//  MiniSQL
//
//  Created by Myh on 11/1/14.
//  Copyright (c) 2014 Myh. All rights reserved.
//

#include "BufferManager.h"
#include "CatalogManager.h"
#include "PublicClass.h"

//初始化catalogmanager并从file中读取元数据
CatalogManager::CatalogManager(BufferManager &buffermanager):buffermanager(buffermanager)
{
    fstream  tableFile("Catalog", ios::in|ios::binary);
    tableFile >> tableNum;      //读取catalogmanager
    for ( int i = 0; i < tableNum ; i++)
    {
        Table table;
        tableFile >> table.name >> table.blockNum >> table.recordNum
        >> table.attriNum >> table.eachRecordLength >> table.primaryKey >> table.freeNum
        >> table.firstRow >> table.freeList >> table.fileEnd >> table.curPtr;      //读取table
        Vtable.push_back(table);
        for (int j = 0; j < Vtable[i].attriNum; j++)
        {
            Attribute attribute;
            Vtable[i].attributes.push_back(attribute);
            tableFile >> Vtable[i].attributes[j].name
            >> Vtable[i].attributes[j].indexName
            >> Vtable[i].attributes[j].type
            >> Vtable[i].attributes[j].length
            >> Vtable[i].attributes[j].isPrimaryKey
            >> Vtable[i].attributes[j].isUnique;      //读取attribute
        }
    }
    tableFile.close();
}

//将table的信息从容器中写入到文
CatalogManager:: ~CatalogManager()
{   //需要先清理freelist?!
    fstream  tableFile( "Catalog", ios::out|ios::binary|ios::app);
    tableFile << tableNum << " ";       //写入catalogmanager
    for (int i = 0; i < tableNum; i++)
    {//写入table
        tableFile << Vtable[i].name  << " "
        <<  Vtable[i].blockNum << " "
        <<  Vtable[i].recordNum << " "
        <<  Vtable[i].attriNum << " "
        <<  Vtable[i].eachRecordLength << " "
        <<  Vtable[i].primaryKey << " "
        <<  Vtable[i].freeNum << " "
        <<  Vtable[i].firstRow << " "
        <<  Vtable[i].freeList << " "
        <<  Vtable[i].fileEnd << " "
        <<  Vtable[i].curPtr << " ";
        for(int j=0; j<Vtable[i].attriNum; j++)
        {
            tableFile << Vtable[i].attributes[j].name  << " "
            << Vtable[i].attributes[j].indexName  << " "
            << Vtable[i].attributes[j].type  << " "
            << Vtable[i].attributes[j].length  << " "
            << Vtable[i].attributes[j].isPrimaryKey  << " "
            << Vtable[i].attributes[j].isUnique  << " ";
        }
    }
    tableFile.close();
}

//下面2个函数一起初始化Table
//构造一个Table
Table& CatalogManager::createTable(string name, string primarykey)
{
    if(findTable(name) != -1)
        throw "table already exist";
    Table* table= new Table(name, primarykey);
    Vtable.push_back(*table);
    this->tableNum++;
    return Vtable.back();
}
//插入attribute
bool CatalogManager::insertAttri(Table& table, string attriName, int type, int length, bool isPrimaryKey, bool isUnique)
{
    if(type == INT) length = 11;
    else if(type == FLOAT) length = 40;
    //else if(type == CHAR) length += 1;
    if(isPrimaryKey==1 && attriName!=table.primaryKey)
        return false;       //primarykey与table中的不对应
    if(getAttriNum(table, attriName)!=-1)
        return false;       //属性已经存在
    Attribute attribute(attriName, type, length, isPrimaryKey, isUnique);
    table.attributes.push_back(attribute);
    initiaTable(table);
    return true;
}
//更新Table的其他信息(在插入或者删除attributes后自动调用)
bool CatalogManager::initiaTable(Table& table)
{
    table.eachRecordLength = 0;
    table.attriNum = 0;
    for(int i=0;i<table.attributes.size();i++)      //更新每条记录长度
    {
        table.eachRecordLength += table.attributes[i].length;
        table.attriNum += 1;
    }
    for(int i=0;i<table.attriNum;i++)       //检查至少有一个PK
        if (table.attributes[i].isPrimaryKey == 1)
            return true;
    return false;
}

//创建Index
bool CatalogManager::createIndex(string indexName, string tableName, string attriName){
    for(int i=0;i<Vtable.size();i++)
        if(Vtable[i].name == tableName)
            for(int j=0;j<Vtable[i].attributes.size();j++)
                if(Vtable[i].attributes[j].name == attriName && Vtable[i].attributes[j].indexName == "NULL")
                {
                    Vtable[i].attributes[j].indexName = indexName;
                    return true;    //成功返回true
                }
    return false;       //失败返回false
                    
}

//查询表
int CatalogManager::findTable(string tableName){
    for(int i=0;i<Vtable.size();i++)
        if(Vtable[i].name == tableName)
            return i;
    return -1;       //没有查询到表
}

//查询Index
int CatalogManager::findIndexTable(string indexName)
{
    for(int i=0;i<Vtable.size();i++)
        for(int j=0;j<Vtable[i].attributes.size();j++)
            if(Vtable[i].attributes[j].indexName==indexName)
                return i;
    return -1;       //没有查询到Index
}
int CatalogManager::findIndexAttri(string indexName)
{
    for(int i=0;i<Vtable.size();i++)
        for(int j=0;j<Vtable[i].attributes.size();j++)
            if(Vtable[i].attributes[j].indexName==indexName)
                return j;
    return -1;      //没有查询到Index
}

//删除table，同时删除table上的index和指向的data
bool CatalogManager::dropTable(string tableName)
{
    for(int i=0;i<Vtable.size();i++)
        if(Vtable[i].name==tableName)
        {
            Vtable.erase(Vtable.begin()+i);
            string filename = tableName+".table";
            remove(filename.c_str());       //删除文件
            buffermanager.dropTableInBuffer(tableName);     //删除buffer中的数据
            tableNum--;     //更新tableNum
            return true;        //成功返回true
        }
    return false;       //失败返回false
}

//删除index,供index调用
bool CatalogManager::dropIndex(string indexName)
{
    for(int i=0;i<Vtable.size();i++)
        for(int j=0;j<Vtable[i].attributes.size();j++)
            if(Vtable[i].attributes[j].indexName==indexName)
            {
                Vtable[i].attributes[j].indexName = "NULL";
                return true;        //成功返回true
            }
    return false;       //失败返回false
}

//从表中删除一个属性
bool CatalogManager::deleteAttri(Table &table, string attriName){
    for(int i=0;i<table.attributes.size();++i)
        if(table.attributes[i].name == attriName)
        {
            table.attributes.erase(table.attributes.begin()+i);
            initiaTable(table);     //更新table数据
            return true;        //成功返回true
        }
    return false;       //失败返回false
}

//检查attriNum在第几个属性
int CatalogManager::getAttriNum(Table &table, string attriName){
    for(int i=0;i<table.attributes.size();++i)
        if(table.attributes[i].name == attriName)
        {
            initiaTable(table);     //更新table数据
            return i;        //成功返回序号
        }
    return -1;       //失败返回-1
}

int CatalogManager::lengthBeforeAttri(Table &table, string attriName){
    int totalLength = 0;
    for(int i=0;i<table.attributes.size();++i)
        if(table.attributes[i].name == attriName)
        {
            initiaTable(table);     //更新table数据
            return totalLength;        //成功返回序号
        }
        else
        {
            totalLength+=table.attributes[i].length;
        }
    return -1;       //失败返回-1
}
//int main(int argc, const char * argv[]) {
//    CatalogManager catalogmanager;
//    Table &table = catalogmanager.createTable("tablename","pk");
//    catalogmanager.insertAttri(table, "at", INT, 4);
//    catalogmanager.insertAttri(table, "pk", INT, 4, true, true);
//    catalogmanager.insertAttri(table, "pk2", STRING, 12);
//    return 0;
//}












