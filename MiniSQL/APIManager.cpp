//
//  APIManager.cpp
//  interpreter
//
//  Created by 李了 on 14/11/1.
//  Copyright (c) 2014年 李了. All rights reserved.
//

#include "APIManager.h"



BufferManager buffermanager;
CatalogManager catalogmanager(buffermanager);
RecordManager recordmanager(buffermanager, catalogmanager);
IndexManager indexmanager(buffermanager, catalogmanager, recordmanager);


//与catalog交互，返回表是否存在，1表示存在，0表示不存在
bool APIManager:: existTable(string tableName)
{
    if(catalogmanager.findTable(tableName) != -1)
        return true;
    else
        return false;
}

//与catalog交互，判断某一索引名是否存在，存在则返回表的序号，否则－1
bool APIManager:: existIndex(string IndexName)
{
    if(catalogmanager.findIndexTable(IndexName))
        return true;
    else
        return false;
}


bool APIManager:: existIndexAttr(string tableName, string attriName)
{
    int tableIndex = catalogmanager.findTable(tableName);
    int attriNum = catalogmanager.getAttriNum(catalogmanager.Vtable[tableIndex], attriName);
    if(catalogmanager.Vtable[tableIndex].attributes[attriNum].indexName!= "NULL")
    {
        return true;
    }
    return false;
}


bool APIManager:: isUnique(string tableName, string attriName)
{
    int tableIndex = catalogmanager.findTable(tableName);
    int attriNum = catalogmanager.getAttriNum(catalogmanager.Vtable[tableIndex], attriName);
    if(catalogmanager.Vtable[tableIndex].attributes[attriNum].isUnique)
        return true;
    else
        return false;
}

//在interpreter中检测table是否存在
//与catalog模块交互建表
Table APIManager:: creatTable(Table &table)
{
    Table & tempTable =  catalogmanager.createTable(table.name, table.primaryKey);
    for(size_t i = 0; i < table.attributes.size(); ++i)
    {
        catalogmanager.insertAttri(tempTable, table.attributes[i].name, table.attributes[i].type, table.attributes[i].length);
        if (table.primaryKey == table.attributes[i].name) // if attribute is primary key 
        {
            table.attributes[i].isPrimaryKey = true;
            table.attributes[i].isUnique = true;
        }
    }
    
    return catalogmanager.Vtable.back();
}

//在interpreter中检测indexname是否合法，table是否存在，attribute是否存在
//与catalog交互，创建index
bool APIManager:: createIndex(string indexName, string tableName, string attriName)
{
    return catalogmanager.createIndex(indexName, tableName, attriName);
}

//在interpreter中检测indexname是否合法，table是否存在，index是否存在
//与catalog交互，删除index
bool APIManager:: dropIndex(string indexName)
{
    int i = catalogmanager.findIndexTable(indexName);
    string tableName = catalogmanager.Vtable[i].name;
    return indexmanager.dropIndex(indexName, tableName);
}

//在interpreter中检测table是否存在
//与catalog，index交互，删除索引；与catalog和record交互，删除表格数据
bool APIManager:: dropTable(string tableName)
{
    return catalogmanager.dropTable(tableName);
}

//在interpreter中检测table是否存在
//与rm交互删除表中所有数据
//与im交互删除index（如果存在）
int APIManager:: deleteValue(string tablename)
{
    int tableIndex = catalogmanager.findTable(tablename);
    int total = recordmanager.deleteAllRow(catalogmanager.Vtable[tableIndex]);
    return total;
}

//在interpreter中检测table是否存在
//与rm交互，根据条件删除表中数据
//与im交互删除index（如果存在）
int APIManager::deleteValue(string tablename, vector<Conditions> &condition)
{
    //Table table;
    int total = 0;
    int tableIndex = catalogmanager.findTable(tablename);
    //table = catalogmanager.Vtable[i];
    for(size_t i =0; i < condition.size(); ++i)
    {
        //判断条件中的属性名在表格中是否存在
        /*if (catalogmanager.getAttriNum(catalogmanager.Vtable[tableIndex], condition[i].attribute) == -1)
        {
            
            return -1;
        }*/
        total += recordmanager.deleteRow(catalogmanager.Vtable[tableIndex], condition[i].attribute, condition[i].attributeValue, condition[i].condition_type);
    }
    return total;
}

//在interpreter中检测表是否存在，插入值格式是否正确
//与record、catalog、index模块交互，插入对应值
bool APIManager:: insertValue(string tablename, vector<string> &row)
{
    int type;
    int tableIndex;
    int intValue = 0;
    float floatValue = 0;
    tableIndex = catalogmanager.findTable(tablename);
    bool indexflag = false;
    int attributeIndex = -1;//如果存在索引，对应的属性位置
    string attributeValue;//如果存在索引，对应的属性值
    // Table table;
    FILEPTR add = 0;
    //table = catalogmanager.Vtable[tableIndex];
    for(size_t i = 0; i < row.size(); ++i)//插入一条记录
    {
        type = catalogmanager.Vtable[tableIndex].attributes[i].type;
        switch (type) {
            case INT:
                intValue = toInt(row[i]);
                add = recordmanager.insertValues(catalogmanager.Vtable[tableIndex], intValue);
                break;
            case FLOAT:
                floatValue = toFloat(row[i]);
                add = recordmanager.insertValues(catalogmanager.Vtable[tableIndex], floatValue);
                break;
            case CHAR:
                row[i] = row[i].substr(row[i].find_first_of("'")+1, row[i].find_last_of("'") - row[i].find_first_of("'") - 1);
                add = recordmanager.insertValues(catalogmanager.Vtable[tableIndex], row[i]);
                break;
        }
        if(catalogmanager.Vtable[tableIndex].attributes[i].indexName != "NULL")//存在索引
        {
            indexflag = true;
            attributeIndex = (int)i;
            switch (type) {
                case INT:
                    attributeValue = format(intValue);
                    break;
                case FLOAT:
                    attributeValue = format(floatValue);
                case CHAR:
                    attributeValue = row[i];
                    break;
            }
        }
    }
    if(indexflag)
    {
        indexmanager.afterInsert(catalogmanager.Vtable[tableIndex].attributes[attributeIndex], attributeValue, catalogmanager.Vtable[tableIndex], add);
    }
    
    return true;
}

