//
//  IndexManager.cpp
//  Database
//
//  Created by Starcalm on 10/31/14.
//  Copyright (c) 2014 Starcalm. All rights reserved.
//

#include "IndexManager.h"

// Node.h
Node::Node()
{
    m_nodeType = LEAF; // set node type
    m_keyNum = 0; // set keynum
}

Node::~Node()
{
    m_keyNum = 0;
}

int Node::getKeyIndex(KeyType key) // use binary search to find key
{
    int left = 0;
    int right = m_keyNum - 1;
    int current;
    while (left != right) // binary search
    {
        current = (left + right) / 2;
        KeyType currentKey = m_keyValues[current];
        if (key > currentKey)
        {
            left = current + 1;
        }
        else
        {
            right = current;
        }
    }
    return left;
}

// Innernode.h
InnerNode::InnerNode(int leafDataLength)
{
    m_nodeType = INNER;
    this->leafDataLength = leafDataLength;
    // a tree node is 4096 bytes and comprises n keyvalues and n+1 recordPointers (which is int)
    // thus leafDataLength * n + 4*(n+1) = 4096
    maxLeafDataNum = (4096 - 4) / (this->leafDataLength + 4);
    minLeafDataNum = ceil(maxLeafDataNum / 2);
    minKeyValueNum = minLeafDataNum;
    maxKeyValueNum = maxLeafDataNum;
    maxChildNum = maxKeyValueNum + 1;
    minChildNum = minKeyValueNum + 1;
    m_keyValues.resize(maxKeyValueNum);
}

InnerNode::~InnerNode()
{
}

void InnerNode::clear() // clear this node
{
    for (int i = 0; i < m_children.size(); i++)
    {
        m_children.at(i) ->clear(); // clear its children's nodes first
        delete m_children.at(i);
        m_children.at(i) = NULL;
    }
}

void InnerNode::split(Node * parent, int childIndex)
{
    InnerNode * newNode = new InnerNode(parent->leafDataLength);
    newNode->m_nodeType = INNER; // set new node's type
    newNode->m_keyNum = minKeyValueNum;
    newNode->selfBlockNum = BPlusTree::blockCounter++;
    newNode->parentBlockNum = parent->selfBlockNum;
    for (int i = 0; i < minKeyValueNum; ++i)  // copy the second half of the keys to the new node
    {
        newNode->m_keyValues[i] = m_keyValues[i + minKeyValueNum];
    }
    for (int i = 0; i < minChildNum; ++i) // copy the second half of the children to the new node
    {
        newNode->m_children.push_back(m_children[i + minChildNum]);
        newNode->childrenBLockNums.push_back(m_children[i + minChildNum]->selfBlockNum);
        m_children[i + minChildNum]->parentBlockNum = newNode->selfBlockNum;
    }
    m_keyNum = minKeyValueNum; // reset the key number of the old node
    ((InnerNode *) parent)->insert(childIndex, childIndex + 1, m_keyValues[minKeyValueNum], newNode); // insert new node to the tree
}

// insert key after keyIndex and childNode afater childIndex
void InnerNode::insert(int keyIndex, int childIndex, KeyType key, Node *childNode)
{
    // if keyIndex == m_keyNum, the node would be inserted into children[m_keyNum]
    int i = m_keyNum;
    for (; i > keyIndex; --i) // move keys and children back a unit whose indices are greater than keyIndex and childIndex
    {
        m_children[i + 1] = m_children[i];
        m_keyValues[i] = m_keyValues[i - 1];
    }
    if (i == childIndex) // if keyIndex and childIndex both equal 0, then move m_children[0] back to m_children[1]
    {
        m_children[i + 1] = m_children[i];
    }
    // reset parameters of inserted node
    m_children[childIndex] = childNode;
    m_keyValues[keyIndex] = key;
    m_keyNum += 1;
}

void InnerNode::mergeChild(Node *parentNode, Node *childNode, int keyIndex)
{
    // insert the first key and child of the second node into the first node
    insert(minKeyValueNum, minKeyValueNum + 1, parentNode->m_keyValues[keyIndex], ((InnerNode*)childNode)->m_children[0]);
    // insert next keys and children
    for (int i = 1; i < childNode->m_keyNum; ++i)
    {
        insert(minKeyValueNum + i, minKeyValueNum + i + 1, childNode->m_keyValues[i - 1], ((InnerNode *)childNode)->m_children[i]);
    }
    // remove the key and child in parent node
    parentNode->removeKey(keyIndex, keyIndex + 1);
//    delete ((InnerNode *)parentNode)->m_children[keyIndex + 1];
    for (vector<int>::iterator iter = parentNode->childrenBLockNums.begin(); iter != parentNode->childrenBLockNums.end(); iter++)
    {
        if (*iter == childNode->selfBlockNum) {
            parentNode->childrenBLockNums.erase(iter);
        }
    } // remove the child's block num
}

