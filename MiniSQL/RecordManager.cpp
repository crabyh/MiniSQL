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
    static Row& row = *new Row;
    static Row& old_row = *new Row;
    if(table.attributes[curAttri].type == CHAR){
        s.resize(table.attributes[curAttri].length, ' ');
    }
    if(table.fileEnd%BLOCKSIZE + table.eachRecordLength > BLOCKSIZE)
        table.fileEnd = BLOCKSIZE*(table.fileEnd/BLOCKSIZE+1);
    if(table.freeList != -1){
        
    }
    else{
        if(curAttri != table.attriNum){
            row.value+=s;
            curAttri++;
        }
        if(curAttri == table.attriNum){
            curAttri = 0;
            //检测是否为第一条记录
            if(table.firstRow == -1){
                table.eachRecordLength = sizeof(row);
                table.firstRow = 0;
            }
            
            //更新上一条记录的指针
            if(old_row.value != ""){
                old_row.ptr = table.fileEnd;
                cout<<sizeof(old_row)<<" "<<old_row.ptr<<endl;
                buffermanager.writeData("filename", table.fileEnd - sizeof(row.ptr), (char*)&old_row.ptr, sizeof(row.ptr), 1);
            }
            
            cout<<sizeof(row)<<" "<<row.ptr<<endl;
            buffermanager.writeData("filename", table.fileEnd, (char*)&row.value, sizeof(row.value), 1);
            table.fileEnd += sizeof(row.value);
            buffermanager.writeData("filename", table.fileEnd, (char*)&row.ptr, sizeof(row.ptr), 1);
            table.fileEnd += sizeof(row.ptr);
            old_row = row;
            row.value.clear();
            table.recordNum++;
            return true;
        }
    }
    return false;
}

bool RecordManager::insertValues(Table &table, int num){
    string data = format(num);
    return insertValues(table, data);
}

bool RecordManager::insertValues(Table &table, double f){
    string data = format(f);
    return insertValues(table, data);
}


//查看下一条记录
Row RecordManager::nextRecord(Table &table){
    Row& row = *new Row;
    if(table.curPtr == -1) table.curPtr = table.firstRow;
    if(table.curPtr%BLOCKSIZE + table.eachRecordLength > BLOCKSIZE)
        table.curPtr = BLOCKSIZE*(table.curPtr/BLOCKSIZE+1);
    memcpy(&row.value, buffermanager.readData("filename", table.curPtr), sizeof(row.value));
    table.curPtr += sizeof(row.value);
    memcpy(&row.ptr, buffermanager.readData("filename", table.curPtr), sizeof(row.ptr));
    table.curPtr = row.ptr;
    return row;
}

//从一条记录中提取一个属性对应的数据
string RecordManager::attriInRecord(Table &table, Row &row, string attriName){
    int x = catalogmanager.getAttriNum(table, attriName);
    int length = catalogmanager.lengthBeforeAttri(table, attriName);
    string s = row.value.substr(length,length+table.attributes[x].length);
    return s;
}

bool RecordManager::compare(string s, string condition, int CONDITION_TYPE){
    switch (CONDITION_TYPE){
        case EQUAL:
            return s == condition;
        case NOT_EQUAL:
            return s != condition;
        case GREATER:
            return s > condition;
        case GREATER_EQUAL:
            return s >= condition;
        case SMALLER:
            return s < condition;
        case SMALLER_EQUAL:
            return s <= condition;
        default:
            return false;
    }
}

bool RecordManager::compare(int s, int condition, int CONDITION_TYPE){
    switch (CONDITION_TYPE){
        case EQUAL:
            return s == condition;
        case NOT_EQUAL:
            return s != condition;
        case GREATER:
            return s > condition;
        case GREATER_EQUAL:
            return s >= condition;
        case SMALLER:
            return s < condition;
        case SMALLER_EQUAL:
            return s <= condition;
        default:
            return false;
    }
}

