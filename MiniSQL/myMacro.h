//
//  myMacro.h
//  interpreter
//  用于存放minisql中基本操作和错误类型的宏定义，之后可能会继续修改
//  Created by 李了 on 14/11/1.
//  Copyright (c) 2014年 李了. All rights reserved.
//

#ifndef interpreter_myMacro_h
#define interpreter_myMacro_h

#define SELECT		0
#define CREATE		1
#define	DROP		2
#define DELETE		3
#define INSERT		4
#define QUIT		5
#define EXECFILE    6
#define TABLE       7
#define INDEX       8
#define FILE        9
#define USE         10


//定义错误类型
#define EMPTY		11
#define UNKNOWN		12
#define SELERR		13
#define	CREINDERR	14
#define CRETABERR	15
#define DELETEERR	16
#define INSERTERR	17
#define DRPTABERR	18
#define DRPINDERR	19
#define EXEFILERR	20
#define NOPRIKEY    21
#define VOIDPRI		22
#define VOIDUNI		23
#define CHARBOUD    24
#define TABLEERROR  25
#define COLUMNERROR 26
#define INSERTNUMBERERROR		27
#define SELECT_WHERE_CAULSE		28
#define SELECT_NOWHERE_CAULSE	29
#define TABLEEXISTED			30
#define INDEXERROR				31
#define INDEXEROR				32
#define INVALIDNAME             33
#define WHEREERROR              34
#define TYPEERROR               35
#define SAMETABLE               36
#define SAMEINDEX               37
#define INSERTTYPEERROR         38
#define SUCCESS                 40
#endif
