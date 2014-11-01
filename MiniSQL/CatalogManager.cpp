//
//  CatalogManager.cpp
//  MiniSQL
//
//  Created by Myh on 11/1/14.
//  Copyright (c) 2014 Myh. All rights reserved.
//

#include "CatalogManager.h"
#include "PublicClass.h"

CatalogManager::CatalogManager()
{
    tableNum = 0;
    IndexNum = 0;
    fstream  tableFile("d:/table.g", ios::in);
    tableFile >> tableNum;
    for ( int i = 0; i < tableNum ; i++)
    {
        Table table;
        tableFile >> table.name;
        tableFile >> table.blockNum;
        tableFile >> table.recordNum;
        tableFile >> table.attriNum;
        tableFile >> table.totalLength;
        tableFile >> table.primaryKey;
        tableFile >> table.emptyNum;
        int j;
        int a = 0;
        for ( j = 0; j < table.recordNum; j++)
        {
            tableFile >> a;
            table.recordList.push_back(a);
        }
        for (j = 0; j < table.emptyNum; j++)
        {
            tableFile >> a;
            table.emptyList.push_back(a);
        }
        for (j = 0; j < table.attriNum; j++)
        {
            Attribute attribute;
            tableFile >> attribute.name;
            tableFile >> attribute.type;
            tableFile >> attribute.length;
            tableFile >> attribute.isPrimaryKey;
            tableFile >> attribute.isUnique;
            table.attributes.push_back(attribute);
        }
        ttable.push_back(table);
    }
    tableFile.close();
    fstream  indexin("d:/index.g", ios::in);
    indexin >> indnum;
    for (int i = 0; i < indnum; i++)
    {
        Index index;
        indexin >> index.indexName;
        indexin >> index.tableName;
        indexin >> index.attriNum;
        indexin >> index.blockNum;
        indexin >> index.freeList;
        indexin >> index.attriLength;
        indexin >> index.rootAddress;
        indexin >> index.type;
        iindex.push_back(index);
    }
    indexin.close();
}

//将table和index信息从容器中写入到文
CatalogManager:: ~CatalogManager()
{
    fstream  tableout( "d:/table.g", ios::out);
    tableout << tableNum << endl;
    int i, j;
    for (i = 0; i < tableNum; i++)
    {
        tableout << ttable[i].name << endl;
        tableout << ttable[i].blockNum << " " << ttable[i].recordNum << " " << ttable[i].attriNum << endl;
        tableout << ttable[i].totalLength << " ";
        tableout << ttable[i].primaryKey << " ";
        tableout << ttable[i].emptyNum << endl;
        for (j = 0; j < ttable[i].recordNum; j++)
        {
            tableout << ttable[i].recordList[j] << " ";
        }
        tableout << endl;
        for(j = 0; j < ttable[i].emptyNum; j++)
        {
            tableout << ttable[i].emptyList[j] << " ";
        }
        tableout << endl;
        for (j = 0; j < ttable[i].attriNum; j++)
        {
            tableout << ttable[i].attributes[j].name << " ";
            tableout << ttable[i].attributes[j].type << " ";
            tableout << ttable[i].attributes[j].length << " ";
            tableout << ttable[i].attributes[j].isPrimaryKey << " ";
            tableout << ttable[i].attributes[j].isUnique << endl;
        }
    }
    tableout.close();
    fstream  indexout("d:/index.g", ios::out);
    indexout << indnum << endl;
    for (i = 0; i < indnum; i++)
    {
        indexout << iindex[i].indexName << " ";
        indexout << iindex[i].tableName << " ";
        indexout << iindex[i].attriNum << " ";
        indexout << iindex[i].blockNum << " ";
        indexout << iindex[i].freeList << " ";
        indexout << iindex[i].attriLength << " ";
        indexout << iindex[i].rootAddress << " ";
        indexout << iindex[i].type << endl;
    }
    
    indexout.close();
}


int main(int argc, const char * argv[]){
    
}
