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



