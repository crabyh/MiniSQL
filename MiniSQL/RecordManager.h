//
//  RecordManager.h
//  MiniSQL
//
//  Created by Myh on 11/13/14.
//  Copyright (c) 2014 Myh. All rights reserved.
//

#ifndef __MiniSQL__RecordManager__
#define __MiniSQL__RecordManager__

#include "PublicClass.h"
#include "BufferManager.h"
#include "CatalogManager.h"

class RecordManager{
    BufferManager &buffermanager;
    CatalogManager &catalogmanager;
public:
    RecordManager(BufferManager &buffermanager, CatalogManager &catalogmanager):buffermanager(buffermanager), catalogmanager(catalogmanager){}
    FILEPTR insertValues(Table &table, string s);
    FILEPTR insertValues(Table &table, int  num);
    FILEPTR insertValues(Table &table, double f);
    Row nextRecord(Table &table);
    Row findRecord(Table &table, FILEPTR addr);
    
    bool compare(string s, string condition, int CONDITION_TYPE);
    bool compare(int s, int condition, int CONDITION_TYPE);
    bool compare(double s, double condition, int CONDITION_TYPE);
    
    vector<Row> select(Table &table, string attriName, string condition,int CONDITION_TYPE);
    vector<Row> select(Table &table, string attriName, int condition,int CONDITION_TYPE);
    vector<Row> select(Table &table, string attriName, double condition,int CONDITION_TYPE);
    
    vector<Row> select(Table &table, vector<Row> &rows, string attriName, string condition,int CONDITION_TYPE);
    vector<Row> select(Table &table, vector<Row> &rows, string attriName, int condition,int CONDITION_TYPE);
    vector<Row> select(Table &table, vector<Row> &rows, string attriName, double condition,int CONDITION_TYPE);
    
    string attriInRecord(Table &table, Row &row, string attriName);
    Row findFreeListEnd(Table &table);
    
    int deleteRow(Table &table, string attriName, string condition,int CONDITION_TYPE);
    int deleteRow(Table &table, string attriName, int condition,int CONDITION_TYPE);
    int deleteRow(Table &table, string attriName, double condition,int CONDITION_TYPE);
};

#endif /* defined(__MiniSQL__RecordManager__) */