void InnerNode::removeKey(int keyIndex, int childIndex)
{
    // move later keys and children ahead
    for (int i = 0; i < m_keyNum - keyIndex - 1; ++i)
    {
        m_keyValues[i + keyIndex] = m_keyValues[i + keyIndex + 1];
        m_children[i + childIndex] = m_children[i + childIndex + 1];
    }
    m_keyNum--;
}

void InnerNode::borrowFrom(Node * siblingNode, Node * parentNode, int keyIndex, SIBLING_DIRECTION d)
{
    switch (d)
    {
        case LEFT: // borrow from left child node
            // insert the last key and child of the left node into this node
//            insert(0, 0, parentNode->m_keyValues[keyIndex], ((InnerNode *)siblingNode)->m_children[m_keyNum]);
            insert(0, 0, siblingNode->m_keyValues[m_keyNum - 1], ((InnerNode *)siblingNode)->m_children[m_keyNum]);
            parentNode->m_keyValues[keyIndex] = siblingNode->m_keyValues[m_keyNum - 1];
            siblingNode->removeKey(siblingNode->m_keyNum - 1, siblingNode->m_keyNum);
            break;
        case RIGHT: // borrow from right child node
            // insert into the m_keyNum's position
            insert(m_keyNum, m_keyNum + 1, parentNode->m_keyValues[keyIndex], ((InnerNode *)siblingNode)->m_children[0]);
            parentNode->m_keyValues[keyIndex] = siblingNode->m_keyValues[0];
            siblingNode->removeKey(0, 0);
        default:
            break;
    }
}

int InnerNode::getChildIndex(KeyType key, int keyIndex) const
{
//    if (GREATER_EQUAL == compare(key, m_keyValues[keyIndex]))
    if (key >= m_keyValues[keyIndex])
    {
        return keyIndex + 1;
    }
    else
    {
        return keyIndex;
    }
}

// LeafNode.h

LeafNode::LeafNode(int leafDataLength) : Node()
{
    m_nodeType = LEAF;
    m_leftSibling = NULL;
    m_rightSibling = NULL;
    this->leafDataLength = leafDataLength;
    // a tree node is 4096 bytes and comprises n keyvalues and n+1 recordPointers (which is int)
    // thus leafDataLength * n + 4*(n+1) = 4096
    maxLeafDataNum = (4096 - 4) / (this->leafDataLength + 4);
    minLeafDataNum = ceil(maxLeafDataNum / 2);
    minKeyValueNum = minLeafDataNum;
    maxKeyValueNum = maxLeafDataNum;
    maxChildNum = maxKeyValueNum + 1;
    minChildNum = minKeyValueNum + 1;
    m_keyValues.resize(maxKeyValueNum);
    m_data.resize(maxLeafDataNum);
}

LeafNode::~LeafNode()
{
}

void LeafNode::clear()
{
    this->m_data.clear();
}

void LeafNode::insert(KeyType key, const DataType & data)
{
    int i;
    for (i = m_keyNum; i >= 1 && m_keyValues[i - 1] > key; --i)
    {
        m_keyValues[i] = m_keyValues[i - 1];
        m_data[i] = m_data[i - 1];
    }
    this->m_data[i] = data;
    this->m_keyValues[i] = key;
    this->m_keyNum++;
}

void LeafNode::split(Node *parentNode, int childIndex)
{
    LeafNode *newLeafNode = new LeafNode(parentNode->leafDataLength);
    // set attributes of new node and old node
    this->m_keyNum = minLeafDataNum;
    newLeafNode->m_keyNum = minLeafDataNum + 1;
    newLeafNode->m_rightSibling = this->m_rightSibling;
    newLeafNode->m_leftSibling = this;
    newLeafNode->selfBlockNum = BPlusTree::blockCounter++;
    newLeafNode->parentBlockNum = parentNode->selfBlockNum;
    parentNode->childrenBLockNums.push_back(newLeafNode->selfBlockNum);
    this->m_rightSibling = newLeafNode;
    for (int i = 0; i < minLeafDataNum + 1; ++i) // copy key values
    {
        newLeafNode->m_keyValues[i] = m_keyValues[i + minLeafDataNum];
    }
    for (int i = 0; i < minLeafDataNum + 1; ++i) // copy data
    {
        newLeafNode->m_data[i] = m_data[i + minLeafDataNum];
    }
    ((InnerNode *)parentNode)->insert(childIndex, childIndex + 1, m_keyValues[minLeafDataNum], newLeafNode);
}

