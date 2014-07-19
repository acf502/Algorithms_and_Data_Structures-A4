/*
 * Name: Aaron Colin Foote
 * User: acf502	4258770
 * Date: 31st October 2013
 * File Description: Assignment 4 header file for CSCI203
**/

#ifndef __A4_H_
#define __A4_H_

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sys/time.h>
#include <vector>

using namespace std;

struct patternInfo {
	string patternText;
	int firstAppearanceLine;
	int firstAppearanceIndex;
	int lastAppearanceLine;
	int lastAppearanceIndex;
	bool punctuationPattern;	//true if text contains or is punctuation
};

#endif

