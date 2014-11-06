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
//        if (GREATER == compare(key, currentKey)) // compare currentKey with key
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
    newNode->m_keyNum = MINIMUM_KEY;
    for (int i = 0; i < MINIMUM_KEY; ++i)  // copy the second half of the keys to the new node
    {
        newNode->m_keyValues[i] = m_keyValues[i + MINIMUM_KEY];
    }
    for (int i = 0; i < MINIMUM_CHILD; ++i) // copy the second half of the children to the new node
    {
        newNode->m_children.push_back(m_children[i + MINIMUM_CHILD]);
    }
    m_keyNum = MINIMUM_KEY; // reset the key number of the old node
    ((InnerNode *) parent)->insert(childIndex, childIndex + 1, m_keyValues[MINIMUM_KEY], newNode); // insert new node to the tree
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
    insert(MINIMUM_KEY, MINIMUM_KEY + 1, parentNode->m_keyValues[keyIndex], ((InnerNode*)childNode)->m_children[0]);
    // insert next keys and children
    for (int i = 1; i < childNode->m_keyNum; ++i)
    {
        insert(MINIMUM_KEY + i, MINIMUM_KEY + i + 1, childNode->m_keyValues[i - 1], ((InnerNode *)childNode)->m_children[i]);
    }
    // remove the key and child in parent node
    parentNode->removeKey(keyIndex, keyIndex + 1);
    delete ((InnerNode *)parentNode)->m_children[keyIndex + 1];
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
//    for (i = m_keyNum; i >= 1 && GREATER == compare(m_keyValues[i - 1], key); --i)
    for (i = m_keyNum; i >= 1 && m_keyValues[i - 1] > key; --i)
    {
        m_keyValues[i] = m_keyValues[i - 1];
        m_data[i] = m_data[i - 1];
    }
//    this->m_data[i] = data;
    this->m_data.push_back(data);
    this->m_keyValues[i] = key;
    this->m_keyNum++;
}

void LeafNode::split(Node *parentNode, int childIndex)
{
    LeafNode *newLeafNode = new LeafNode(parentNode->leafDataLength);
    // set attributes of new node and old node
    this->m_keyNum = MINIMUM_LEAF;
    newLeafNode->m_keyNum = MINIMUM_LEAF + 1;
    newLeafNode->m_rightSibling = this->m_rightSibling;
    newLeafNode->m_leftSibling = this;
    this->m_rightSibling = newLeafNode;
    for (int i = 0; i < MINIMUM_LEAF + 1; ++i) // copy key values
    {
        newLeafNode->m_keyValues[i] = m_keyValues[i + MINIMUM_LEAF];
    }
    for (int i = 0; i < MINIMUM_LEAF + 1; ++i) // copy data
    {
        newLeafNode->m_data[i] = m_data[i + MINIMUM_LEAF];
    }
    ((InnerNode *)parentNode)->insert(childIndex, childIndex + 1, m_keyValues[MINIMUM_LEAF], newLeafNode);
}

void LeafNode::mergeChild(Node *parentNode, Node *childNode, int keyIndex) // insert values of right sibling into this node
{
    for (int i = 0 ; i < childNode->m_keyNum; ++i)
    {
        insert(childNode->m_keyValues[i], ((LeafNode *)childNode)->m_data[i]);
    }
    this->m_rightSibling = ((LeafNode *)childNode)->m_rightSibling;
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
        m_dataHead = (LeafNode *)m_root;
        m_maxKey = key;
    }
    if (m_root->m_keyNum >= MAXIMUM_KEY) // root is full
    {
        InnerNode * newNode = new InnerNode(leafDataLength);
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
        if (childNode->m_keyNum >= MAXIMUM_LEAF)
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
            if (childNode1->m_keyNum == MINIMUM_KEY && MINIMUM_KEY == childNode2->m_keyNum)
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
        if (pChildNode->m_keyNum == MINIMUM_KEY) // when its keynumber is going to be less than minimum key number
        {
            Node *pLeft = childIndex > 0 ? ((InnerNode *)parentNode)->m_children[childIndex - 1] : NULL;
            Node *pRight = childIndex < parentNode->m_keyNum ? ((InnerNode *)parentNode)->m_children[childIndex + 1] : NULL;
            if (pLeft && pLeft->m_keyNum > MINIMUM_KEY)
            {
                pChildNode->borrowFrom(pLeft, parentNode, childIndex - 1, LEFT);
            }
            else if (pRight && pRight->m_keyNum > MINIMUM_KEY)
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
        if (INVALID_INDEX == dataValue)
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
        dataValue = INVALID_INDEX;
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
            if (MINIMUM_KEY == pChild1->m_keyNum && MINIMUM_KEY == pChild2->m_keyNum)
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
        if (pChildNode->m_keyNum == MINIMUM_KEY) // when its keynumber is going to be less than minimum key number
        {
            Node *pLeft = childIndex > 0 ? ((InnerNode *)parentNode)->m_children[childIndex - 1] : NULL;
            Node *pRight = childIndex < parentNode->m_keyNum ? ((InnerNode *)parentNode)->m_children[childIndex + 1] : NULL;
            if (pLeft && pLeft->m_keyNum > MINIMUM_KEY)
            {
                pChildNode->borrowFrom(pLeft, parentNode, childIndex - 1, LEFT);
            }
            else if (pRight && pRight->m_keyNum > MINIMUM_KEY)
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