void LeafNode::mergeChild(Node *parentNode, Node *childNode, int keyIndex) // insert values of right sibling into this node
{
    for (int i = 0 ; i < childNode->m_keyNum; ++i)
    {
        insert(childNode->m_keyValues[i], ((LeafNode *)childNode)->m_data[i]);
    }
    this->m_rightSibling = ((LeafNode *)childNode)->m_rightSibling;
    parentNode->removeKey(keyIndex, keyIndex+1);
    for (vector<int>::iterator iter = parentNode->childrenBLockNums.begin(); iter != parentNode->childrenBLockNums.end(); iter++)
    {
        if (*iter == childNode->selfBlockNum) {
            parentNode->childrenBLockNums.erase(iter);
        }
    } // remove the child's block num
}

void LeafNode::removeKey(int keyIdex, int childIndex) // remove key and data
{
    for (int i = keyIdex; i < this->m_keyNum - 1; ++i) {
        this->m_keyValues[i] = this->m_keyValues[i + 1];
        this->m_data[i] = this->m_data[i + 1];
    }
    this->m_keyNum--;
}

void LeafNode::borrowFrom(Node *siblingNode, Node *parentNode, int keyIndex, SIBLING_DIRECTION d)
{
    switch (d) {
        case LEFT: // borrow from left sibling
            insert(siblingNode->m_keyValues[this->m_keyNum - 1], ((LeafNode *)siblingNode)->m_data[this->m_keyNum - 1]);
            siblingNode->removeKey(m_keyNum - 1, m_keyNum - 1);
            parentNode->m_keyValues[keyIndex] = m_keyValues[0];
            break;
        case RIGHT: // borrow from right sibling
            insert(siblingNode->m_keyValues[0], ((LeafNode *)siblingNode)->m_data[0]);
            siblingNode->removeKey(0, 0);
            parentNode->m_keyValues[keyIndex] = m_keyValues[0];
        default:
            break;
    }
}

int LeafNode::getChildIndex(KeyType key, int keyIndex) const
{
    return keyIndex;
}

// BPlusTree.h

BPlusTree::BPlusTree(int leafDataLength)
{
    m_root = NULL;
    m_dataHead = NULL;
    this->leafDataLength = leafDataLength;
}

int BPlusTree::blockCounter = 0;

BPlusTree::~BPlusTree()
{
    clear();
}

bool BPlusTree::insertValue(KeyType key, const DataType & data)
{
    if (search(key)) // key is already in the tree
    {
        return  false;
    }
    if (NULL == m_root) // there is no root
    {
        m_root = new LeafNode(leafDataLength);
        m_root->selfBlockNum = blockCounter;
        m_root->parentBlockNum = -1;
        m_root->childrenBLockNums.clear();
        blockCounter++;
        m_dataHead = (LeafNode *)m_root;
        m_maxKey = key;
    }
    if (m_root->m_keyNum >= m_root->maxKeyValueNum) // root is full
    {
        InnerNode * newNode = new InnerNode(leafDataLength);
        newNode->selfBlockNum = blockCounter;
        newNode->parentBlockNum = -1;
        newNode->childrenBLockNums.push_back(m_root->selfBlockNum);
        blockCounter++;
        newNode->m_children[0] = m_root;
        m_root->split(newNode, 0);
        m_root = newNode;
    }
    if (key > m_maxKey)
    {
        m_maxKey = key;
    }
    recursive_insert(m_root, key, data);
    return true;
}

void BPlusTree::recursive_insert(Node * parentNode, KeyType key, const DataType & data)
{
    if (LEAF == parentNode->m_nodeType)
    {
        ((LeafNode *)parentNode)->insert(key, data); // insert directly
    }
    else
    {
        int keyIndex = parentNode->getKeyIndex(key);
        int childIndex = parentNode->getChildIndex(key, keyIndex);
        Node * childNode = ((InnerNode *)parentNode)->m_children[childIndex];
        if (childNode->m_keyNum >= childNode->maxLeafDataNum)
        {
            childNode->split(parentNode, childIndex);
            if (parentNode->m_keyValues[childIndex] >= key)
            {
                childNode = ((InnerNode *)parentNode)->m_children[childIndex + 1];
            }
        }
        recursive_insert(childNode, key, data);
    }
}

void BPlusTree::clear()
{
    if (NULL != m_root) {
        m_root->clear();
        delete m_root;
        m_root = NULL;
        m_dataHead = NULL;
    }
}