//在interpreter中检测table是否存在
//与record交互，获取表中信息
vector<Row> APIManager::select(string tablename)
{
    int tableIndex = catalogmanager.findTable(tablename);
    vector<Row> result;
    result = recordmanager.selectAll(catalogmanager.Vtable[tableIndex]);
    /*for(size_t i = 0; i < recordmanager.selectAll(catalogmanager.Vtable[tableIndex]).size(); ++i)
    {
        result.push_back(recordmanager.selectAll(catalogmanager.Vtable[tableIndex])[i]);
    }*/
    return result;
}

//在interpreter中检测table是否存在
//与record交互，根据条件获取表中信息
vector<Row> APIManager::select(string tablename, vector<Conditions>& condition)
{
    int tableIndex = catalogmanager.findTable(tablename);
    vector<Row> result;
    int type;
    type = condition[0].condition_type;
    switch (type) {
        case INT:
            result = recordmanager.select(catalogmanager.Vtable[tableIndex], condition[0].attribute, toInt(condition[0].attributeValue), INT);
            break;
        case FLOAT:
            result = recordmanager.select(catalogmanager.Vtable[tableIndex], condition[0].attribute, toFloat(condition[0].attributeValue), FLOAT);
            break;
        case CHAR:
            result = recordmanager.select(catalogmanager.Vtable[tableIndex], condition[0].attribute, condition[0].attributeValue, CHAR);
            break;
    }
    if(condition.size() > 1)
    {
        for(size_t i = 1; i < condition.size(); ++i)
        {
            type = condition[i].condition_type;
            switch (type) {
                case INT:
                    result = recordmanager.select(catalogmanager.Vtable[tableIndex], result, condition[i].attribute, toInt(condition[i].attributeValue), INT);
                    break;
                case FLOAT:
                    result = recordmanager.select(catalogmanager.Vtable[tableIndex], result, condition[i].attribute, toFloat(condition[i].attributeValue), FLOAT);
                    break;
                case CHAR:
                    result = recordmanager.select(catalogmanager.Vtable[tableIndex], result, condition[i].attribute, condition[i].attributeValue, CHAR);
                    break;
            }
        }
    }
    return result;
}

//总长度不超过11位
bool APIManager:: isValidInt(const string s)
{
    if(s.size() == 0 || s.size() > 11)
    {
        return false;
    }
    else
    {
        if((s[0] != '-') &&(!isdigit(s[0])))
        {
            return false;
        }
        else
        {
            for(size_t i = 1; i < s.size(); ++i)
            {
                if(!isdigit(s[i]))
                    return false;
            }
        }
    }
    return true;
}

//判断字符串中是否有数字之外字符（首位负号除外）；判断字符串中小数点数量是否超过1；判断小数点之前是否超过33位
bool APIManager:: isValidFloat(const string s)
{
    int count = 0;
    if(s.size() == 0)
    {
        return false;
    }
    else
    {
        if((s[0] != '-') &&(!isdigit(s[0])))
        {
            return false;
        }
        else
        {
            for(size_t i = 1; i < s.size(); ++i)
            {
                if((!isdigit(s[i]) && (s[i]!= '.')))
                    return false;
                else
                {
                    if(s[i] == '.')
                        count++;
                }
            }
            if(count > 1)
                return false;
            else
            {
                for(size_t i = 1; i < s.size(); ++i)
                {
                    if(s[i] == '.')
                    {
                        string beforeDot = s.substr(0, i);
                        //string afterDot = s.substr(i+1, s.size() - i - 1);
                        if(beforeDot.size()>33)
                            return false;
                    }
                }
            }
        }
    }
    return true;
}

//默认不支持空值
bool APIManager:: checkInsertNume(string tableName, vector<string> insert)
{
    int tableIndex = catalogmanager.findTable(tableName);
    int arrinum = catalogmanager.Vtable[tableIndex].attriNum;
    if((int)insert.size()!=arrinum)
        return false;
    else
    return true;
}

//CHAR 类型的长度限制在interpreter部分实现
bool APIManager:: checkInsertType(string tableName, vector<string> insert)
{
    int tableIndex = catalogmanager.findTable(tableName);
    int attrType;
    int arrNum = catalogmanager.Vtable[tableIndex].attriNum;
    for(int i = 0; i< arrNum; ++i )
    {
        attrType = catalogmanager.Vtable[tableIndex].attributes[i].type;
        switch (attrType) {
            case INT:
                if (!isValidInt(insert[i])) {
                    return false;
                }
                break;
            case FLOAT:
                if (!isValidFloat(insert[i])) {
                    return false;
                }
            default:
                break;
        }
    }
    return true;
}

