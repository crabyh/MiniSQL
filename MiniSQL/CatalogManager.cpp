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
        tableFile >> table.eachRecordLength;
        tableFile >> table.primaryKey;
        tableFile >> table.freeNum;
        int j;
        string a = 0;
        for ( j = 0; j < table.recordNum; j++)
        {
            tableFile >> a;
            table.data->push_back(a);
        }
        table.freeNum=NULL;
        for (j = 0; j < table.attriNum; j++)
        {
            Attribute attribute;
            tableFile >> attribute.name;
            tableFile >> attribute.type;
            tableFile >> attribute.length;
            tableFile >> attribute.isPrimaryKey;
            tableFile >> attribute.isUnique;
            table.attributes->push_back(attribute);
        }
        Vtable.push_back(table);
    }
    tableFile.close();
    fstream  indexFile("d:/index.g", ios::in);
}

//将table和index信息从容器中写入到文
CatalogManager:: ~CatalogManager()
{
    fstream  tableFile( "d:/table.g", ios::out);
    tableFile << tableNum << endl;
    int i, j;
    for (i = 0; i < tableNum; i++)
    {
        tableFile << Vtable[i].name << endl;
        tableFile << Vtable[i].blockNum << " " << Vtable[i].recordNum << " " << Vtable[i].attriNum << endl;
        tableFile << Vtable[i].totalLength << " ";
        tableFile << Vtable[i].primaryKey << " ";
        tableFile << Vtable[i].emptyNum << endl;
        for (j = 0; j < Vtable[i].recordNum; j++)
        {
            tableFile << Vtable[i].recordList[j] << " ";
        }
        tableFile << endl;
        for(j = 0; j < Vtable[i].emptyNum; j++)
        {
            tableFile << Vtable[i].emptyList[j] << " ";
        }
        tableFile << endl;
        for (j = 0; j < Vtable[i].attriNum; j++)
        {
            tableFile << Vtable[i].attributes[j].name << " ";
            tableFile << Vtable[i].attributes[j].type << " ";
            tableFile << Vtable[i].attributes[j].length << " ";
            tableFile << Vtable[i].attributes[j].isPrimaryKey << " ";
            tableFile << Vtable[i].attributes[j].isUnique << endl;
        }
    }
    tableFile.close();
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
