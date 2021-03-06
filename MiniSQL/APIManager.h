//
//  APIManager.h
//  interpreter
//  API模块的功能:根据interpreter模块的结果，调用相应模块的函数
//  Created by 李了 on 14/11/1.
//  Copyright (c) 2014年 李了. All rights reserved.
//

#ifndef __interpreter__APIManager__
#define __interpreter__APIManager__

#include "PublicClass.h"
#include <string>

class APIManager
{
public:
    bool existTable(string tableName);//与catalog交互，返回表是否存在，1表示存在，0表示不存在
    bool existIndex(string indexName);//与catalog交互，判断某一索引名是否存在，存在则返回表的序号，否则－1
    bool existIndexAttr(string tableName, string attriName);//判断某一属性上是否已经有索引
    bool exsitAttrTable(string tableName, string attriName);//判断某一表中是否存在某一属性
    bool isUnique(string tableName, string attriName);//判断某一属性是否为unique
    
    Table & creatTable(Table &table);//与catalog模块交互建表
    bool dropTable(string tablename); //与catalog，index交互，删除索引；与catalog和record交互，删除表格数据
    
    bool createIndex(string indexName, string tableName, string attriName);//与catalog交互，创建index
    bool dropIndex(string indexName);//与catalog交互，删除index
    
    bool insertValue(string tablename, vector<string>& row);//与record、catalog、index模块交互，插入对应值
    bool uniqueValue(string tableName, vector<string>& row);//当属姓为unique时，判断插入值是否相等
    
    vector<Row> select(string tablename);                                    //与record交互，获取表中信息
    vector<Row> select(string tablename, vector<Conditions>& condition);     //根据条件获取表中信息
    void showResults(string tableName, vector<Row> row);
    
    int  deleteValue(string tablename);//与record和catalog交互，无条件删除表格数据：
                                       //调用record中的删除表格函数实现表格内容删除，调用index中的函数删除表格中索引项对应的索引
                                       //如果成功，返回被删除表格的元组数目；失败返回－1
    int  deleteValue(string tablename, vector<Conditions>& condition); //根据条件删除表格
    
    bool isValidInt(string input);
    bool isValidFloat(string input);
    bool checkInsertType(string tableName, vector<string>insert);//检查输入格式是否正确
    bool checkInsertNume(string tableName, vector<string>insert);//检查输入个数是否超过属性个数
};
#endif /* defined(__interpreter__APIManager__) */
