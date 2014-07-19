/*
 * Name: Aaron Colin Foote
 * User: acf502	4258770
 * Date: 1st November 2013
 * File Description: Assignment 4 cpp file for CSCI203
 *
 * What I did with newline characters:
 * Newline characters have essentially been converted into a regular space character, where they appear at the end of a line after some text
 * However, if the line has no content and solely exists of a newline character, the line is not appended to my total text string.
 * The line is still treated as existing in the line count
 *
 * E.g: 1)	The Project Gutenberg EBook of Pride and Prejudice, by Jane Austen
	2)
	3)	This eBook is for the use of anyone anywhere at no cost and with
**/

#include "A4.h"

//GLOBAL VARIABLES
vector<patternInfo> patterns;	//holds the patterns loaded from the file and all their relative data
const int ASCII_SIZE = 256;	//used to setup size of tables, based on the amount of possible ASCII characters in text

//DATA PREPARATION
bool getTextandPatterns(const char *arg1, const char *arg2, string &text, vector<int> &lineNumbers);

//HORSPOOL
void shiftTable_Horspool(const string pattern, int *skiptable);
void horspool(bool outputAppendMode, const string text, vector<int> lineNumbers);
bool testSurroundingCharactersHorspool(int i, int patternLength, const string text);

//BOYER MOORE
void deltaOneSetup(int *deltaOne, const string pattern, const int patternLength);
void deltaTwoSetup(int *deltaTwo, const string pattern, const int patternLength);
bool isPrefix(const string pattern, const int patternLength, const int p);
int suffixLength(const string pattern, const int patternLength, const int p);
void boyerMoore(bool outputAppendMode, const string text, vector<int> lineNumbers);
int max(int x, int y);

//KNUTH-MORRIS PRATT
void shiftTable_kmpTable(const string pattern, int *skiptable);
void kmp(bool outputAppendMode, const string text, vector<int> lineNumbers);

//Boyer Moore and KMP use this to test surrounding characters of the pattern
bool testSurroundingCharacters_MooreKMP(int i, int patternLength, const string text);

/*
Function name: main
Parameters: int args, char *argv[]
Returns: int
Purpose: Ensure that program starts and sets up correctly, govern control of algorithm initiation
Algorithm outline:
	Test argument count (given from command-line)
		End program if incorrect argument count is given
	Manually test to identify whether output file exists already, used to determine what is appended on program run
	If pattern and text file loads correctly
		Begin algorithm runs
	Else
		End program
	Return success
*/
int main(int args, char *argv[])
{
	string text = "";		//holds the entire text file's contents
	vector<int> lineNumbers;	//holds the index of the last character on each line. If the line is empty, the index is the same as the last line with content

	if (args != 3)
	{
		cout << "Error starting program, did not provide text file and/or patterns file as command-line arguments!" << endl;
		return 1;
	}

	bool outInAppendMode = false;		//used to determine what output should be done (as per assignment spec, only C and D if file already existed)
	if (ifstream("A4outcomes.txt"))
		outInAppendMode = true;		//outInAppendMode determines what text is output to output file
	
	if (getTextandPatterns(argv[1], argv[2], text, lineNumbers))
	{
		horspool(outInAppendMode, text, lineNumbers);	//start Horspool
		boyerMoore(outInAppendMode, text, lineNumbers);	//start Boyer Moore
		kmp(outInAppendMode, text, lineNumbers);	//start Knuth-Morris Pratt
	}
	else
	{
		cout << "Error starting program, text or pattern file provided did not open correctly!" << endl;
		return 1;
	}
	return 0;
}

