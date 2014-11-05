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
    fstream  tableFile("table", ios::in|ios::binary);
    tableFile.read((char*)this, sizeof(CatalogManager));        //读取catalogmanager
    Vtable= *new vector <Table>;
    for ( int i = 0; i < tableNum ; i++)
    {
        Table* table = new Table;
        tableFile.read((char*)table,sizeof(Table));        //读取table
        table->attributes= *new vector <Attribute>;      //清空指针
        table->data.clear();
        table->emptyList.clear();
        for (int j = 0; j < table->attriNum; j++)
        {
            Attribute* attribute = new Attribute;
            tableFile.read((char*)attribute, sizeof(Attribute));       //读取attribute
            table->attributes.push_back(*attribute);
        }
        Vtable.push_back(*table);
    }
    tableFile.close();
}

//将table的信息从容器中写入到文
CatalogManager:: ~CatalogManager()
{   //需要先清理freelist?!
    fstream  tableFile( "table", ios::out|ios::binary|ios::app);
    tableFile.write((char*)this, sizeof(CatalogManager));       //写入catalogmanager
    for (int i = 0; i < tableNum; i++)
    {
        tableFile.write((char*)&Vtable[i], sizeof(Table));      //写入table
        for(int j=0; j<Vtable[i].attriNum; j++)
        {
            tableFile.write((char*)&(Vtable[i].attributes[j]), sizeof(Attribute));//写入attribute
        }
    }
    tableFile.close();
}

//下面3个函数一起初始化Table
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
                if(Vtable[i].attributes[j].name == attriName && Vtable[i].attributes[j].indexName == "")
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

//删除table，同时删除table上的index和指向的data（我需要一个recordmanager的函数！）
bool CatalogManager::dropTable(string tableName)
{
    for(int i=0;i<Vtable.size();i++)
        if(Vtable[i].name==tableName)
        {
            Vtable.erase(Vtable.begin()+i);
            tableNum--;
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

int main(int argc, const char * argv[]) {
    CatalogManager catalogmanager;
    Table &table = catalogmanager.createTable("tablename","pk");
    catalogmanager.insertAttri(table, "at", INT, 4);
    catalogmanager.insertAttri(table, "pk", INT, 4, true, true);
    
    return 0;
}












