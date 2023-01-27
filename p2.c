/*
Brett Gallagher
Carroll
CS480
October 12th, 2022
*/

#include "p2.h"

extern int errno;
#define CHK(x) \
    do{if ((x) == -1) \
        {fprintf(stderr,"In file %s, on line %d: \n",__FILE__,__LINE__); \
               fprintf(stderr,"errno = %d\n",errno);                 \
               exit(1); \
               }       \
   }while(0)

//int variables in order of appearance
int i,j,k;				//possible loop variables
int numPrompt = 1;		//used to display the number of prompts, only goes up when some text is entered
int stop;				//when stop = parse = -1, program stops, else loop continues issuing prompts
int currentPlace;		//used to track what address of the full line storage to put in allnewargv 2d array
int linePlace;			//used to enter a line character by character into arrays
int c;					//return number for getword, is the number of characters in a "word"
int argNum;				//track which argument you are on for newargv
int lineChoice;			//track history by accessing elements of the arrays. defaults to prompt number, changes when !num
int isSpecial;			//used when an action uses a getword and receives an (un)expected newline character o it doesnt print the prompt twice
int kidpid;				//kid process id
int file;				//file used in redirection for < and > and other metacharacters
int flags = 0; 				//used as bool to block exec in certain situations

//arrays in order of appearance
char s[STORAGE];					//storage for 1 word at a time
char lineStorage[STORAGE];			//stores a whole line, get reset
char newargvLineStorage[STORAGE];	//stores everything typed, not reset so newargv can point to the memory still	
char allLineStorage[10][STORAGE];	//stores each line in its own array for history access
char *allnewargv[10][STORAGE];		//stores newargv in its own array for history access

//arrays for storing flags and values for all metacharacters
char cdFlagArg[10][STORAGE];	//cd
char cdFlagHistory[10][1];
char lessthanFlagArg[10][STORAGE];	//<
char lessthanFlagHistory[10][1];
char greaterthanFlagArg[10][STORAGE];	//>
char greaterthanFlagHistory[10][1];
char greaterthanampersandFlagArg[10][STORAGE];	//>&
char greaterthanampersandFlagHistory[10][1];
char doublegreaterthanFlagArg[10][STORAGE];	//>>
char doublegreaterthanFlagHistory[10][1];
char doublegreaterthanampersandFlagArg[10][STORAGE];	//>>&
char doublegreaterthanampersandFlagHistory[10][1];
char pipeFlagArg[10][STORAGE];	//|
char pipeFlagHistory[10][1];
char ampersandFlagArg[10][STORAGE];	//&
char ampersandFlagHistory[10][1];

//Pipe
int pipeNum;
int pipeFiles[2];
pid_t firstChild;
pid_t secondChild;
pid_t pid;

int main()
{
	setpgid(0,0);

	for (;;){
		printf("%%%d%% ", numPrompt);
		stop = parse();

		//handle end.
		if ((allLineStorage[lineChoice - 1][0] == 'e') && (allLineStorage[lineChoice - 1][1] == 'n') && (allLineStorage[lineChoice - 1][2] == 'd') && (allLineStorage[lineChoice - 1][3] == '.')){
			stop = -1;
		}

		cdFunction();
		
		fflush(stdout);
		fflush(stdin);
		fflush(stderr);
		if (cdFlagHistory[lineChoice - 1][0] == 0 && (stop != -1)){	//dont need to fork if you're just using cd, causes a failed exec
			kidpid = fork();
			if (kidpid == -1){	//fork failed
				perror("Cannot fork\n");
				exit(EXIT_FAILURE);
			}else if (kidpid == 0){	//in child
				if ((lessthanFlagHistory[lineChoice - 1][0] == 1) && (lessthanFlagArg[lineChoice - 1][0] != '\0')){ lessThanFunction();}  											//<
				if (greaterthanFlagHistory[lineChoice - 1][0] == 1){greaterThanFunction();}											//>
				if (greaterthanampersandFlagHistory[lineChoice - 1][0] == 1){greaterThanAmpersandFunction();}						//>&
				if (doublegreaterthanFlagHistory[lineChoice - 1][0] == 1){doubleGreaterThanFunction();}								//>>
				if (doublegreaterthanampersandFlagHistory[lineChoice - 1][0] == 1){doubleGreaterThanAmpersandFunction();}			//>>&
				if (pipeFlagHistory[lineChoice - 1][0] == 1){pipeFunction();}														//|
				//might need to add protection when there are flags so you dont exec the flags
				if ((greaterthanFlagHistory[lineChoice - 1][0] == 0) && (greaterthanampersandFlagHistory[lineChoice - 1][0] == 0) && (pipeFlagHistory[lineChoice - 1][0] == 0)){execvpFunction();}
				dup2(STDIN_FILENO, STDIN_FILENO);
				dup2(STDOUT_FILENO, STDOUT_FILENO);
			}else{
				if (ampersandFlagHistory[lineChoice - 1][0] == 1){		//&
					printf("%s [%ld]\n", allnewargv[lineChoice - 1][0], (long)getpid());
					continue;
				}else{
					wait();
				}
				
			}
		}
		
		if (stop == -1) {break;}
	}
	killpg(getpgrp(), SIGTERM);
	printf("p2 terminated.\n");
	exit(0);
}