bool BPlusTree::search(KeyType key)
{
    return recursive_search(m_root, key);
}

bool BPlusTree::recursive_search(Node *pNode, KeyType key) const
{
    if (NULL == pNode)
        return false;
    else
    {
        int keyIndex = pNode->getKeyIndex(key);
        int childIndex = pNode->getChildIndex(key, keyIndex);
        
        if (keyIndex < pNode->m_keyNum && key == pNode->m_keyValues[keyIndex])
            return true;
        else
        {
            if (LEAF == pNode->m_nodeType)
                return false;
            else
                return recursive_search(((InnerNode *)pNode)->m_children[childIndex], key);
        }
    }
}

Node * BPlusTree::findNodeFor(KeyType key)
{
    return recursive_findNodeFor(m_root, key);
}

Node * BPlusTree::recursive_findNodeFor(Node * node, KeyType key)
{
    int keyIndex = m_root->getKeyIndex(key);
    int childIndex = m_root->getChildIndex(key, keyIndex);
    if (keyIndex < m_root->m_keyNum && key == m_root->m_keyValues[keyIndex])
    {
        return ((InnerNode *)m_root)->m_children[childIndex];
    }
    else return recursive_findNodeFor(((InnerNode *)m_root)->m_children[childIndex], key);
}

nodeData BPlusTree::findNodeData(KeyType key)
{
    return recursive_findNodeData(m_root, key);
}
// find record pointer correspoding to the key
nodeData BPlusTree::recursive_findNodeData(Node * pNode, KeyType key)
{
//    if (NULL == pNode)
//        return ;
//    else
//    {
        int keyIndex = pNode->getKeyIndex(key);
        int childIndex = pNode->getChildIndex(key, keyIndex);
        nodeData invalid;
        invalid.isValid = false;
    
        if (keyIndex < pNode->m_keyNum && key == pNode->m_keyValues[keyIndex])
            return ((LeafNode *)pNode)->m_data[keyIndex];
        else
        {
            if (LEAF == pNode->m_nodeType)
                return invalid;
            else
                return recursive_findNodeData(((InnerNode *)pNode)->m_children[childIndex], key);
        }
//    }
}

