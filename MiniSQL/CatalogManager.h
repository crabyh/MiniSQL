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

class CatalogManager
{
    vector<Table> Vtable;
    int tableNum;
    
public:
    CatalogManager();
    ~CatalogManager();
};

#endif /* defined(__MiniSQL__CatalogManager__) */
