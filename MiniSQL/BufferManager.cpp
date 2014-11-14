//
//  BufferManager.cpp
//  MiniSQL-BufferManager
//
//  Created by Myh on 10/31/14.
//  Copyright (c) 2014 Myh. All rights reserved.
//

#include "BufferManager.h"

//初始化Buffer块函数
bool BufferBlock::initialize()
{
    blockNum = -1;
    isWritten = false;
    isValid = false;
    fileName = "";
    LRUValue = 0;
    pin = false;
    pin = false;
    
    for(int bufferNum=0;bufferNum<BLOCKSIZE;bufferNum++)
        values[bufferNum]='\0';
    return true;
}

//初始化BufferManager
BufferManager::BufferManager()
{
    for(int i=0;i<MAXBUFFERNUM;i++)
        buffer[i]=*new BufferBlock;
}

//析构函数，将数据全部存入文件
BufferManager::~BufferManager()
{
    for(int i=0;i<MAXBUFFERNUM;i++)
        flashBufferToFile(i);
};

//找到buffer数组中,需要被替换的buffer并初始化&flashToFile
int BufferManager::getBufferToReplace()
{
    for(int bufferNum = 0; bufferNum < MAXBUFFERNUM; bufferNum++)    //先找空的buffer块
        if(buffer[bufferNum].isValid == false)
        {
            buffer[bufferNum].initialize();
            buffer[bufferNum].isValid = true;
            flashLRU(bufferNum);
            //cout<<bufferNum<<endl;
            return bufferNum;
        }
    //没有空块，使用LRU策略
    int highestLRUValue = buffer[0].LRUValue;
    int bufferNum = 0;/// add:=0  LRU选取的block的数组下标，LRU全相等的时候就替换第一块
    for(int i = 0; i < MAXBUFFERNUM; i++) //找到最大的LRU块
        if(buffer[i].LRUValue > highestLRUValue)
        {
            highestLRUValue = buffer[i].LRUValue;
            bufferNum = i;//最大LRU块的下标
        }
    flashBufferToFile(bufferNum);   //flashBufferToFile
    buffer[bufferNum].initialize();
    buffer[bufferNum].isValid = true;
    flashLRU(bufferNum);
    return bufferNum;
}

//从文件中读取数据存到Buffer中
bool BufferManager::readDataToBuffer(string fileName, int bufferNum, int blockNum)
{
    buffer[bufferNum].fileName = fileName;
    buffer[bufferNum].blockNum = blockNum;
    fstream in;
    in.open(fileName.data(),ios::in|ios::binary);
    if(!in.is_open())
    {
        //cout<<"Can not find the file !"<<endl;
    }
    in.seekp(blockNum*BLOCKSIZE, in.beg);
    in.read(buffer[bufferNum].values, BLOCKSIZE);
    in.close();
    return true;
}

//把Buffer中的缓存写入文件并初始化Buffer块
bool BufferManager::flashBufferToFile(int bufferNum)
{
    if(buffer[bufferNum].isWritten == false) return true;//是否被修改过。true就重新写回文件，false不用管它
    string filename = buffer[bufferNum].fileName ;
    fstream out;
    out.open(filename.data(),ios::out|ios::in|ios::binary);
    if(!out.is_open())
    {
        out.open(filename.data(),ios::app);
        out.close();
        out.open(filename.data(),ios::out|ios::in|ios::binary);
    }
    out.seekp(buffer[bufferNum].blockNum*BLOCKSIZE, out.beg);
    out.write(buffer[bufferNum].values , BLOCKSIZE);
    buffer[bufferNum].initialize();
    out.close();
    return true;
}

//刷新LRU
bool BufferManager::flashLRU(int bufferNum)
{
    for(int i = 0; i < MAXBUFFERNUM; i++)
    {
        if(i == bufferNum)
            buffer[bufferNum].LRUValue = 0; // bufferNum置为0
        else buffer[i].LRUValue++;          // 其他的block LRU值++
    }
    return true;
}