/*
Function name: getTextandPatterns
Parameters: const char *arg1, const char *arg2, string &text, vector<int> &lineNumbers
Returns: bool
Purpose: Ensure that text and pattern files are opened and read correctly
Algorithm outline:
	Open text file
	If file opened is good
		Read lines while they can be read
			Increment line count
			If line is not just a newline character (empty line)
				If the line ends with a newline character (line is not the last line of text file)
					Trim last character (newline) and add space character instead. Append line to total text string
				Else
					Append line to total text string
				Current index = text length - 1
			Put current index into line number vector (tracks the index at the end of each line)
		Close text file
	Else
		Return false (failure)
	Open patterns file
	If file opened is good
		Read lines while they can be read
			Allocate new pattern info struct (info about pattern text, first and last appearances by index and line)
			If the line ends with a newline character (line is not the last line of pattern file)
				Trim last character (newline). Set pattern to this string
			Else
				Set pattern to this string
			Set first and last appearances and indexes to 0
			Push pattern struct into vector of pattern structs
		Close pattern file
		Return true (success)
	Else
		Return false (failure)
*/
bool getTextandPatterns(const char *arg1, const char *arg2, string &text, vector<int> &lineNumbers)
{
	string lineRead;
	ifstream textFile(arg1);		//open and read text file
	if (textFile.good())
	{
		int lineReadNum = 0;
		int index = 0;
		getline(textFile, lineRead);
		text += lineRead.substr(0, lineRead.length() - 1) + " ";	//compensate for Byte Order Marking at start of file
		text.erase(0, 3);
		lineReadNum++;							//increment line count as lines read
		index = text.length() - 1;					//equal to line's last character index
		lineNumbers.push_back(index);					//add index to line number vector
		while ( getline(textFile, lineRead) )				//read lines from file
		{
			lineReadNum++;						//increment line count as lines read
			if (! isspace(lineRead[0]) || lineRead.length() >= 2)	//if the first character is not a space character or the line's length is greater than 1
			{
				if (isspace(lineRead[lineRead.length()]))
					text += lineRead.substr(0, lineRead.length() - 1) + " ";	//replace newline with space character
				else
					text += lineRead;	//or just add line if no newline character

				index = text.length() - 1;	//equal to line's last character index
			}
			lineNumbers.push_back(index);		//add index to line number vector
		}
    		textFile.close();
	}
	else
		return false;		//bad file opening

	ifstream patternFile(arg2);	//open and read pattern file
	if (patternFile.good())
	{
		while ( getline(patternFile, lineRead) )		//read patterns
		{
			patternInfo pat;				//prepare pattern struct
			if (isspace(lineRead[lineRead.length() - 1]))
				pat.patternText = lineRead.substr(0, lineRead.length() - 1);	//remove newline character from pattern
			else
				pat.patternText = lineRead;		//pattern has no newline character

			pat.firstAppearanceLine = 0;			//initialize all values
			pat.firstAppearanceIndex = 0;
			pat.lastAppearanceLine = 0;
			pat.lastAppearanceIndex = 0;
			bool punctuationFound = false;			//tests whether the pattern is solely punctuation ('?' in pattern file)
			if (! isalpha(pat.patternText[0]) && pat.patternText.length() == 1)	//looking for single punctuation character
				punctuationFound = true;

			pat.punctuationPattern = punctuationFound;

			patterns.push_back(pat);	//add pattern struct to vector of patterns
		}
		patternFile.close();
		return true;				//both file's opened and read successfully
	}
	else
		return false;				//bad file opening
}


//	HORSPOOL


/*
Function name: shiftTable_Horspool
Parameters: const string pattern, int *skipTable
Returns: nothing
Purpose: Set up the shift table for Horspool's algorithm
Algorithm outline:
	Get pattern length
	for all ASCII characters
		set shift to pattern length
	for all characters in pattern
		set shift to pattern length - 1 - current character count so far
*/
void shiftTable_Horspool(const string pattern, int *skipTable)
{
	int m = pattern.length();		//m tracks length of pattern

	for (int i = 0; i < ASCII_SIZE; i++)	//initialize all elements to m
		skipTable[i] = m;

	for (int j = 0; j < (m - 1); j++)	//set skip to distance from pattern's last character
		skipTable[pattern[j]]= m - 1 - j;
}

