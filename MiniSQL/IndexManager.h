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
#include <cmath>

using namespace std;

enum NODE_TYPE { INNER, LEAF };
enum SIBLING_DIRECTION { LEFT, RIGHT };
enum COMPARE_RESULT { EQUAL, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL, NOT_EQUAL };
const int KEYNUM = 20;
const int ORDER = 7;                   // B+树的阶（非根内结点的最小子树个数）
const int MINIMUM_KEY = ORDER-1;        // 最小键值个数
const int MAXIMUM_KEY = 2*ORDER-1;      // 最大键值个数
const int MINIMUM_CHILD = MINIMUM_KEY+1; // 最小子树个数
const int MAXIMUM_CHILD = MAXIMUM_KEY+1; // 最大子树个数
const int MINIMUM_LEAF = MINIMUM_KEY;    // 最小叶子结点键值个数
const int MAXIMUM_LEAF = MAXIMUM_KEY;    // 最大叶子结点键值个数
typedef int DataType; // data stored in leaf represent offset of records in the table, thus use int
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
    friend COMPARE_RESULT compare(KeyType key1, KeyType key2);

    virtual void removeKey(int keyIndex, int childIndex) = 0;  // 从结点中移除键值
    virtual void split(Node* parentNode, int childIndex) = 0; // 分裂结点
    virtual void mergeChild(Node* parentNode, Node* childNode, int keyIndex) = 0;  // 合并结点
    virtual void clear() = 0; // 清空结点，同时会清空结点所包含的子树结点
    virtual void borrowFrom(Node* destNode, Node* parentNode, int keyIndex, SIBLING_DIRECTION d) = 0; // 从兄弟结点中借一个键值
    virtual int getChildIndex(KeyType key, int keyIndex)const = 0;  // 根据键值获取孩子结点指针下标
    
    NODE_TYPE m_nodeType;
    int m_keyNum;
    KeyType m_keyValues[MAXIMUM_KEY];
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
//    vector<DataType> select(KeyType compareKey, COMPARE_OPERATOR compareOperator);
//    vector<DataType> select(KeyType smallestKey, KeyType largestKey);
    bool search(KeyType key);
    void clear();
    
    void recursive_insert(Node* parentNode, KeyType key, const DataType& data);
    void recursive_remove(Node* parentNode, KeyType key);
    bool recursive_search(Node *pNode, KeyType key)const;
    void changeKey(Node *pNode, KeyType oldKey, KeyType newKey);
    void search(KeyType key, SelectResult& result);
    void recursive_search(Node* pNode, KeyType key, SelectResult& result);
    void remove(KeyType key, DataType& dataValue);
    void recursive_remove(Node* parentNode, KeyType key, DataType& dataValue);
    
    Node * m_root;
    LeafNode * m_dataHead;
    KeyType m_maxKey;
    int leafDataLength;
};

class IndexManager
{
public:
    IndexManager();
    ~IndexManager();
    
    bool createIndex(string indexName, string tableName, vector<string> attributes, bool isUnique = false);
    bool dropIndex(string indexName, string tableName);
private:
    string tableName;
    string attributeName;
//    BufferManager * bm;
};

#endif /* defined(__Database__IndexManager__) */
