//
//  IndexManager.h
//  Database
//
//  Created by Starcalm on 10/31/14.
//  Copyright (c) 2014 Starcalm. All rights reserved.
//

#ifndef __Database__IndexManager__
#define __Database__IndexManager__

#include <iostream>
#include <vector>
#include <set>
#include <cmath>
#include <sstream>
#include <fstream>
#include "PublicClass.h"
#include <cstdio>
#include "RecordManager.h"
#include "BufferManager.h"
#include "CatalogManager.h"

using namespace std;

/* Stored in leaf node
 * blockNum pointers to the block which store the index
 * blockOffset sepcifies the address of the index in the block
 * indexName is the name of the index
 * talbeName is the name of the table, tableName-indexName.idx is the name of the index file
 * isValid is used in functions to determine whether this nodeData has data 
 */

enum NODE_TYPE { INNER, LEAF };
enum SIBLING_DIRECTION { LEFT, RIGHT };
typedef struct nodeData DataType; // data stored in leaf represent offset of records in the table, thus use int
typedef string KeyType; // all attributes are treated as string ( int and float are formatted into string )

/* Abstract nodes which inner and leaf nodes inherite from
 * Must implement removeKey, split, mergeChild, clear, borrowFrom, getChildIndex in child class
 */
class Node
{
public:
    Node();
    virtual ~Node();
    
    int getKeyIndex(KeyType key);

    virtual void removeKey(int keyIndex, int childIndex) = 0;  // 从结点中移除键值
    virtual void split(Node* parentNode, int childIndex) = 0; // 分裂结点
    virtual void mergeChild(Node* parentNode, Node* childNode, int keyIndex) = 0;  // 合并结点
    virtual void clear() = 0; // 清空结点，同时会清空结点所包含的子树结点
    virtual void borrowFrom(Node* destNode, Node* parentNode, int keyIndex, SIBLING_DIRECTION d) = 0; // 从兄弟结点中借一个键值
    virtual int getChildIndex(KeyType key, int keyIndex)const = 0;  // 根据键值获取孩子结点指针下标
    
    int selfBlockNum;
    vector<int> childrenBLockNums;
    int parentBlockNum;
    
    NODE_TYPE m_nodeType;
    int m_keyNum;
    vector<KeyType> m_keyValues;
    int maxLeafDataNum;
    int minLeafDataNum;
    int maxChildNum;
    int minChildNum;
    int maxKeyValueNum;
    int minKeyValueNum;
    int leafDataLength;
};

class InnerNode : public Node
{
public:
    InnerNode(int);
    virtual ~InnerNode();
    
    void insert(int keyIndex, int childIndex, KeyType key, Node * childNode);
    virtual void split(Node* parentNode, int childIndex);
    virtual void mergeChild(Node* parentNode, Node* childNode, int keyIndex);
    virtual void removeKey(int keyIndex, int childIndex);
    virtual void clear();
    virtual void borrowFrom(Node* destNode, Node* parentNode, int keyIndex, SIBLING_DIRECTION d);
    virtual int getChildIndex(KeyType key, int keyIndex)const;
    
    vector<Node *> m_children;
};

class LeafNode : public Node
{
public:
    LeafNode(int);
    virtual ~LeafNode();
    
    void insert(KeyType key, const DataType & data);
    virtual void split(Node* parentNode, int childIndex);
    virtual void mergeChild(Node* parentNode, Node* childNode, int keyIndex);
    virtual void removeKey(int keyIndex, int childIndex);
    virtual void clear();
    virtual void borrowFrom(Node* destNode, Node* parentNode, int keyIndex, SIBLING_DIRECTION d);
    virtual int getChildIndex(KeyType key, int keyIndex)const;
    
    LeafNode* m_leftSibling;
    LeafNode* m_rightSibling;
    vector<DataType>  m_data;
};

const int INVALID_INDEX = -1;

struct SelectResult
{
    int keyIndex;
    LeafNode* targetNode;
};

class BPlusTree
{
public:
    BPlusTree(int);
    ~BPlusTree();
    
    bool insertValue(KeyType key, const DataType & data); // insert value into b+ tree
    bool deleteValue(KeyType key); // delete value from b+ tree
    bool updateValue(KeyType oldKey, KeyType newKey);
    bool search(KeyType key);
    void clear();
    nodeData findNodeData(KeyType key);
    nodeData recursive_findNodeData(Node * pNode, KeyType key);
    Node * findNodeFor(KeyType key);
    Node * recursive_findNodeFor(Node * node, KeyType key);
    vector<nodeData> findRangeNodeData(Node * pNode, KeyType key, CONDITION_TYPE condType);

    void recursive_insert(Node* parentNode, KeyType key, const DataType& data);
    void recursive_remove(Node* parentNode, KeyType key);
    bool recursive_search(Node *pNode, KeyType key)const;
    void changeKey(Node *pNode, KeyType oldKey, KeyType newKey);
//    void search(KeyType key, SelectResult& result);
    void recursive_search(Node* pNode, KeyType key, SelectResult& result);
    void remove(KeyType key, DataType& dataValue);
    void recursive_remove(Node* parentNode, KeyType key, DataType& dataValue);
    
    void printData() const;
    
    Node * m_root;
    LeafNode * m_dataHead;
    KeyType m_maxKey;
    int leafDataLength;
    static int blockCounter;
};

class IndexManager
{
public:
    IndexManager(BufferManager &bm, CatalogManager &cm, RecordManager &rm);
    ~IndexManager();
    
    // format int and float numbers in order to compare them in string
    // do nothing to string
//    friend string format(int & num); // pad 0 to end of num until 11 digits
//    friend string format(float & num); // pad 0 to end of num until 40 digits, with 7 decimal digits
    void traverse(Node *, string fileName);
    vector<nodeData> findAttributeValues(string, Table &, string);
    nodeData transferAddrToNodeData(Attribute &attri, Table &table, FILEPTR addr);
    
    // public API
    bool createIndex(string indexName, Table &table, string attributeName, int maxLength);
    bool dropIndex(string indexName, string tableName);
    // select operation
    // return nodeData which contain filePtr to the record 
    nodeData findEqualRecord(Attribute attribute, string attributeValue, Table & table);
    vector<Row> findRangeRecord(Attribute attribute, string attributeValue, Table & table, CONDITION_TYPE condition);

    // maintain indices after insert, delete and update operation
    void afterInsert(Attribute & attribute, string attributeValue, Table & table, FILEPTR addr);
    void afterDelete(Attribute & attribute, string attributeValue, Table & table, FILEPTR addr);
    void afterUpdate(Attribute & attribute, string attributeValue, Table & table, FILEPTR addr);
    
    set<string> indexSet; // store all index(indexName)
    set<BPlusTree *> btreeSet;
    BufferManager & bm;
    RecordManager & rm;
    CatalogManager & cm;
};

#endif /* defined(__Database__IndexManager__) */
