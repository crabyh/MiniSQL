//
//  test.cpp
//  MiniSQL
//
//  Created by Myh on 11/13/14.
//  Copyright (c) 2014 Myh. All rights reserved.
//

#include "BufferManager.h"
#include "RecordManager.h"
#include "CatalogManager.h"
int main(){
    BufferManager buffermanager;
    CatalogManager catalogmanager(buffermanager);
    Table &table = catalogmanager.createTable("tablename","pk");
    catalogmanager.insertAttri(table, "pk", CHAR, 22, true, true);
    RecordManager recordmanager(buffermanager,catalogmanager);
    table = catalogmanager.Vtable[0];
    recordmanager.insertValues(catalogmanager.Vtable[0], "0");
    recordmanager.insertValues(catalogmanager.Vtable[0], "1");
    recordmanager.insertValues(catalogmanager.Vtable[0], "2");
    int deleteNum = recordmanager.deleteRow(table, "pk", "0", ALL);
    vector <Row> result = recordmanager.select(table, "pk", 1, ALL);
    recordmanager.insertValues(catalogmanager.Vtable[0], "later");
    result = recordmanager.select(table, "pk", "later", ALL);
    return 0;
}
