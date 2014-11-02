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
#include <fstream>
#include <iostream>

class CatalogManager
{
    vector<Table> Vtable;
    int tableNum;
    
public:
    CatalogManager();
    ~CatalogManager();
    Table createTable(string name, int attriNum, string primarykey);
    bool insertAttri(Table& table, string attriName, int type, int length, bool isPrimaryKey=false, bool isUnique=false);
    bool initiaTable(Table& table);
    Table findTable(string tableName);
    
};

#endif /* defined(__MiniSQL__CatalogManager__) */