vector<nodeData> BPlusTree::findRangeNodeData(Node * pNode, KeyType key, CONDITION_TYPE condType)
{
    int keyindex = pNode->getKeyIndex(key);
    int childIndex = pNode->getChildIndex(key, keyindex);
    nodeData end;
    vector<nodeData> results;
    if (keyindex < pNode->m_keyNum && key == pNode->m_keyValues[keyindex]) // find the key
    {
        switch (condType) {
            case SMALLER:
            {
                LeafNode * iter = m_dataHead;
                for (; iter->selfBlockNum < pNode->selfBlockNum; iter++) {
                    for (int i = 0; i < iter->m_keyNum; ++i)
                    {
                        results.push_back(((LeafNode *)pNode)->m_data[i]);
                    }
                }
                for (int i = 0 ; i < keyindex; ++i) {
                    results.push_back(iter->m_data[i]); //push_back rest dataNode of this block
                }
                break;
            }
            case SMALLER_EQUAL:
            {
                LeafNode * iter = m_dataHead;
                for (; iter->selfBlockNum < pNode->selfBlockNum; iter++) {
                    for (int i = 0; i < iter->m_keyNum; ++i)
                    {
                        results.push_back(((LeafNode *)pNode)->m_data[i]);
                    }
                }
                for (int i = 0 ; i <= keyindex; ++i) {
                    results.push_back(iter->m_data[i]); //push_back rest dataNode of this block
                }
                break;
            }
            case GREATER:
            {
                LeafNode * iter = m_dataHead;
                while (iter->selfBlockNum < pNode->selfBlockNum) {
                    iter++;
                }  // skip to the block which contains the key
                for (int i = iter->m_keyNum ; i > keyindex; --i) {
                    results.push_back(((LeafNode *)pNode)->m_data[i]);
                } // push_back rest value of this block
                while (iter != NULL) { // push_back rest blocks' values
                    for (int i = 0; i > iter->m_keyNum; ++i)
                    {
                        results.push_back(iter->m_data[i]);
                    }
                }
                break;
            }
            case GREATER_EQUAL:
            {
                LeafNode * iter = m_dataHead;
                while (iter->selfBlockNum < pNode->selfBlockNum) {
                    iter++;
                }  // skip to the block which contains the key
                for (int i = iter->m_keyNum ; i >= keyindex; --i) {
                    results.push_back(((LeafNode *)pNode)->m_data[i]);
                } // push_back rest value of this block
                while (iter != NULL) { // push_back rest blocks' values
                    for (int i = 0; i > iter->m_keyNum; ++i)
                    {
                        results.push_back(iter->m_data[i]);
                    }
                }
                break;
            }
            case NOT_EQUAL:
            {
                LeafNode * iter = m_dataHead;
                while (iter != NULL) //traverse every block
                {
                    for (int i = 0; i < iter->m_keyNum; )
                    { //traverse in the block
                        if (iter == pNode && i == keyindex)
                        {
                            break;
                        }
                        results.push_back(iter->m_data[i]);
                    }
                }
            }
            default:
                break;
        }
    }
    else // key is not found
    {
        if (pNode->m_nodeType != LEAF) // find in its children
        {
            return findRangeNodeData(((InnerNode *)pNode)->m_children[childIndex], key, condType);
        }
        else
        {
            switch (condType) {
                case SMALLER:
                case SMALLER_EQUAL:
                {
                    LeafNode * iter = m_dataHead;
                    for (; iter->selfBlockNum < pNode->selfBlockNum; iter++) {
                        for (int i = 0; i < iter->m_keyNum; ++i)
                        {
                            results.push_back(((LeafNode *)pNode)->m_data[i]);
                        }
                    }
                    for (int i = 0 ; i < keyindex; ++i) {
                        results.push_back(iter->m_data[i]); //push_back rest dataNode of this block
                    }
                    break;
                }
                case GREATER:
                case GREATER_EQUAL:
                {
                    LeafNode * iter = m_dataHead;
                    while (iter->selfBlockNum < pNode->selfBlockNum) {
                        iter++;
                    }  // skip to the block which contains the key
                    for (int i = iter->m_keyNum ; i > keyindex; --i) {
                        results.push_back(((LeafNode *)pNode)->m_data[i]);
                    } // push_back rest value of this block
                    while (iter != NULL) { // push_back rest blocks' values
                        for (int i = 0; i > iter->m_keyNum; ++i)
                        {
                            results.push_back(iter->m_data[i]);
                        }
                    }
                    break;
                }
                case NOT_EQUAL:
                {
                    LeafNode * iter = m_dataHead;
                    while (iter != NULL) //traverse every block
                    {
                        for (int i = 0; i < iter->m_keyNum; )
                        { //traverse in the block
                            if (iter == pNode && i == keyindex)
                            {
                                break;
                            }
                            results.push_back(iter->m_data[i]);
                        }
                    }
                }
                default:
                    break;
            }
        }
    }
    return results;
}
bool BPlusTree::deleteValue(KeyType key)
{
    if (!search(key))
    {
        return false;
    }
    if (1 == m_root->m_keyNum)
    {
        if (LEAF == m_root->m_nodeType)
        {
            clear();
            return true;
        }
        else
        {
            Node * childNode1 = ((InnerNode *)m_root)->m_children[0];
            Node * childNode2 = ((InnerNode *)m_root)->m_children[1];
            if (childNode1->m_keyNum == childNode1->minKeyValueNum
                && childNode2->minKeyValueNum == childNode2->m_keyNum)
            {
                childNode1->mergeChild(m_root, childNode2, 0);
                delete m_root;
                m_root = childNode1;
            }
        }
    }
    recursive_remove(m_root, key);
    return true;
}

void BPlusTree::recursive_remove(Node * parentNode, KeyType key)
{
    int keyIndex = parentNode->getKeyIndex(key);
    int childIndex = parentNode->getChildIndex(key, keyIndex);
    if (LEAF == parentNode->m_nodeType) // find target node
    {
        if ((key == m_maxKey) && (keyIndex > 0))
        {
            m_maxKey = parentNode->m_keyValues[keyIndex - 1];
        }
        parentNode->removeKey(keyIndex, childIndex); // delete directly
        if (0 == childIndex && LEAF != m_root->m_nodeType && parentNode != m_dataHead)
        {
            changeKey(m_root, key, parentNode->m_keyValues[0]);
        }
    }
    else
    {
        Node *pChildNode = ((InnerNode *)parentNode)->m_children[childIndex];
        if (pChildNode->m_keyNum == pChildNode->minKeyValueNum) // when its keynumber is going to be less than minimum key number
        {
            Node *pLeft = childIndex > 0 ? ((InnerNode *)parentNode)->m_children[childIndex - 1] : NULL;
            Node *pRight = childIndex < parentNode->m_keyNum ? ((InnerNode *)parentNode)->m_children[childIndex + 1] : NULL;
            if (pLeft && pLeft->m_keyNum >  pLeft->minKeyValueNum)
            {
                pChildNode->borrowFrom(pLeft, parentNode, childIndex - 1, LEFT);
            }
            else if (pRight && pRight->m_keyNum > pRight->minKeyValueNum)
            {
                pChildNode->borrowFrom(pRight, parentNode, childIndex + 1, RIGHT);
            }
            else if (pLeft)
            {
                pChildNode->mergeChild(parentNode, pLeft, childIndex - 1);
                pChildNode = pLeft;
            }
            else if (pRight)
            {
                pChildNode->mergeChild(parentNode, pRight, childIndex);
            }
        }
        recursive_remove(pChildNode, key);
    }
}