bool RecordManager::compare(double s, double condition, int CONDITION_TYPE){
    switch (CONDITION_TYPE){
        case EQUAL:
            return s == condition;
        case NOT_EQUAL:
            return s != condition;
        case GREATER:
            return s > condition;
        case GREATER_EQUAL:
            return s >= condition;
        case SMALLER:
            return s < condition;
        case SMALLER_EQUAL:
            return s <= condition;
        default:
            return false;
    }
}

vector<Row> RecordManager::select(Table &table, string attriName, string condition, int CONDITION_TYPE){
    vector<Row> result;
    Row row;
    string data;
    for(int i=0;i<table.recordNum;i++){
        row = nextRecord(table);
        data = attriInRecord(table, row, attriName);
        data = trim(data);
        if(compare(data,condition,CONDITION_TYPE) == true){
            result.push_back(row);
        }
    }
    return result;
}

vector<Row> RecordManager::select(Table &table, string attriName, int condition, int CONDITION_TYPE){
    vector<Row> result;
    Row row;
    string data;
    for(int i=0;i<table.recordNum;i++){
        row = nextRecord(table);
        data = attriInRecord(table, row, attriName);
        int num = toInt(data);
        if(compare(num,condition,CONDITION_TYPE) == true){
            result.push_back(row);
        }
    }
    return result;
}

vector<Row> RecordManager::select(Table &table, string attriName, double condition, int CONDITION_TYPE){
    vector<Row> result;
    Row row;
    string data;
    for(int i=0;i<table.recordNum;i++){
        row = nextRecord(table);
        data = attriInRecord(table, row, attriName);
        double f = toFloat(data);
        if(compare(f,condition,CONDITION_TYPE) == true){
            result.push_back(row);
        }
    }
    return result;
}

//从筛选结果中继续筛选 用于and连接操作
vector<Row> RecordManager::select(Table &table, vector<Row> &rows, string attriName, string condition, int CONDITION_TYPE){
    vector<Row> result;
    Row row;
    string data;
    for(int i=0;i<rows.size();i++){
        row = nextRecord(table);
        data = attriInRecord(table, row, attriName);
        data = trim(data);
        if(compare(data,condition,CONDITION_TYPE) == true){
            result.push_back(row);
        }
    }
    return result;
}

vector<Row> RecordManager::select(Table &table, vector<Row> &rows, string attriName, int condition, int CONDITION_TYPE){
    vector<Row> result;
    Row row;
    string data;
    for(int i=0;i<table.recordNum;i++){
        row = nextRecord(table);
        data = attriInRecord(table, row, attriName);
        int num = toInt(data);
        if(compare(num,condition,CONDITION_TYPE) == true){
            result.push_back(row);
        }
    }
    return result;
}

vector<Row> RecordManager::select(Table &table, vector<Row> &rows, string attriName, double condition, int CONDITION_TYPE){
    vector<Row> result;
    Row row;
    string data;
    for(int i=0;i<table.recordNum;i++){
        row = nextRecord(table);
        data = attriInRecord(table, row, attriName);
        double f = toFloat(data);
        if(compare(f,condition,CONDITION_TYPE) == true){
            result.push_back(row);
        }
    }
    return result;
}

Row RecordManager::findFreeListEnd(Table &table){
    Row row;
    long freeptr = table.freeList;
    while(true){
    memcpy(&row.value, buffermanager.readData("filename", freeptr), sizeof(row.value));
    freeptr += sizeof(row.value);
    memcpy(&row.ptr, buffermanager.readData("filename", freeptr), sizeof(row.ptr));
    if(row.ptr = -1) return
    freeptr = row.ptr;
    }
}

int RecordManager::deleteRow(Table &table, string attriName, string condition,int CONDITION_TYPE){
    int delete_num;
    Row old_row;
    Row row;
    string data;
    for(int i=0;i<table.recordNum;i++){
        row = nextRecord(table);
        data = attriInRecord(table, row, attriName);
        data = trim(data);
        if(compare(data,condition,CONDITION_TYPE) == true){
            if(old_row.ptr != 0 && old_row.ptr != -1){
                if(table.freeList == -1){
                    table.freeList = old_row.ptr;
                }
                else
            }
        }
    }
    return result;
}

















