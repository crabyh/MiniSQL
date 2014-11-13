//
//  RecordManager.h
//  MiniSQL
//
//  Created by Myh on 11/13/14.
//  Copyright (c) 2014 Myh. All rights reserved.
//

#ifndef __MiniSQL__RecordManager__
#define __MiniSQL__RecordManager__

#include <stdio.h>
#include <iostream>
#include <string>
#include "PublicClass.h"
#include "BufferManager.h"

class RecordManager{
    BufferManager buffermanager;
public:
    RecordManager(BufferManager &buffermanager):buffermanager(buffermanager){}
    bool insertValues(Table &table, string s);
    
};

#endif /* defined(__MiniSQL__RecordManager__) */