void BPlusTree::changeKey(Node *pNode, KeyType oldKey, KeyType newKey)
{
    if (NULL != pNode && LEAF != pNode->m_nodeType)
    {
        int keyIndex = pNode->getKeyIndex(oldKey);
        if (keyIndex < pNode->m_keyNum && oldKey == pNode->m_keyValues[keyIndex]) // if find the key
        {
            pNode->m_keyValues[keyIndex] = newKey;
        }
        else // search its children
        {
            changeKey(((InnerNode *)pNode)->m_children[keyIndex], oldKey, newKey);
        }
    }
}

bool BPlusTree::updateValue(KeyType oldKey, KeyType newKey)
{
    if (search(newKey))
    {
        return false;
    }
    else
    {
        DataType dataValue;
        remove(oldKey, dataValue);
        if (false == dataValue.isValid)
        {
            return false;
        }
        else
        {
            return insertValue(oldKey, dataValue);
        }
    }
}

void BPlusTree::remove(KeyType key, DataType & dataValue)
{
    if (!search(key))
    {
        dataValue.isValid = false;
    }
    if (1 == m_root->m_keyNum)
    {
        if (LEAF == m_root->m_nodeType)
        {
            dataValue = ((LeafNode *)m_root)->m_data[0];
            clear();
            return ;
        }
        else
        {
            Node *pChild1 = ((InnerNode *)m_root)->m_children[0];
            Node *pChild2 = ((InnerNode *)m_root)->m_children[1];
            if (pChild1->minKeyValueNum == pChild1->m_keyNum && pChild2->minKeyValueNum == pChild2->m_keyNum)
            {
                pChild1->mergeChild(m_root, pChild2, 0);
                delete m_root;
                m_root = pChild1;
            }
        }
        recursive_remove(m_root, key, dataValue);
    }
}

void BPlusTree::recursive_remove(Node * parentNode, KeyType key, DataType & dataValue)
{
    int keyIndex = parentNode->getKeyIndex(key);
    int childIndex = parentNode->getChildIndex(key, keyIndex);
    if (LEAF == parentNode->m_nodeType)
    {
        if (key == m_maxKey && keyIndex > 0)
        {
            m_maxKey = parentNode->m_keyValues[keyIndex - 1];
        }
        dataValue = ((LeafNode *)parentNode)->m_data[keyIndex];
        parentNode->removeKey(keyIndex, childIndex);
        if (0 == childIndex && m_root->m_nodeType != LEAF && m_dataHead != parentNode)
        {
            changeKey(m_root, key, parentNode->m_keyValues[0]);
        }
    }
    else
    {
        Node *pChildNode = ((InnerNode *)parentNode)->m_children[childIndex];
        if (pChildNode->m_keyNum == pChildNode->minKeyValueNum) // when its keynumber is going to be less than minimum key number
        {
            Node *pLeft = childIndex > 0 ? ((InnerNode *)parentNode)->m_children[childIndex - 1] : NULL;
            Node *pRight = childIndex < parentNode->m_keyNum ? ((InnerNode *)parentNode)->m_children[childIndex + 1] : NULL;
            if (pLeft && pLeft->m_keyNum > pLeft->minKeyValueNum)
            {
                pChildNode->borrowFrom(pLeft, parentNode, childIndex - 1, LEFT);
            }
            else if (pRight && pRight->m_keyNum > pRight->minKeyValueNum)
            {
                pChildNode->borrowFrom(pRight, parentNode, childIndex + 1, RIGHT);
            }
            else if (pLeft)
            {
                pChildNode->mergeChild(parentNode, pLeft, childIndex - 1);
                pChildNode = pLeft;
            }
            else if (pRight)
            {
                pChildNode->mergeChild(parentNode, pRight, childIndex);
            }
        }
        recursive_remove(pChildNode, key, dataValue);
    }
}

