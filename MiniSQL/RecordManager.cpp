//
//  RecordManager.cpp
//  MiniSQL
//
//  Created by Myh on 11/13/14.
//  Copyright (c) 2014 Myh. All rights reserved.
//

#include "RecordManager.h"
using namespace std;

//插入record中的一个属性，当此属性为record中的最后一个属性时，生成一条记录
FILEPTR RecordManager::insertValues(Table &table, string s){
    static int curAttri = 0;
    static Row row;
    static Row old_row;
    
    if(table.dataEndPTR != -1)      //上一条记录有的话存在old_row里
        old_row = findRecord(table, table.dataEndPTR);
    
    if(table.attributes[curAttri].type == CHAR)
        s.resize(table.attributes[curAttri].length, ' ');
    
    if(curAttri != table.attriNum){
        row.value+=s;
        curAttri++;
    }
    
    if(curAttri == table.attriNum){
        curAttri = 0;       //重置计数器
        
        if(table.firstRow == -1) table.firstRow = 0;        //检测是否为第一条记录
        
        //插入一整条记录
        if(table.freeList == -1){       //freelist为空插入到文件尾
            
            //防止跨块读取
            if(table.fileEnd%BLOCKSIZE + table.eachRecordLength + 8 > BLOCKSIZE)
                table.fileEnd = BLOCKSIZE*(table.fileEnd/BLOCKSIZE+1);
            
            //更新上一条记录的指针
            if(old_row.value != ""){
                old_row.ptr = table.fileEnd;
                buffermanager.writeData(table.name+".table", table.dataEndPTR+table.eachRecordLength, (char*)&old_row.ptr, sizeof(row.ptr), 1);
            }
            
            buffermanager.writeData(table.name+".table", table.fileEnd, row.value.c_str(), table.eachRecordLength, 1);
            table.fileEnd += table.eachRecordLength;
            
            buffermanager.writeData(table.name+".table", table.fileEnd, (char*)&row.ptr, sizeof(row.ptr), 1);
            table.fileEnd += sizeof(row.ptr);
            
            old_row = row;
            row.value.clear();
            table.recordNum++;
            table.dataEndPTR = table.fileEnd-table.eachRecordLength-8;
            return table.dataEndPTR;
        }
        
        else{       //插入到freelist里
            FILEPTR curPtr = table.freeList;
            if(old_row.value != ""){    //插入之前有数据
                old_row.ptr = curPtr;
                buffermanager.writeData(table.name+".table", table.dataEndPTR+table.eachRecordLength, (char*)&old_row.ptr, sizeof(row.ptr), 1);
            }
            else{   //插入的是第一条数据
                table.firstRow = curPtr;
            }
            
            table.freeList = findRecord(table, curPtr).ptr;
            
            buffermanager.writeData(table.name+".table", curPtr, row.value.c_str(), table.eachRecordLength, 1);
            buffermanager.writeData(table.name+".table", curPtr+table.eachRecordLength, (char*)&row.ptr, 8, 1);
            old_row = row;
            table.dataEndPTR = curPtr;
            table.recordNum++;
            row.value.clear();
            return table.dataEndPTR;
        }
        
    }
    return false;
}

//重载insertValues
FILEPTR RecordManager::insertValues(Table &table, int num){
    string data = format(num);
    return insertValues(table, data);
}

FILEPTR RecordManager::insertValues(Table &table, double f){
    string data = format(f);
    return insertValues(table, data);
}

//返回该地址的记录
Row RecordManager::findRecord(Table &table, FILEPTR addr){
    Row &row = *new Row;
    if(addr%BLOCKSIZE + table.eachRecordLength + 8 > BLOCKSIZE)
        addr = BLOCKSIZE*(table.curPtr/BLOCKSIZE+1);
    char* chptr=buffermanager.readData(table.name+".table", addr);
    for(int i=0;i<table.eachRecordLength;i++){
        row.value+=chptr[i];
    }
    memcpy(&row.ptr, buffermanager.readData(table.name+".table", addr+table.eachRecordLength), sizeof(row.ptr));
    return row;
}

//返回下一条记录
Row RecordManager::nextRecord(Table &table){
    Row &row = *new Row;
    if(table.curPtr == -1) table.curPtr = table.firstRow;
    if(table.curPtr%BLOCKSIZE + table.eachRecordLength + 8 > BLOCKSIZE)
        table.curPtr = BLOCKSIZE*(table.curPtr/BLOCKSIZE+1);
    char* chptr=buffermanager.readData(table.name+".table", table.curPtr);
    for(int i=0;i<table.eachRecordLength;i++){
        row.value+=chptr[i];
    }
//    memcpy(&row.value, chptr, table.eachRecordLength);
    table.curPtr += table.eachRecordLength;
    memcpy(&row.ptr, buffermanager.readData(table.name+".table", table.curPtr), sizeof(row.ptr));
    table.curPtr = row.ptr;
    return row;
}

//从一条记录中提取一个属性对应的数据
string RecordManager::attriInRecord(Table &table, Row &row, string attriName){
    int x = catalogmanager.getAttriNum(table, attriName);
    int length = catalogmanager.lengthBeforeAttri(table, attriName);
    string s = row.value.substr(length,table.attributes[x].length);
    return s;
}

