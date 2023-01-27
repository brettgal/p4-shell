#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include "getword.h"

#define MAXITEMS 100
#define HOME "/home/cs/carroll/cssc0021"

int parse();
void resetForParse();
int parseNormalChars();
int parseSpecialChars();
int whichLine();

void checkForCD();				            //cd
void checkForLessThan();		            //<
void checkForGreaterThan();		            //>
void checkForGreaterThanAmpersand();        //>&
void checkForDoubleGreaterThan();           //>>
void checkForDoubleGreaterThanAmpersand();  //>>&
void checkForPipe(); 			            //|
void checkForAmpersand();		            //&

void cdFunction();                          //cd
void execvpFunction();                      
void lessThanFunction();                    //<
void greaterThanFunction();                 //>
void greaterThanAmpersandFunction();        //>&
void doubleGreaterThanFunction();           //>>
void doubleGreaterThanAmpersandFunction();  //>>&
void pipeFunction();                        //|

void secondPipeExecvpFunction();

void myhandler(int signum);