//
//  BufferManager.h
//  MiniSQL-BufferManager
//
//  Created by Myh on 10/31/14.
//  Copyright (c) 2014 Myh. All rights reserved.
//

#ifndef __MiniSQL_BufferManager__BufferManager__
#define __MiniSQL_BufferManager__BufferManager__

#include "PublicClass.h"
using namespace std;

class BufferManager;

class BufferBlock{
    
    friend class BufferManager;
    
    int blockNum;
    bool isWritten;//脏块
    bool isValid;//是否有效
    string fileName;
    int LRUValue;
    bool pin;
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
    bool dropTableInBuffer(string tableName);
    char* readData(string fileName, FILEPTR addr);
    bool writeData(string fileName, FILEPTR addr, const char* dataAddr, int recordSize, int recordNum);

    
};

#endif /* defined(__MiniSQL_BufferManager__BufferManager__) */