void BPlusTree::printData() const
{
    LeafNode* itr = m_dataHead;
    while(itr!=NULL)
    {
        for (int i=0; i < itr->m_keyNum; ++i)
        {
            cout<< itr->m_data[i].recordValue <<" ";
        }
        cout<<endl;
        itr = itr->m_rightSibling;
    }
}

IndexManager::IndexManager(BufferManager &bm, CatalogManager &cm, RecordManager &rm): bm(bm), cm(cm), rm(rm)
{
    indexSet.clear();
}

IndexManager::~IndexManager()
{
}

// recursively traverse the tree and transfer the blocks
void IndexManager::traverse(Node * node, string fileName)
{
    if (node == NULL) // aimed for situation where there is no tree
    {
        return;
    }
    for (int i = 0; i < ((InnerNode *)node)->m_children.size(); ++i)
    {
        if (((InnerNode *)node)->m_children[i]->m_nodeType != LEAF)
        {
            traverse(((InnerNode *)node)->m_children[i], fileName);
        }
        bm.writeData(fileName,
                      node->selfBlockNum * sizeof(*node),
                      (char*)((InnerNode *)node)->m_children[i],
                      sizeof(*(((InnerNode *)node)->m_children[i])),
                      1);
        ((InnerNode *)node)->m_children[i]->parentBlockNum = node->selfBlockNum; //set the block number to parent's block number
        node->childrenBLockNums.push_back(((InnerNode *)node)->m_children[i]->selfBlockNum);
    }
    // write self 
    bm.writeData(fileName,
                  node->selfBlockNum * sizeof(*node),
                  (char*)node,
                  sizeof(*node),
                  1);
}

// push all record values which are correspoding to attribute into the vector attributeValues
vector<nodeData> IndexManager::findAttributeValues(string attribute, Table &table, string filename)
{
    int attributeNum = cm.getAttriNum(table, attribute);
    int size = 0, i = 0;
    for (table.AttriIt = table.attributes.begin(); i < attributeNum; table.AttriIt++, i++)
    {
        size += sizeof(*table.AttriIt);
    }
    vector<nodeData> attributeValues;
//    for (int i = 0; i < table.attriNum; i++)
    for (int i = 0; i < table.recordNum; i++)
    {
        nodeData newNodeData;
        // table.eachRecordLength equals to the length of all attributes
        // thus i * length represents ith row, 1 * length represents length of all attributes
        long fileAddr = (table.eachRecordLength + 8) * i + size;
        if(fileAddr % BLOCKSIZE + table.eachRecordLength + 8 > BLOCKSIZE) // prevent reading cross-block
            fileAddr = BLOCKSIZE * (fileAddr / BLOCKSIZE + 1);
        newNodeData.recordValue = bm.readData(filename, fileAddr);
        newNodeData.blockNum = table.blockNum + fileAddr / BLOCKSIZE;
        newNodeData.blockOffset = fileAddr % BLOCKSIZE;
        newNodeData.isValid = true;
        newNodeData.indexName = table.name + "-" + attribute; // indexName is named after this convention
        newNodeData.tableName = table.name;
        attributeValues.push_back(newNodeData);
    }
    return attributeValues;
}

//---------------Public API------------------
// create index indexName on table(attributeName)
// maxLength is the maximum length of this attibute
bool IndexManager::createIndex(string indexName, Table &table, string attributeName, int maxLength)
{
    string filename = indexName + ".idx";
    fstream fout;
    fout.open(filename, ios::app);
//    if (indexSet.find(indexName) == indexSet.end()) // do not find the index, create a new one
    if (cm.findIndexTable(indexName) == -1 && indexSet.find(indexName) == indexSet.end()) //cannot find the index
    {
        // maxLength is equal to the length of this attribute which is set when creating table
        BPlusTree * newtree = new BPlusTree(maxLength);
        btreeSet.insert(newtree);
        vector<nodeData> attributeValues = findAttributeValues(attributeName, table, table.name + ".table"); // find record values corresponding to the attributeName
        if (!attributeValues.empty()) // there are values
        {
            for (int i = 0; i < attributeValues.size(); ++i)
            {
                newtree->insertValue(attributeValues[i].recordValue, attributeValues[i]); // insert record and its pointer into leaves
            }
            traverse((*--btreeSet.end())->m_root, filename); // transfer each block in the tree into the file
        }
        cm.createIndex(indexName, table.name, attributeName);
        indexSet.insert(indexName);
        return true;
    }
    else
    {
        cout << "Index already exists" << endl;
        return false;
    }
}

