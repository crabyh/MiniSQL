//
//  CatalogManager.h
//  MiniSQL
//
//  Created by Myh on 11/1/14.
//  Copyright (c) 2014 Myh. All rights reserved.
//

#ifndef __MiniSQL__CatalogManager__
#define __MiniSQL__CatalogManager__

#include "PublicClass.h"

class CatalogManager
{
public:
    vector<Table> Vtable;
    vector<Table>::iterator VtableIt;
    int tableNum;
    CatalogManager();
    ~CatalogManager();
    Table &createTable(string name, string primarykey);
    bool insertAttri(Table& table, string attriName, int type, int length, bool isPrimaryKey=false, bool isUnique=false);
    bool initiaTable(Table& table);
    bool createIndex(string indexName, string tableName, string attriName);
    int findTable(string tableName);
    int findIndexTable(string indexName);
    int findIndexAttri(string indexName);
    bool dropTable(string tableName);
    bool dropIndex(string indexName);
    bool deleteAttri(Table &table, string attriName);
    int getAttriNum(Table &table, string attriName);
    
};

#endif /* defined(__MiniSQL__CatalogManager__) */
