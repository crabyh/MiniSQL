//
//  BufferManager.h
//  MiniSQL-BufferManager
//
//  Created by Myh on 10/31/14.
//  Copyright (c) 2014 Myh. All rights reserved.
//

#ifndef __MiniSQL_BufferManager__BufferManager__
#define __MiniSQL_BufferManager__BufferManager__
#define MAXBUFFERNUM 40 //BUFFER中的块数
#define BLOCKSIZE 4096//在MAIN.H中定义？

#include <iostream>
#include <string>
#include <fstream>
using namespace std;

class BufferManager;

class BufferBlock{
    
    friend class BufferManager;
    
    int blockNum;
    bool isWritten;
    bool isValid;
    string fileName;
    int LRUValue;
    bool Slock;
    bool Xlock;
    char values[BLOCKSIZE];
    
    BufferBlock(){initialize();}
    bool initialize();

};

class BufferManager{
    
    BufferBlock buffer[MAXBUFFERNUM];
    
    bool readDataToBuffer(string fileName, int bufferNum, int blockNum);
    bool flashBufferToFile(int bufferNum);
    bool flashLRU(int bufferNum);
    int getBufferToReplace();
    int ifInBuffer(string fileName,int blockNum);

public:
    BufferManager();
    ~BufferManager();
    char* readData(string fileName, int addr);
    char* writeData(string fileName, int addr);

    
};

#endif /* defined(__MiniSQL_BufferManager__BufferManager__) */