int parse(){
	resetForParse();
	for(;;){
		c = getword(s);
		if (c == -1 || c == 0){
			break;
		}
		memmove(lineStorage + linePlace, s, c + 1);
		linePlace = linePlace + c + 1;
		memmove(allLineStorage[numPrompt - 1], lineStorage, linePlace);
		isSpecial = parseSpecialChars();
		if (isSpecial != 0){break;}					//when getword eats a new line in a special character function, break early
		c = parseNormalChars();
	}
	lineChoice = whichLine();
	if (lineStorage[0] != '\0'){ numPrompt++; }		//only increment when it is a non empty line
	return c;
}


void resetForParse(){

	argNum = 0;

	//reset newargv and linestorage
	for (i = 0; i < linePlace; i++){
		lineStorage[i] = '\0';
		s[i] = '\0';
	}
	linePlace = 0;

}

int whichLine(){			//history, chooses which flags and args to use
	if (lineStorage[0] == '!' && (lineStorage[1] >= '1'  && lineStorage[1] <= '9') && (lineStorage[2] == '\0')){
		lineChoice = (lineStorage[1]) - 48;
		return lineChoice;
	}else if (lineStorage[0] == '!' && (lineStorage[1] == '1' && lineStorage[2] == '0')){
		lineChoice = 10;
		return lineChoice;
	}else{
		return numPrompt;
	}
	
}

int parseSpecialChars(){		//parse things not in newargv
	isSpecial = 0;

	checkForCD();							// cd
	checkForLessThan();						// < 
	checkForGreaterThan();					// >
	checkForGreaterThanAmpersand(); 		// >&
	checkForDoubleGreaterThan(); 			// >>
	checkForDoubleGreaterThanAmpersand();	// >>&
	checkForPipe(); 						// |
	checkForAmpersand();		 			// &

	return isSpecial;
}

int parseNormalChars(){		//things in newaragv
	if ((c == 0) || (c == -1)){return c;}
	memmove(newargvLineStorage + currentPlace, s, c + 1);
	allnewargv[lineChoice][argNum] = &newargvLineStorage[currentPlace];
	currentPlace = currentPlace + c + 1;
	argNum++;
}

void checkForCD(){
	if (allLineStorage[lineChoice][0] == 'c' && allLineStorage[lineChoice][1] == 'd')
	{
		cdFlagHistory[lineChoice][0] = 1;
		c = getword(s);
		if (c != 0){
			for (i = 0; i < c; i++){
				cdFlagArg[lineChoice][i] = s[i];		//save next word after cd
			}	
			//add something to protect against someone entering two arguments
		}
		else{
			isSpecial = 1;		//eats the newline in previous get word, isSpecial makes the prompt print once instead of twice
			cdFlagArg[lineChoice][0] = '\0';
		}
	}
}

//<
void checkForLessThan(){
	if (s[0] == '<'  && s[1] == '\0'){
		lessthanFlagHistory[lineChoice][0] = 1;
		c = getword(s);
		if (c == 0){
			fprintf(stderr,"error: need arg after <\n");
			isSpecial = 1;
			exit(EXIT_FAILURE);
		}
		for (i = 0; i < c; i++){
			lessthanFlagArg[lineChoice][i] = s[i];		//save next word after less than
		}
		c = getword(s);
		if (c == 0){
			isSpecial = 1;
		}
	}
}

