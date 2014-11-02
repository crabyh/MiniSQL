//
//  CatalogManager.cpp
//  MiniSQL
//
//  Created by Myh on 11/1/14.
//  Copyright (c) 2014 Myh. All rights reserved.
//

#include "CatalogManager.h"
#include "PublicClass.h"

//初始化catalogmanager并从file中读取元数据
CatalogManager::CatalogManager()
{
    fstream  tableFile("table.g", ios::in|ios::binary);
    tableFile.read((char*)this, sizeof(CatalogManager));        //读取catalogmanager
    for ( int i = 0; i < tableNum ; i++)
    {
        Table table;
        tableFile.read((char*)&table,sizeof(Table));        //读取table
        table.attributes.clear();       //清空指针
        table.data.clear();
        table.emptyList.clear();
        for (int j = 0; j < table.attriNum; j++)
        {
            Attribute attribute;
            tableFile.read((char*)&attribute, sizeof(attribute));       //读取attribute
            table.attributes.push_back(attribute);
        }
        Vtable.push_back(table);
    }
    tableFile.close();
}

//将table的信息从容器中写入到文
CatalogManager:: ~CatalogManager()
{   //需要先清理freelist?!
    fstream  tableFile( "table.g", ios::out|ios::binary|ios::app);
    tableFile.write((char*)this, sizeof(CatalogManager));       //写入catalogmanager
    for (int i = 0; i < tableNum; i++)
    {
        tableFile.write((char*)&Vtable[i], sizeof(Table));      //写入table
        for(int j=0; j<Vtable[i].attriNum; j++)
        {
            tableFile.write((char*)&Vtable[i].attributes, sizeof(Attribute));//写入attribute
        }
    }
    tableFile.close();
}

//下面3个函数一起初始化Table
//构造一个Table
Table CatalogManager::createTable(string name, int attriNum, string primarykey)
{
    return Table(name, attriNum, primarykey);
}
//插入attribute
bool CatalogManager::insertAttri(Table& table, string attriName, int type, int length, bool isPrimaryKey, bool isUnique)
{
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
    for(int i=0;i<table.attributes.size();i++)      //更新每条记录长度
    {
        table.eachRecordLength += table.attributes[i].length;
        table.attriNum+=1;
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
                if(Vtable[i].attributes[j].name == attriName && Vtable[i].attributes[j].indexName == "")
                {
                    Vtable[i].attributes[j].indexName = indexName;
                    return true;    //成功返回true
                }
    return false;       //失败返回false
                    
}

//查询表
Table CatalogManager::findTable(string tableName){
    for(int i=0;i<Vtable.size();i++)
        if(Vtable[i].name == tableName)
            return Vtable[i];
    Table table("notExit",-1,0);       //没有查询到表
    return table;
}

//查询Index
Table CatalogManager::findIndexTable(string indexName)
{
    for(int i=0;i<Vtable.size();i++)
        for(int j=0;j<Vtable[i].attributes.size();j++)
            if(Vtable[i].attributes[j].indexName==indexName)
                return Vtable[i];
    Table table("notExit",-1,0);       //没有查询到Index
    return table;
}
Attribute CatalogManager::findIndexAttri(string indexName)
{
    for(int i=0;i<Vtable.size();i++)
        for(int j=0;j<Vtable[i].attributes.size();j++)
            if(Vtable[i].attributes[j].indexName==indexName)
                return Vtable[i].attributes[j];
    Attribute attribute("notExit",0,0);       //没有查询到Index
    return attribute;
}

//删除table，同时删除table上的index
bool CatalogManager::dropTable(string tableName)
{
    for(int i=0;i<Vtable.size();i++)
        if(Vtable[i].name==tableName)
        {
            Vtable.erase(Vtable.begin()+i);
            return true;        //成功返回true
        }
    return false;       //失败返回false
}

//删除index
bool CatalogManager::dropIndex(string indexName)
{
    for(int i=0;i<Vtable.size();i++)
        for(int j=0;j<Vtable[i].attributes.size();j++)
            if(Vtable[i].attributes[j].indexName==indexName)
            {
                Vtable[i].attributes[j].indexName = "";
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
            table.attributes.erase(table.attributes.begin()+i);
            initiaTable(table);     //更新table数据
            return i;        //成功返回序号
        }
    return -1;       //失败返回-1
}














