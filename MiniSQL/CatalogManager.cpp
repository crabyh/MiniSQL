//
//  CatalogManager.cpp
//  MiniSQL
//
//  Created by Myh on 11/1/14.
//  Copyright (c) 2014 Myh. All rights reserved.
//

#include "CatalogManager.h"
#include "PublicClass.h"

//初始化catalogmanager并从file中读取数据（指针会无效？）
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
    fstream  indexFile("index.g", ios::in);
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
        throw "primarykey error!";
    Attribute attribute(attriName, type, length, isPrimaryKey, isUnique);
    table.attributes.push_back(attribute);
    return true;
}

//更新Table的其他信息
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

//查询表
Table CatalogManager::findTable(string tableName){
    for(int i=0;i<Vtable.size();i++)
        if(Vtable[i].name==tableName)
            return Vtable[i];
    Table table("No such table",-1,0);       //没有查询到表
    return table;
}

//查询Index