/*
Function name: horspool
Parameters: bool outAppendMode, const string text, vector<int> lineNumbers
Returns: nothing
Purpose: Run Horspool's algorithm
Algorithm outline:
	Prepare result output stream
	If output stream is good
		Write what algorithm is being run (Horspool)
		Begin timer
		For each pattern
			Write what pattern is being searched
			For the duration of text
				Set k to 0 (counts matching characters)
				While characters match and less characters have been checked than the pattern's length, increment k and comparison count. The increment of k will automatically cause checking of earlier characters
				If the count of matching characters is not equal to the pattern's length
					Increment comparison count
				If matching character count is equals to pattern length
					Find the line that it was found on
					Set first and last indexes and lines, of this pattern's appearance (if first appearance has been found, overwrite last appearance)
					Increment match count
			Add pattern's comparisons to the total comparisons in this algorithm
			If the output file didn't exist before this program started, write the pattern's first and last appearing line and index, as well as the matching count
			Write the comparison count
		End algorithm timer
		Write the total comparisons and time taken, as well as a separating line of hyphens
		Close output file
	Else
		Output error
		Exit program
*/
void horspool(bool outputAppendMode, const string text, vector<int> lineNumbers)
{
	ofstream resultOutput("A4outcomes.txt", ios::app);
	int totalComparisons = 0;

	if (resultOutput.good())			//output file opened successfully
	{
		resultOutput << "Results of Horspool algorithm:\n\n";
		clock_t t = clock();			//algorithm starts from this point
		for (vector<patternInfo>::iterator it = patterns.begin(); it != patterns.end(); it++)
		{
			int skipTable[ASCII_SIZE];	//basic table size to accomodate all ASCII characters (particularly this size for horspool, still re-used for others)
			string pattern = it->patternText;
			shiftTable_Horspool(pattern, skipTable);	//prepare shift table
			int patternLength = pattern.length(), patternComparisons = 0, matches = 0, lineMatched = 1;
			vector<int>::iterator itLine = lineNumbers.begin();

			resultOutput << "Results for pattern: \"" << pattern << "\"\n";

			for (int i = patternLength - 1; i <= text.length(); i += skipTable[text[i]])	//for the text's length
			{
				int k = 0;

				//test how many characters match from index
				while((k < patternLength) && (tolower(pattern[patternLength - 1 - k]) == tolower(text[i - k])))
				{
					k++;
					patternComparisons++;
				}

				//if all characters match, this is the first horspool output and (pattern is either punctuation or text surrounding pattern is non-alphabetic)
				if (k == patternLength && !outputAppendMode && (it->punctuationPattern || testSurroundingCharactersHorspool(i, patternLength, text)))
				{
					for (; i >= *itLine; itLine++)		//move line watcher forward to current index
						lineMatched++;

					if (it->firstAppearanceLine == 0)	//if first appearance is still vacant, fill values
					{
						it->firstAppearanceIndex = i - (patternLength - 1);
						it->firstAppearanceLine = lineMatched;
					}
					it->lastAppearanceIndex = i - (patternLength - 1);	//update last index and line appearance
					it->lastAppearanceLine = lineMatched;
					matches++;						//increment matches
				}
				else
					patternComparisons++;	//if text didn't match, increment comparison count
			}
			totalComparisons += patternComparisons;	//add comparisons to the algorithm's total count of comparisons

			if (! outputAppendMode)			//if first run of Horspool for this file
			{
				resultOutput << "First appears on line: " << it->firstAppearanceLine << " at text index: " << it->firstAppearanceIndex << endl;
				resultOutput << "Last appears on line: " << it->lastAppearanceLine << " at text index: " << it->lastAppearanceIndex <<endl;
				resultOutput << "Matches: " << matches << endl;
			}
			resultOutput << "Comparisons: " << patternComparisons << "\n\n";	//write comparisons
		}
		t = clock() - t;	//agorithm is complete at this point

		resultOutput << "Total comparisons in Horspool algorithm: " << totalComparisons << endl;	//write algorithm's comparisons and time taken
		resultOutput << "Horspool algorithm took a total of " << (float)t/CLOCKS_PER_SEC << " seconds\n\n";
		resultOutput << setfill('-') << setw (40) << "\n\n";

		resultOutput.close();
	}
	else
	{
		cout << "Failure to open output file in Horspool algorithm" << endl;	//bad file opening
		exit(1);
	}
}

