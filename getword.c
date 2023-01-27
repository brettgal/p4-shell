
/*Brett Gallagher
 * Carroll
 * CS480
 * September 12th, 2022
 */

/* p1 documentation
 * A lexical analyzer that will output an integer showing number of character in a word, and the word itself based on specifications
 * The characters " ", a newline, EOF, "<", ">", ">&", ">>", ">>&", "|", and "&" can be used to end words, provided they are not backslashed before typed
 * Leading spaces are to be not counted. The program is designed to end when the user types "end."
 */

#include <stdbool.h>
#include "getword.h"

int getword(char *w)
{
	int i = 0;
	int numLetters = 0;		//keep count of letters in current word to be outputted
	int curChar;			//character being read
	int lastChar;			//previous character, necessary for "\" and other uses
	int nextChar;			//peeks ahead to next character to implement greedy search
	int nextNextChar;		//peeks ahead two characters to implement greedy search
	bool isDoubleArrow = false;		//check if ">>" to implement greedy search for >>&

	while ((curChar = getchar()) != EOF)
	{	
		//protect array from overflow by returning it before the maximum
		if(numLetters == (255 - 1)){
			ungetc(curChar,stdin);
			return numLetters;
		}

		/******************************** if newline is pressed without escaping *********************************/

		if (curChar == '\n')
		{
			// returns the number of characters and the string when a new line is pressed before any characters are stored.
			if (numLetters == 0 && lastChar != '\\')
			{
				w[numLetters] = '\0';
				return 0;
			}
			// returns the number of characters and the string when a new line terminates a word. ungetc counts the newline again so it is still displayed.
			if (numLetters != 0)
			{
				w[numLetters] = '\0';
				ungetc(curChar, stdin);
				return numLetters;
			}
		}

		/*************************************************** when backslash is used *************************************************************/

		// moves backslash to lastChar and allows backslash to not be added to the word when it is being used to escape other characters. allows "\\" to be read as \ if user wants a \ in a word
		if (curChar == '\\' && lastChar != '\\')
		{
			lastChar = curChar;
			continue;
		}

		if (lastChar == '\\')
		{

			// allows a newline to be in a word if it is escaped by a backslash first.
			if (curChar == '\n')
			{
				lastChar = curChar;
				ungetc(curChar, stdin);
				continue;
			}

			// keeps space in the word if it is escaped by a backslash first.
			if (curChar == ' ')
			{
				w[numLetters] = curChar;
				numLetters++;
				lastChar = curChar;
				continue;
			}
			if (curChar == '\\'){
				w[numLetters] = '\\';
				numLetters++;
				lastChar = 'p';
				continue;
			}
		}

		/*************************************************** metacharcters *************************************************************/
		// metacharacters are <, >, >&, >>, >>&, |, &

	//this implements the greedy search of metacharacters following ">""
		if (curChar == '>' && lastChar != '\\')
		{
			//split > into its own word by returning current array and leaving > to be picked up next pass
			if (numLetters != 0)
			{
				w[numLetters] = '\0';
				ungetc(curChar, stdin);
				return numLetters;
			}
			else if ((nextChar = getchar()) == '>')
			{
				//ungetc(curChar, stdin);
				isDoubleArrow = true;
			}
			else if(nextChar == '&'){
				w[numLetters] = curChar;
				numLetters++;
				w[numLetters] = nextChar;
				numLetters++;
				w[numLetters] = '\0';
				return numLetters;
			}	
			else{
				ungetc(nextChar, stdin);
				isDoubleArrow = false;
				w[numLetters] = curChar;
				lastChar = curChar;
				numLetters++;
				w[numLetters] = '\0';
				return numLetters;
				}
			if(isDoubleArrow == true){
				//check if more than just >>
				if ((nextNextChar = getchar()) == '&')
				{
					w[numLetters] = curChar;
					numLetters++;
					w[numLetters] = nextChar;
					numLetters++;
					w[numLetters] = nextNextChar;
					numLetters++;
					w[numLetters] = '\0';
					return numLetters; // should always 3, >>&
				}
				else
				{
					ungetc(nextNextChar, stdin);
					w[numLetters] = curChar;
					numLetters++;
					w[numLetters] = nextChar;
					numLetters++;
					w[numLetters] = '\0';
					return numLetters;
				}
				}
		}

		if (curChar == '&' && lastChar != '\\')
		{
			//put & in array
			if (numLetters == 0 && lastChar != '\\')
			{
				w[numLetters] = curChar;
				lastChar = curChar;
				numLetters++;
				w[numLetters] = '\0';
				return numLetters;
			}
			//split & into its own word by returning current array and leaving & to be picked up next pass
			if (numLetters != 0)
			{
				w[numLetters] = '\0';
				ungetc(curChar, stdin);
				return numLetters;
			}
		}

		if (curChar == '<' && lastChar != '\\')
		{
			//put < in array
			if (numLetters == 0 && lastChar != '\\')
			{
				w[numLetters] = curChar;
				lastChar = curChar;
				numLetters++;
				w[numLetters] = '\0';
				return numLetters;
			}
			//split < into its own word by returning current array and leaving < to be picked up next pass
			if (numLetters != 0)
			{
				w[numLetters] = '\0';
				ungetc(curChar, stdin);
				return numLetters;
			}
		}

		if (curChar == '|' && lastChar != '\\')
		{
			//put | in array
			if (numLetters == 0 && lastChar != '\\')
			{
				w[numLetters] = curChar;
				lastChar = curChar;
				numLetters++;
				w[numLetters] = '\0';
				return numLetters;
			}
			//split | into its own word by returning current array and leaving | to be picked up next pass
			if (numLetters != 0)
			{
				w[numLetters] = '\0';
				ungetc(curChar, stdin);
				return numLetters;
			}
		}

		// skips over the leading space if there is no current string.
		if (curChar == ' ' && numLetters == 0)
		{
			lastChar = curChar;
			continue;
		}

		// returns the number of characters and the string when space is pressed to end a word.
		if (curChar == ' ' && numLetters > 0)
		{
			w[numLetters] = '\0';
			return numLetters;
		}

		// counts the characters and adds to the string when a non special character is typed.
		if (curChar != ' ' || curChar != '\n' || '\\')
		{
			w[numLetters] = curChar;
			numLetters++;
			lastChar = curChar;
		}
	}

	// accounts for EOF terminating a word instead of a space or newline.
	if ((curChar = getchar()) == EOF && numLetters != 0)
	{
		if (w[0] == 'e' && w[1] == 'n' && w[2] == 'd' && w[3] == '.')
		{
			w[4] = '\0';
			return 4;
		}
		w[numLetters] = '\0';
		return numLetters;
	}

	// returns negative 1 if the file ends with no current characters stored.
	if ((curChar = getchar()) == '\0' && numLetters == 0)
	{
		w[numLetters] = '\0';
		numLetters = 0;
		return numLetters;
	}
}