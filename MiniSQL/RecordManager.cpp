//
//  RecordManager.cpp
//  MiniSQL
//
//  Created by Myh on 11/13/14.
//  Copyright (c) 2014 Myh. All rights reserved.
//

#include "RecordManager.h"


using namespace std;
bool RecordManager::insertValues(Table &table, string s){
    static int curAttri = 0;
    static string row;
    if(curAttri == 0){
        row.clear();
    }
    if(curAttri != table.attriNum){
        row+=s;
        curAttri++;
    }
    if(curAttri == table.attriNum){
        curAttri = 0;
        cout<<row;
        buffermanager.writeData("filename", 0, row.c_str(), sizeof(row), 1);
        return true;
    }
    return false;
}