/*
Function name: testSurroundingCharactersHorspool
Parameters: int i, int patternLength, const string text
Returns: bool
Purpose: Test whether the characters around a pattern in the text are alphanumeric, for Horspool
Algorithm outline:
	If pattern is at the start of the text and the following character is non alphanumeric
		return true
	Else if pattern is at the end of the text and the preceding character is non alphanumeric
		return true
	Else if pattern is in the middle of the text somewhere and the preceding and following characters are non alphanumeric
		return true
	Else
		return false
*/
bool testSurroundingCharactersHorspool(int i, int patternLength, const string text)
{
	if (i - patternLength + 1 == 0 && !isalnum(text[i + 1]))		//pattern match at text beginning and character following pattern is not letter or digit
		return true;
	else if (i == text.length() - 1 && !isalnum(text[i - patternLength]))	//pattern match at end of text and character preceding pattern is not letter or digit
		return true;
	else if (!isalnum(text[i - patternLength]) && !isalnum(text[i + 1]))	//characters before and after pattern are not letters or digits
		return true;
	else			//none of these cases
		return false; 	
}


//	BOYER-MOORE


/*
Function name: deltaOneSetup
Parameters: int *deltaOne, const string pattern, const int patternLength
Returns: nothing
Purpose: Refer to deltaOne table to find the distance between the end of the pattern and the rightmost occurence of each letter in the pattern
Algorithm outline:
	For every ASCII letter
		Set table value to pattern length
	For pattern length - 1
		Set table value of current pattern letter to (pattern's length - 1 - letters checked)
*/
void deltaOneSetup(int *deltaOne, const string pattern, const int patternLength)
{
	for (int i = 0; i < ASCII_SIZE; i++)
		deltaOne[i] = patternLength;

	for (int i = 0; i < patternLength - 1; i++)
		deltaOne[tolower(pattern[i])] = patternLength - 1 - i;

}

/*
Function name: deltaTwoSetup
Parameters: int *deltaTwo, const string pattern, const int patternLength
Returns: nothing
Purpose: Refer to deltaTwo table to find the next possible match, based on what letter in the text failed to match
Algorithm outline:
	From the end of the pattern to the start
		If the next character is a prefix of the pattern
			Set the last prefix occurence to the next character's position
		Set the deltaTwo value of the current pattern position to the last prefix occurence + ((pattern length - 1) - current position)
	From the pattern start to the end
		Find the suffix length
		If the suffixes do not match
			Set deltaTwo table value from the pattern end to ((patternLength - 1) - p + suffix length)
*/
void deltaTwoSetup(int *deltaTwo, const string pattern, const int patternLength)
{
	int nMinusOne = patternLength - 1, lastPrefix = patternLength - 1;
	
	for (int p = nMinusOne; p >= 0; p--)
	{
		if (isPrefix(pattern, patternLength, (p + 1)))
			lastPrefix = (p + 1);
		deltaTwo[p] = lastPrefix + (nMinusOne - p);
	}

	for (int p = 0; p < nMinusOne; p++)
	{
		int suffixLen = suffixLength(pattern, patternLength, p);
		if (tolower(pattern[p - suffixLen]) != tolower(pattern[nMinusOne - suffixLen]))
			deltaTwo[nMinusOne - suffixLen] = nMinusOne - p + suffixLen;
	}
}

/*
Function name: isPrefix
Parameters: const string pattern, const int patternLength, const int p
Returns: int i
Purpose: Find whether the prefix matches another part of the pattern string
Algorithm outline:
	Set suffix length to pattern length - current position
	For suffix length
		If pattern from start of text encounters a mismatch to the end of the text
			Return false
	Return true (no mismatch occurred)
*/
bool isPrefix(const string pattern, const int patternLength, const int p) 
{
	int suffixLen = patternLength - p;

	for (int i = 0; i < suffixLen; i++)
	{
		if (tolower(pattern[i]) != tolower(pattern[p + i]))
			return false;
	}
	return true;
}