//>
void checkForGreaterThan(){
	if (s[0] == '>'  && s[1] == '\0'){
		greaterthanFlagHistory[lineChoice][0] = 1;
		c = getword(s);
		if (c == 0){
			printf("error: need arg after >\n");
			isSpecial = 1;
		}
		for (i = 0; i < c; i++){
				greaterthanFlagArg[lineChoice][i] = s[i];		//save next word after greater than	
		}
		c = getword(s);
		if (c == 0){
			isSpecial = 1;
		}
	}
}

//>&
void checkForGreaterThanAmpersand(){
	if (s[0] == '>' && s[1] == '&' && s[2] == '\0'){
		greaterthanampersandFlagHistory[lineChoice][0] = 1;
		c = getword(s);
		if (c == 0){
			printf("error: need arg after >&\n");
			isSpecial = 1;
		}
		for (i = 0; i < c; i++){
				greaterthanampersandFlagArg[lineChoice][i] = s[i];
		}
		c = getword(s);
		if (c == 0){
			isSpecial = 1;
		}
	}
}

//>>
void checkForDoubleGreaterThan(){
	if (s[0] == '>' && s[1] == '>'&& s[2] == '\0'){
		doublegreaterthanFlagHistory[lineChoice][0] = 1;
		c = getword(s);
		if (c == 0){
			printf("error: need arg after >>\n");
			isSpecial = 1;
		}
		for (i = 0; i < c; i++){
				doublegreaterthanFlagArg[lineChoice][i] = s[i];		//save next word after double greater than
		}
		c = getword(s);
		if (c == 0){
			isSpecial = 1;
		}
	}
}

//>>&
void checkForDoubleGreaterThanAmpersand(){
	if (s[0] == '>' && s[1] == '>' && s[2] == '&' && s[3] == '\0'){
		doublegreaterthanampersandFlagHistory[lineChoice][0] = 1;
		c = getword(s);
		if (c == 0){
			fprintf(stderr, "error: need arg after >>&\n");
			isSpecial = 1;
		}
		for (i = 0; i < c; i++){
			doublegreaterthanampersandFlagArg[lineChoice][i] = s[i];		//save next word after double greater than amp
		}
		c = getword(s);
		if (c == 0){
			isSpecial = 1;
		}
	}
}

//|
void checkForPipe(){
	if (s[0] == '|' && s[1] == '\0'){
		pipeFlagHistory[lineChoice][0] = 1;
		allnewargv[lineChoice - 1][argNum] = '\0';
		argNum++;
		pipeNum = argNum;
		c = getword(s);
		if (c == 0){
			fprintf(stderr,"need argument after |");
			exit(EXIT_FAILURE);
		}
	}
}

//&
void checkForAmpersand(){
	//what do we do when & is randomly in a line? example: "there is a random & in this line", do we put & in newargv or not? current implementation is to have everything but & in newargv
	if (s[0] == '&' && s[1] == '\0'){
		c = parseNormalChars();
		c = getword(s);
		if (c == 0){		//ampersand is last in line
			ampersandFlagHistory[lineChoice][0] = 1;
			allnewargv[lineChoice][argNum - 1] = '\0';
			isSpecial = 1;
		}
	}
}

void cdFunction(){
	if (cdFlagHistory[lineChoice - 1][0] == 1){
		if (cdFlagArg[lineChoice - 1][0] == '\0'){
			chdir(HOME);
		}else{
			chdir(cdFlagArg[lineChoice - 1]);
		}
	}
}

void execvpFunction(){
	if (execvp(allnewargv[lineChoice - 1][0], allnewargv[lineChoice - 1]) == -1){
		perror("failed execvp\n");
		exit(EXIT_FAILURE);
	}
}

void lessThanFunction(){
	fflush(stdin);
	//check if file exists so you know there is something to read from
	if ((file = open(lessthanFlagArg[lineChoice - 1], O_RDONLY, 0777)) < 0){
		fprintf(stderr, "%s does not exist\n", lessthanFlagArg[lineChoice - 1]);
		exit(EXIT_FAILURE);
	}else{		//proceed with redirection
		file = open(lessthanFlagArg[lineChoice - 1], O_RDONLY, 0777);
		if (dup2(file, STDIN_FILENO) == -1){
			perror("error with dup2 in lessThanFunction");
			exit(EXIT_FAILURE);
		}
		close(file);
	}
}

