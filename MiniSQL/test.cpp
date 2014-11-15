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
    CatalogManager catalogmanager;
    Table &table = catalogmanager.createTable("tablename","pk");
    catalogmanager.insertAttri(table, "pk", CHAR, 22, true, true);
    RecordManager recordmanager(buffermanager,catalogmanager);
    table = catalogmanager.Vtable[0];
    recordmanager.insertValues(catalogmanager.Vtable[0], "12345");
    recordmanager.insertValues(catalogmanager.Vtable[0], "hello");
    recordmanager.insertValues(catalogmanager.Vtable[0], "321");
    vector <Row> result = recordmanager.select(table, "pk", "321", EQUAL);
    int deleteNum = recordmanager.deleteRow(table, "pk", "hello", EQUAL);
    result = recordmanager.select(table, "pk", "hello", EQUAL);
    return 0;
}