//比较函数
bool RecordManager::compare(string s, string condition, int CONDITION_TYPE){
    cout << s << condition << endl;
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
        case ALL:
            return true;
        default:
            return false;
    }
}
//重载比较函数
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
        case ALL:
            return true;
        default:
            return false;
    }
}
//重载比较函数
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
        case ALL:
            return true;
        default:
            return false;
    }
}

//查询函数，返回所有符合条件的记录
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
//重载查询函数
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
//重载查询函数
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

vector<Row> RecordManager::selectAll(Table &table){
    vector<Row> result;
    Row row;
    for(int i=0;i<table.recordNum;i++){
        row = nextRecord(table);
        result.push_back(row);
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
//重载
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
//重载
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

//删除所有符合条件的记录，返回删除的个数
int RecordManager::deleteRow(Table &table, string attriName, string condition,int CONDITION_TYPE){
    int delete_num = 0;
    Row old_row, row;
    FILEPTR old_ptr = -1, ptr = 0;
    string data;
    for(int i=0;i<table.recordNum;i++){
        //row=nextRecord(table)
        if(table.curPtr == -1) table.curPtr = table.firstRow;
        if(table.curPtr%BLOCKSIZE + table.eachRecordLength + 8 > BLOCKSIZE)
            table.curPtr = BLOCKSIZE*(table.curPtr/BLOCKSIZE+1);
        ptr = table.curPtr;
        char* chptr=buffermanager.readData(table.name+".table", table.curPtr);
        for(int i=0;i<table.eachRecordLength;i++){
            row.value+=chptr[i];
        }
        //    memcpy(&row.value, chptr, table.eachRecordLength);
        table.curPtr += table.eachRecordLength;
        memcpy(&row.ptr, buffermanager.readData(table.name+".table", table.curPtr), sizeof(row.ptr));
        table.curPtr = row.ptr;
        
        data = attriInRecord(table, row, attriName);
        data = trim(data);
//        cout<<data<<endl<<condition<<endl;
        if(compare(data,condition,CONDITION_TYPE) == true){
            if(old_ptr == -1){      //删除第一条记录
                FILEPTR ptrToFistRow = table.firstRow;
                table.firstRow = row.ptr;
                row.ptr = table.freeList;
                table.freeList = ptrToFistRow;
                buffermanager.writeData(table.name+".table", ptr+table.eachRecordLength, (char*)&row.ptr, sizeof(row.ptr), 1);
                ptr = -1;
            }
            if(old_ptr != -1){  //正常情况
                FILEPTR freeList = table.freeList;
                table.freeList = old_row.ptr;
                old_row.ptr = row.ptr;
                row.ptr = freeList;
                buffermanager.writeData(table.name+".table", old_ptr+table.eachRecordLength, (char*)&old_row.ptr, sizeof(row.ptr), 1);
                buffermanager.writeData(table.name+".table", ptr+table.eachRecordLength, (char*)&row.ptr, sizeof(row.ptr), 1);
            }
            delete_num++;
        }
        old_row = row;
        old_ptr = ptr;
        row.value.clear();
    }
    table.recordNum -= delete_num;
    return delete_num;
}
//重载
int RecordManager::deleteRow(Table &table, string attriName, int condition,int CONDITION_TYPE){
    int delete_num = 0;
    Row old_row, row;
    FILEPTR old_ptr = -1, ptr = 0;
    string data;
    int num;
    for(int i=0;i<table.recordNum;i++){
        //row=nextRecord(table)
        if(table.curPtr == -1) table.curPtr = table.firstRow;
        if(table.curPtr%BLOCKSIZE + table.eachRecordLength + 8 > BLOCKSIZE)
            table.curPtr = BLOCKSIZE*(table.curPtr/BLOCKSIZE+1);
        ptr = table.curPtr;
        char* chptr=buffermanager.readData(table.name+".table", table.curPtr);
        for(int i=0;i<table.eachRecordLength;i++){
            row.value+=chptr[i];
        }
        //    memcpy(&row.value, chptr, table.eachRecordLength);
        table.curPtr += table.eachRecordLength;
        memcpy(&row.ptr, buffermanager.readData(table.name+".table", table.curPtr), sizeof(row.ptr));
        table.curPtr = row.ptr;
        
        data = attriInRecord(table, row, attriName);
        num = toInt(data);
//        cout<<data<<endl<<condition<<endl;
        if(compare(num,condition,CONDITION_TYPE) == true){
            if(old_ptr == -1){      //删除第一条记录
                FILEPTR ptrToFistRow = table.firstRow;
                table.firstRow = row.ptr;
                row.ptr = table.freeList;
                table.freeList = ptrToFistRow;
                buffermanager.writeData(table.name+".table", ptr+table.eachRecordLength, (char*)&row.ptr, sizeof(row.ptr), 1);
                ptr = -1;
            }
            if(old_ptr != -1){  //正常情况
                FILEPTR freeList = table.freeList;
                table.freeList = old_row.ptr;
                old_row.ptr = row.ptr;
                row.ptr = freeList;
                buffermanager.writeData(table.name+".table", old_ptr+table.eachRecordLength, (char*)&old_row.ptr, sizeof(row.ptr), 1);
                buffermanager.writeData(table.name+".table", ptr+table.eachRecordLength, (char*)&row.ptr, sizeof(row.ptr), 1);
            }
            delete_num++;
        }
        old_row = row;
        old_ptr = ptr;
        row.value.clear();
    }
    table.recordNum -= delete_num;
    return delete_num;
}
//重载
int RecordManager::deleteRow(Table &table, string attriName, double condition,int CONDITION_TYPE){
    int delete_num = 0;
    Row old_row, row;
    FILEPTR old_ptr = -1, ptr = 0;
    string data;
    double f;
    for(int i=0;i<table.recordNum;i++){
        //row=nextRecord(table)
        if(table.curPtr == -1) table.curPtr = table.firstRow;
        if(table.curPtr%BLOCKSIZE + table.eachRecordLength + 8> BLOCKSIZE)
            table.curPtr = BLOCKSIZE*(table.curPtr/BLOCKSIZE+1);
        ptr = table.curPtr;
        char* chptr=buffermanager.readData(table.name+".table", table.curPtr);
        for(int i=0;i<table.eachRecordLength;i++){
            row.value+=chptr[i];
        }
        //    memcpy(&row.value, chptr, table.eachRecordLength);
        table.curPtr += table.eachRecordLength;
        memcpy(&row.ptr, buffermanager.readData(table.name+".table", table.curPtr), sizeof(row.ptr));
        table.curPtr = row.ptr;
        
        data = attriInRecord(table, row, attriName);
        f = toFloat(data);
//        cout<<data<<endl<<condition<<endl;
        if(compare(f,condition,CONDITION_TYPE) == true){
            if(old_ptr == -1){      //删除第一条记录
                FILEPTR ptrToFistRow = table.firstRow;
                table.firstRow = row.ptr;
                row.ptr = table.freeList;
                table.freeList = ptrToFistRow;
                buffermanager.writeData(table.name+".table", ptr+table.eachRecordLength, (char*)&row.ptr, sizeof(row.ptr), 1);
                ptr = -1;
            }
            if(old_ptr != -1){  //正常情况
                FILEPTR freeList = table.freeList;
                table.freeList = old_row.ptr;
                old_row.ptr = row.ptr;
                row.ptr = freeList;
                buffermanager.writeData(table.name+".table", old_ptr+table.eachRecordLength, (char*)&old_row.ptr, sizeof(row.ptr), 1);
                buffermanager.writeData(table.name+".table", ptr+table.eachRecordLength, (char*)&row.ptr, sizeof(row.ptr), 1);
            }
            delete_num++;
        }
        old_row = row;
        old_ptr = ptr;
        row.value.clear();
    }
    table.recordNum -= delete_num;
    return delete_num;
}

int RecordManager::deleteAllRow(Table &table){
    int delete_num = 0;
    Row old_row, row;
    FILEPTR old_ptr = -1, ptr = 0;
    for(int i=0;i<table.recordNum;i++){
        //row=nextRecord(table)
        if(table.curPtr == -1) table.curPtr = table.firstRow;
        if(table.curPtr%BLOCKSIZE + table.eachRecordLength + 8> BLOCKSIZE)
            table.curPtr = BLOCKSIZE*(table.curPtr/BLOCKSIZE+1);
        ptr = table.curPtr;
        char* chptr=buffermanager.readData(table.name+".table", table.curPtr);
        for(int i=0;i<table.eachRecordLength;i++){
            row.value+=chptr[i];
        }
        //    memcpy(&row.value, chptr, table.eachRecordLength);
        table.curPtr += table.eachRecordLength;
        memcpy(&row.ptr, buffermanager.readData(table.name+".table", table.curPtr), sizeof(row.ptr));
        table.curPtr = row.ptr;
        if(old_ptr == -1){      //删除第一条记录
            FILEPTR ptrToFistRow = table.firstRow;
            table.firstRow = row.ptr;
            row.ptr = table.freeList;
            table.freeList = ptrToFistRow;
            buffermanager.writeData(table.name+".table", ptr+table.eachRecordLength, (char*)&row.ptr, sizeof(row.ptr), 1);
            ptr = -1;
        }
        if(old_ptr != -1){  //正常情况
            FILEPTR freeList = table.freeList;
            table.freeList = old_row.ptr;
            old_row.ptr = row.ptr;
            row.ptr = freeList;
            buffermanager.writeData(table.name+".table", old_ptr+table.eachRecordLength, (char*)&old_row.ptr, sizeof(row.ptr), 1);
            buffermanager.writeData(table.name+".table", ptr+table.eachRecordLength, (char*)&row.ptr, sizeof(row.ptr), 1);
        }
        delete_num++;
        old_row = row;
        old_ptr = ptr;
        row.value.clear();
    }
    table.recordNum -= delete_num;
    return delete_num;
}


