bool IndexManager::dropIndex(string indexName, string tableName)
{
    if (cm.findIndexTable(indexName) != -1) // index exists
//    if (indexSet.find(indexName) != indexSet.end())
    {
        if (indexSet.find(indexName) != indexSet.end()) indexSet.erase(indexName); // remove index in the indexset
        cm.dropIndex(indexName); // remove relative info in cm
        string f = indexName + ".idx";
        const char * filename = f.c_str();
        remove(filename); // delete index file
        return true;
    }
    else
    {
        return false;
    }
}

// record select operation
nodeData IndexManager::findEqualRecord(Attribute attribute, string attributeValue, Table & table)
{
    nodeData filePointer;
    BPlusTree * btree = new BPlusTree(attribute.length);
    vector<nodeData> attributeValues = findAttributeValues(attribute.name, table, table.name + ".table");
    for (int i = 0; i < attributeValues.size(); ++i)
    {
        btree->insertValue(attributeValues[i].recordValue, attributeValues[i]); // insert record and its pointer into leaves
    }
    filePointer = btree->findNodeData(attributeValue);
    return filePointer; // start pointer of continuous field
}

vector<Row> IndexManager::findRangeRecord(Attribute attri, string attriValue, Table & table, CONDITION_TYPE condType)
{
    vector<Row> results;
    vector<nodeData> filePointers;
    BPlusTree * btree = new BPlusTree(attri.length);
    vector<nodeData> attriValues = findAttributeValues(attri.name, table, table.name + ".table");
    for (int i = 0; i < attriValues.size(); ++i)
    {
        btree->insertValue(attriValues[i].recordValue, attriValues[i]);
    }
    filePointers = btree->findRangeNodeData(btree->m_root, attriValue, condType); // find nodeData correspoding to required conditions
    vector<nodeData>::iterator iter = filePointers.begin();
    for (; iter != filePointers.end(); iter++)
    {
        char * a = bm.readData(table.name + ".table",  (long)((*iter).blockNum * BLOCKSIZE + (*iter).blockOffset)); //read data in file
        Row row;
        for (int i = 0; i < table.eachRecordLength; ++i)
        {
            row.value += a[i];
        }
        results.push_back(row);
    }
    return results;
}

void IndexManager::afterInsert(Attribute & attribute, string attributeValue, Table & table, FILEPTR addr)
{
    BPlusTree * btree = new BPlusTree(attribute.length);
    nodeData recordPtr = transferAddrToNodeData(attribute, table, addr);
    vector<nodeData> attributeValues = findAttributeValues(attribute.name, table, table.name + ".table");
    for (int i = 0; i < attributeValues.size(); ++i)
    {
        btree->insertValue(attributeValues[i].recordValue, attributeValues[i]); // insert record and its pointer into leaves
    }
    if (!btree->search(attributeValue))  // if attributeValue is not stored
    {
        btree->insertValue(attributeValue, recordPtr);
    }
    traverse(btree->m_root, attribute.indexName + ".idx");
}

void IndexManager::afterDelete(Attribute & attribute, string attributeValue, Table & table, FILEPTR addr)
{
    BPlusTree * btree = new BPlusTree(attribute.length);
    nodeData recordPtr = transferAddrToNodeData(attribute, table, addr);
    vector<nodeData> attributeValues = findAttributeValues(attribute.name, table, table.name + ".table");
    for (int i = 0; i < attributeValues.size(); ++i)
    {
        btree->insertValue(attributeValues[i].recordValue, attributeValues[i]); // insert record and its pointer into leaves
    }
    btree->remove(attributeValue, recordPtr);
    traverse(btree->m_root, attribute.indexName + ".idx");
}

void IndexManager::afterUpdate(Attribute & attribute, string attributeValue, Table & table, FILEPTR addr)
{
    BPlusTree * btree = new BPlusTree(attribute.length);
    nodeData recordPtr = transferAddrToNodeData(attribute, table, addr);
    vector<nodeData> attributeValues = findAttributeValues(attribute.name, table, table.name + ".table");
    for (int i = 0; i < attributeValues.size(); ++i)
    {
        btree->insertValue(attributeValues[i].recordValue, attributeValues[i]); // insert record and its pointer into leaves
    }
    Node * oldNode = btree->findNodeFor(attributeValue);
    btree->changeKey(oldNode, recordPtr.recordValue, attributeValue);
}

nodeData IndexManager::transferAddrToNodeData(Attribute &attri, Table &table, FILEPTR addr)
{
    nodeData recordPtr;
    recordPtr.blockNum = addr / BLOCKSIZE;
    recordPtr.blockOffset = addr % BLOCKSIZE;
    recordPtr.indexName = attri.indexName;
    recordPtr.tableName = table.name;
    recordPtr.isValid = true;
    return recordPtr;
}


















