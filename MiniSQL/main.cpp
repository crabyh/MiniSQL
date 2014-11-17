//
//  main.cpp
//  1114
//
//  Created by 李了 on 11/14/14.
//  Copyright (c) 2014 李了. All rights reserved.
//

#include <iostream>
#include "PublicClass.h"
#include "Interpreter.h"
#include "myMacro.h"
#include <ctime>
Interpreter inter;

int main(int argc, const char * argv[]) {

    inter.init();
    while(!inter.quit())
    {
        inter.init();
        inter.getInput();
        inter.converseCase();
        if(inter.judgeCommandType(inter.originalInput))
        {
            if(inter.parseCommand(inter.originalInput))
            {
                time_t t1, t2;
                t1 = clock();
                inter.executeCommand();
                t2 = clock();
                cout << (t2-t1) << endl;
            }
        }
        else
        inter.outputHelp(UNKNOWN);
//        inter.test();
    }
        
    return 0;
}
