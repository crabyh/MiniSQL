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
//    Table &table = catalogmanager.createTable("tablename","pk");
//    catalogmanager.insertAttri(table, "pk", INT, 4, true, true);
    RecordManager recordmanager(buffermanager);
    recordmanager.insertValues(catalogmanager.Vtable[0], "helloworld2");
    return 0;
}