void greaterThanFunction(){
	fflush(stdout);
	//check if file already exists so you dont overwrite it
	if ((file = open(greaterthanFlagArg[lineChoice - 1], O_WRONLY, 0777)) > 0){
		fprintf(stderr, "%s already exists\n", greaterthanFlagArg[lineChoice - 1]);
		//close(file);
		exit(EXIT_FAILURE);
	}else{		//proceed with redirection
		file = open(greaterthanFlagArg[lineChoice - 1], O_WRONLY | O_CREAT, 0777);
		if (dup2(file, STDOUT_FILENO) == -1){
			perror("error with dup2 in greaterThanFunction");
			exit(EXIT_FAILURE);
		}
		execvpFunction();
		close(file);
	}
}

void greaterThanAmpersandFunction(){
	fflush(stdout);
	fflush(stderr);
	//check if file already exists so you dont overwrite it
	if ((file = open(greaterthanampersandFlagArg[lineChoice - 1], O_WRONLY, 0777)) > 0){
		fprintf(stderr, "%s already exists\n", greaterthanampersandFlagArg[lineChoice - 1]);
		close(file);
		exit(EXIT_FAILURE);
	}else{		//proceed with redirection
		file = open(greaterthanampersandFlagArg[lineChoice - 1], O_WRONLY | O_CREAT, 0777);
		if(dup2(file, STDOUT_FILENO) == -1){
			perror("error with dup2 in greaterThanAmpersandFunction");
			exit(EXIT_FAILURE);
		}
		if(dup2(file, STDERR_FILENO) == -1){
			perror("error with dup2 in greaterThanAmpersandFunction");
			exit(EXIT_FAILURE);
		}
		close(file);
	}
}

void doubleGreaterThanFunction(){
	fflush(stdout);
	file = open(doublegreaterthanFlagArg[lineChoice - 1], O_WRONLY | O_CREAT | O_APPEND, 0777);
	if (dup2(file, STDOUT_FILENO) == -1){
			perror("error with dup2 in doublegreaterThanFunction");
			exit(EXIT_FAILURE);
	}
	close(file);
}

void doubleGreaterThanAmpersandFunction(){
	fflush(stdout);
	fflush(stderr);
	file = open(doublegreaterthanampersandFlagArg[lineChoice - 1], O_WRONLY | O_CREAT | O_APPEND, 0777);
	if(dup2(file, STDOUT_FILENO) == -1){
			perror("error with dup2 in doublegreaterThanAmpersandFunction");
			exit(EXIT_FAILURE);
		}
	if(dup2(file, STDERR_FILENO) == -1){
			perror("error with dup2 in doublegreaterThanAmpersandFunction");
			exit(EXIT_FAILURE);
	}
	close(file);
}

void secondPipeExecvpFunction(){
	if (execvp(allnewargv[lineChoice - 1][pipeNum], allnewargv[lineChoice - 1] + pipeNum) == -1){
		perror("failed second exec\n");
		exit(EXIT_FAILURE);
	}
}

void pipeFunction(){
	fflush(stdout);

	//child1
	CHK(firstChild = fork());

	if (firstChild == 0){
		//create pipe
		if (pipe(pipeFiles) < 0){
            exit(EXIT_FAILURE); // Pipe failed
        }
		//grandchild
		CHK(secondChild = fork());
		if (secondChild == 0){
			CHK(dup2(pipeFiles[1], STDOUT_FILENO));
			CHK(close(pipeFiles[0]));
            CHK(close(pipeFiles[1]));
			execvpFunction();
		}else{
			//child
			CHK(dup2(pipeFiles[0], STDIN_FILENO));
            CHK(close(pipeFiles[0]));
            CHK(close(pipeFiles[1]));

            //Child executing commands
            secondPipeExecvpFunction();
		}
	}else{
		for(;;){
        CHK(pid = wait(NULL));
        if (pid == firstChild){
            break;
        }
    	}
	}
}

void myHandler(int signum){
	if (signum == 15){
		printf("p2 terminated.");
		exit(-1);
	}else{
		return;
	}
}