/*
Function name: deltaTwoSetup
Parameters: const string pattern, const int patternLength, const int p
Returns: int i
Purpose: Get the length of the longest suffix of the pattern from a given index
Algorithm outline:
	Increment i while the start and end of the pattern substring match, or until i is equal to p
	Return i
*/
int suffixLength(const string pattern, const int patternLength, const int p)
{
	int i = 0;
	while ((tolower(pattern[p - i]) == tolower(pattern[patternLength - 1 - i])) && (i < p))
		i++;

	return i;
}

/*
Function name: boyerMoore
Parameters: bool outAppendMode, const string text, vector<int> lineNumbers
Returns: nothing
Purpose: Run Boyer Moore's algorithm
Algorithm outline:
	Prepare result output stream
	If output stream is good
		Write what algorithm is being run (Boyer Moore)
		Begin timer
		For each pattern
			Build prefix and suffix tables
			Output pattern name to file
			For duration of text
				Test whether the current text matches from current index backwards
				If the text matched
					Get current line
					Store location of word, based on index and line
					Increment match count
					Move index forward by pattern length
				Else
					Increment index by maximum of prefix and suffix tables
			Add comparisons to toal comparison count
			If outputting  in append mode
				Output details of first and last appearance and total matches count
			Output comparison count
		End algorithm timer
		Write the total comparisons and time taken, as well as a separating line of hyphens
		Close output file
	Else
		Output error
		Exit program
*/
void boyerMoore(bool outputAppendMode, const string text, vector<int> lineNumbers)
{
	ofstream resultOutput("A4outcomes.txt", ios::app);
	int totalComparisons = 0;
	
	if (resultOutput.good())		//open output file
	{
		resultOutput << "Results of Boyer Moore algorithm:\n\n";
		clock_t t = clock();		//algorithm starts from this point
		for (vector<patternInfo>::iterator it = patterns.begin(); it != patterns.end(); it++)
		{
			string pattern = it->patternText;
			int patternLength = pattern.length(), patternComparisons = 0, matches = 0, lineMatched = 1;
			int deltaOne[ASCII_SIZE];						//basic table size to accomodate all ASCII characters
			int *deltaTwo = new int[patternLength];
			deltaOneSetup(deltaOne, pattern, patternLength);			//prepare prefx and suffix table
			deltaTwoSetup(deltaTwo, pattern, patternLength);
			vector<int>::iterator itLine = lineNumbers.begin();

			resultOutput << "Results for pattern: \"" << pattern << "\"\n";

			for (int i = patternLength - 1; i < text.length();)			//for text length
			{
				int j = patternLength - 1;					//set pattern index to read from pattern end
				while (j >= 0 && (tolower(text[i]) == tolower(pattern[j])))	//decrement indexes until pattern has matched or mismatch occurs
				{
					i--;
					j--;
					patternComparisons++;
				}
				if (j < 0)	//if patterns match
				{
					i++;
					//if this is first boyer moore output for this file and (pattern is punctuation or surrounded by nondigits and letters)
					if (!outputAppendMode && (it->punctuationPattern || testSurroundingCharacters_MooreKMP(i, patternLength, text)))
					{
						for (; i >= *itLine; itLine++)		//move current line matched forward
							lineMatched++;

						if (it->firstAppearanceLine == 0)	//if first index and line still not set, update to current values
						{
							it->firstAppearanceIndex = i;
							it->firstAppearanceLine = lineMatched;
						}
						it->lastAppearanceIndex = i;		//update latest index and line
						it->lastAppearanceLine = lineMatched;
						matches++;				//increment matches
					}
					i += patternLength + 1;
				}
				else
				{
					i += max(deltaOne[text[i]], deltaTwo[j]);	//move index forward
					patternComparisons++;				//increment comparisons made
				}
			}

			totalComparisons += patternComparisons;				//increment total comparisons

			if (! outputAppendMode)						//first time outputting to file
			{
				resultOutput << "First appears on line: " << it->firstAppearanceLine << " at text index: " << it->firstAppearanceIndex << endl;
				resultOutput << "Last appears on line: " << it->lastAppearanceLine << " at text index: " << it->lastAppearanceIndex <<endl;
				resultOutput << "Matches: " << matches << endl;
			}
			resultOutput << "Comparisons: " << patternComparisons << "\n\n";
			delete []deltaTwo;
		}
		t = clock() - t;	//agorithm is complete at this point

		resultOutput << "Total comparisons in Boyer Moore algorithm: " << totalComparisons << endl;
		resultOutput << "Boyer Moore algorithm took a total of " << (float)t/CLOCKS_PER_SEC << " seconds\n\n";
		resultOutput << setfill('-') << setw (40) << "\n\n";

		resultOutput.close();
	}
	else
	{
		cout << "Failure to open output file in Boyer Moore algorithm" << endl;	//bad file opening
		exit(1);
	}
}