//判断块是不是在Buffer中
int BufferManager::ifInBuffer(string fileName,int blockNum)
{
    for(int i = 0; i < MAXBUFFERNUM; i++)
        if((buffer[i].fileName == fileName) && (buffer[i].blockNum == blockNum))  return i;
    return -1;
}

//供外部调用读取块!!!请勿跨块存取!!!
char* BufferManager::readData(string fileName, FILEPTR addr)
{
    int blockNum = (int) (addr / BLOCKSIZE); //数据存在于file的第 blockNum+1 块内，从blockNum末尾开始读取
    int blockOffset = addr % BLOCKSIZE; //所需数据在块内的偏移量
    int blockNumInBuffer=ifInBuffer(fileName, blockNum);//block在buffer中的位置
    
    if(blockNumInBuffer != -1)//已经存在于buffer中
    {
        flashLRU(blockNumInBuffer);//// 更新所有buffer block的LRU值
    }
    else //需要的内容不在buffer中
    {
        blockNumInBuffer=getBufferToReplace();
        readDataToBuffer(fileName, blockNumInBuffer, blockNum);//将文件块读到buffer[blockNumInBuffer]
    }
    return buffer[blockNumInBuffer].values+blockOffset/sizeof(char);//返回数据地址
}

//供外部调用写入块
bool BufferManager::writeData(string fileName, FILEPTR addr, const char* dataAddr, int recordSize, int recordNum)
{
    int blockNum = (int) (addr / BLOCKSIZE); //数据存在于file的第 blockNum+1 块内，从blockNum末尾开始读取
    int blockOffset = addr % BLOCKSIZE; //所需数据在块内的偏移量
    
    while(recordNum>0)//还有数据没写入Buffer,一块一块写到buffer里
    {
        int recordToWrite=min(recordNum,(BLOCKSIZE-blockOffset)/recordSize);//当前块能插多少插多少
        recordNum -=recordToWrite;
        if(recordNum > 0) blockOffset = 0;
        int size = recordToWrite * recordSize;
        
        int blockNumInBuffer=ifInBuffer(fileName, blockNum);//block在buffer中的位置
        
        if(blockNumInBuffer != -1)//已经存在于buffer中
        {
            flashLRU(blockNumInBuffer);//// 更新所有buffer block的LRU值
        }
        else //需要的内容不在buffer中
        {
            blockNumInBuffer=getBufferToReplace();
            readDataToBuffer(fileName, blockNumInBuffer, blockNum);//将文件块读到buffer[blockNumInBuffer]
        }
        buffer[blockNumInBuffer].isWritten = true;
        char* addrInBuffer = buffer[blockNumInBuffer].values+blockOffset/sizeof(char);//Buffer中数据地址
        memcpy(addrInBuffer, dataAddr, size);
        addr = (addr/BLOCKSIZE+1)*BLOCKSIZE;
        dataAddr = dataAddr+size;
        blockNum++;
    }
    return true;
}

//int main(int argc, const char * argv[]) {
//    BufferManager m;
//    string s="HelloWorld";
//    m.writeData("abc",0, &s[0], sizeof(s), 1);
//    m.writeData("abc",2*BLOCKSIZE, &s[0], sizeof(s), 1);
//    m.writeData("abc",1*BLOCKSIZE, &s[0], sizeof(s), 1);
//    m.writeData("abc",0*BLOCKSIZE, &s[0], sizeof(s), 1);
//    m.writeData("abc",3*BLOCKSIZE, &s[0], sizeof(s), 1);
//    m.writeData("abc",4*BLOCKSIZE, &s[0], sizeof(s), 1);
//    m.writeData("abc",5*BLOCKSIZE, &s[0], sizeof(s), 1);
//    printf( "%s" , m.readData("abc", 0x00000000));
//}