/*
Function name: max
Parameters: int x, int y
Returns: int
Purpose: Get the maximum value
Algorithm outline:
	Return the larger value, x if equal or less than
*/
int max(int x, int y)
{
	return ((x < y) ? y : x);
}


//	KNUTH-MORRIS PRATT


/*
Function name: shiftTable_kmpTable
Parameters: const string pattern, int *skipTable
Returns: nothing
Purpose: Set up the shift table for Knuth-Morris Pratt's algorithm
Algorithm outline:
	Set first and second skips to -1 and 0, respectively
	For the rest of the pattern's length
		If the current pattern element is equal to the last prefix ending
			Increment prefix index
			Set skip to index of prefix ending
			Increment current character
		Else if prefix is not null
			set prefix to the current prefixes prefix (0 if this prefix has no shared prefix)
		Else
			Set the skip to 0
			Increment current character
*/
void shiftTable_kmpTable(const string pattern, int *skipTable)
{
	int k = 2, i = 0;
	skipTable[0] = -1;
	skipTable[1] = 0;

	while (k < pattern.length())
	{	//the substring continues
		if (pattern[k - 1] == pattern[i])
		{
			i++;
			skipTable[k] = i;
			k++;
		}
		//substring doesn't contnue, revert
		else if (i > 0)
			i = skipTable[i];

		//run out of letters
		else
		{
			skipTable[k] = 0;
			k++;
		}
	}
}
/*
Function name: kmp
Parameters: bool outAppendMode, const string text, vector<int> lineNumbers
Returns: nothing
Purpose: Run Knuth Morris Pratt's algorithm
Algorithm outline:
	Prepare result output stream
	If output stream is good
		Write what algorithm is being run (Knuth Morris Pratt)
		Begin timer
		For each pattern
			Prepare shift table for pattern
			Write what pattern is being searched
			For the duration of text
				If character at index matches
					If the word has been matched, output is in append mode and the word's surrounding characters have been tested for
						Find the current line
					If first appearance has not yet been found
						Fill first and last appearance values with current values
					Else
						Fill last appearance values
				Else
					Refer to skip table for skip distance
				Increment character comparison count
			Add pattern comparisons to the total count of comparisons for the algorithm
			If outputting  in append mode
				Output details of first and last appearance and total matches count
			Output comparison count
		End timer
		Output algorithm's total comparisons and time
		Close output stream
	Else
		Output error
		Exit program
			
*/
void kmp(bool outputAppendMode, const string text, vector<int> lineNumbers)
{
	ofstream resultOutput("A4outcomes.txt", ios::app);
	int fileLength = text.length(), totalComparisons = 0;

	if (resultOutput.good())
	{
		resultOutput << "Results of Knuth-Morris Pratt algorithm:\n\n";
		clock_t t = clock();		//algorithm starts from this point

		for (vector<patternInfo>::iterator it = patterns.begin(); it != patterns.end(); it++)
		{
			int skipTable[100];	//basic table size to accomodate pattern skip table
			string pattern = it->patternText;
			int lineMatched = 1, patternComparisons = 0, matches = 0, i = 0, m = 0, patternLength = pattern.length();
			shiftTable_kmpTable(pattern, skipTable);
			vector<int>::iterator itLine = lineNumbers.begin();

			resultOutput << "Results for pattern: \"" << pattern << "\"\n";

			while ((m + 1) < text.length())					//iterate through text
			{
				if (tolower(pattern[i]) == tolower(text[m + i]))	//if text at index matches pattern's same index
				{
					//if pattern matched in text, first time outputting kmp to this file and (pattern is punctuation or surrounded by digits or letters)
					if ( i == patternLength - 1 && !outputAppendMode && (it->punctuationPattern || testSurroundingCharacters_MooreKMP(m, patternLength, text)))
					{
						for (; m >= *itLine; itLine++)		//move current line marker forward to current line
							lineMatched++;

						if (it->firstAppearanceLine == 0)	//if first appearances are still unset
						{
							it->firstAppearanceIndex = m - (patternLength - 1);
							it->firstAppearanceLine = lineMatched;
						}
						it->lastAppearanceIndex = m - (patternLength - 1);	//update last appearances
						it->lastAppearanceLine = lineMatched;
						matches++;				//increment matches
					}
					i++;
				}
				else
				{
					m = (m + i) - skipTable[i];			//move index checking forward based on skip table
					i = (skipTable[i] > -1) ? skipTable[i] : 0;
				}
				patternComparisons++;
			}
			totalComparisons += patternComparisons;					//add pattern comparisons to total

			if (! outputAppendMode)
			{
				resultOutput << "First appears on line: " << it->firstAppearanceLine << " at text index: " << it->firstAppearanceIndex << endl;
				resultOutput << "Last appears on line: " << it->lastAppearanceLine << " at text index: " << it->lastAppearanceIndex <<endl;
				resultOutput << "Matches: " << matches << endl;
			}
			resultOutput << "Comparisons: " << patternComparisons << "\n\n";
		}

		t = clock() - t;								//agorithm is complete at this point
		resultOutput << "Total comparisons in Knuth-Morris Pratt algorithm: " << totalComparisons << endl;
		resultOutput << "Knuth-Morris Pratt algorithm took a total of " << (float)t/CLOCKS_PER_SEC << " seconds\n\n";
		resultOutput << setfill('-') << setw (40) << "\n\n";
		resultOutput.close();
	}
	else
	{
		cout << "Failure to open output file in Knuth-Morris Pratt algorithm" << endl;	//bad file opening
		exit(1);
	}
}

/*
Function name: testSurroundingCharacters_MooreKMP
Parameters: int m, int patternLength, const string text
Returns: bool
Purpose: Test whether the characters around a pattern in the text are alphanumeric, for Boyer Moore and KMP
Algorithm outline:
	If pattern is at the start of the text and the following character is non alphanumeric
		return true
	Else if pattern is at the end of the text and the preceding character is non alphanumeric
		return true
	Else if pattern is in the middle of the text somewhere and the preceding and following characters are non alphanumeric
		return true
	Else
		return false

NOTE: this algorithm is not a repeat of the horspool check! The if statements are different, as is the way the index value is passed
*/
bool testSurroundingCharacters_MooreKMP(int i, int patternLength, const string text)
{
	if (i == 0 && !isalnum(text[i + patternLength]))	//if pattern is at start of text and following character is neither letter nor digit
		return true;
	else if (i == text.length() - (patternLength - 1) && !isalnum(text[i - 1]))	//if pattern is at end of text and preceding character is neither letter nor digit
		return true;
	else if (!isalnum(text[i - 1]) && !isalnum(text[i + patternLength]))	//if characters before and after pattern are not letters or digits
		return true;
	else			//tests fail
		return false;
}

