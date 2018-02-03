#include "stdafx.h"
#include "Str.h"
#include "CStr.h"
#include "ArrayOfStr.h"
#include "ArrayOfCStr.h"
#include "Services.h"
#include <stdarg.h> //va_start and other macros for variable argument lists.

//==============================================================================
ArrayOfCStr::ArrayOfCStr(const ArrayOfStr & ARR){
	int size = ARR.GetSize();
	Debug::Assert(size > 0);
	m_Size = size;
	m_pType = new CStr[m_Size]; //Assumes TYPE has a default constructor.
	for(int index = 0; index < m_Size; ++index){
		const Str & sz = ARR.GetAt(index);
		CStr cz(sz);
		SetAt(index, cz);
	}
}										//Constructor from ArrayOfStr
//--------------------------------------------------------------------
int ArrayOfCStr::CreateBySeparatingIntoNonemptyWords(const Str & phrase){
//A word begins at the beginning of m_pC or just after each group of isspace characters
//(isspace characters are space, formfeed, newline, carriage return, tab, vertical tab).
//The word ends just before the following isspace or terminating '\0' character. 
//Returns the number of non-empty words and the array "Words" of Str objects.
//The Str objects contain no isspace characters and are all non-empty.

	int nWords = phrase.CountNonemptyWords();
	SetArraySize(nWords);
	if(nWords == 0)return 0;
	const char * ps = phrase.GetString();
	const char * pb = ps;
	int word = 0;
	while(true){
		//Put pa at the next nonspace character or '\0' in "phrase".
		const char * pa = pb;
		while(*pa && isspace(*pa)){++pa;}
		if(!(*pa))break; //found `\0'
		//pa is now at a non-isspace character. Put pb at the next isspace character or '\0'.
		pb = pa;
		while(*pb && !isspace(*pb)){++pb;}
		Debug::Assert(pb > pa && word < nWords);
		//Count the word between pa and pb - 1.
		CStr & row = m_pType[word]; //m_pType is this ArrayOfCStr
		++word;
//bool CStr::Create(const char * mainString, int offset, int nbrCharacters);
		row.Create(ps, pa - ps, pb - pa);
		if(!(*pb))break;
	}
	Debug::Assert(word == nWords);
	return nWords;
}										//CreateBySeparatingIntoNonemptyWords
//--------------------------------------------------------------------
int ArrayOfCStr::CreateBySeparatingIntoNonemptyLines(const Str & text){
//A line starts at the beginning of m_pC or just after each '\n' character.
//The line ends just before the following '\n' character or terminating '\0'.
//Returns the number of nonempty lines and the array "Lines" of Str objects.
//The Str objects contain no '\n' characters.

	int nLines = text.CountNonemptyLines();
	SetArraySize(nLines);
	if(nLines == 0)return 0;
	const char * ps = text.GetString();
	const char * pb = ps;
	int line = 0;
	while(true){
		const char * pa = pb;
		//Put pb at the next '\n' or '\0'.
		while(*pb && *pb != '\n'){++pb;}
		if(pb > pa){  //*pb could now be `\n'
			Debug::Assert(line < nLines);
			//Count the line between pa and pb - 1.
			CStr & row = m_pType[line]; //m_pType is this ArrayOfCStr
			row.Create(ps, pa - ps, pb - pa);
			++line;
		}
		if(*pb == '\0')break;
		//Skip the '\n'.
		Debug::Assert(*pb == '\n');
		++pb;
	}
	Debug::Assert(line == nLines);
	return nLines;
}										//CreateBySeparatingIntoNonemptyLines
//--------------------------------------------------------------------
int ArrayOfCStr::CreateBySeparatingIntoNonemptyWords(const CStr & phrase){

	int nWords = phrase.CountNonemptyWords();
	SetArraySize(nWords);
	if(nWords == 0)return 0;
	const char * ps = phrase.GetMainString();
	const char * pb = phrase.GetRangeString();     //MainString + Offset;
	const char * pc = phrase.GetPointerPastEnd();  //pb + NumberCharacters;
	int word = 0;
	while(true){
		//Put pa at beginning of a word
		const char * pa = pb;
		//Put pb at the next isspace character or just after end of Range.
		while(pb < pc && !isspace(*pb)){++pb;}
		//A nonempty word has been found.
		if(pb > pa){
			Debug::Assert(word < nWords);
			CStr & row = m_pType[word];
			row.Create(ps, pa - ps, pb - pa);
			row.Trim();
			++word;
		}
		//Break if past the end of Range.
		if(pb == pc)break;
		//Skip the isspace character.
		Debug::Assert(isspace(*pb) != 0); //isspace returns nonzero for spaces.
		++pb;
	}
	Debug::Assert(word == nWords);
	return nWords;
}										//CreateBySeparatingIntoNonemptyWords
//--------------------------------------------------------------------
int ArrayOfCStr::CreateBySeparatingIntoLines(const Str & text){
	int nLines = text.CountLines();
	Debug::Assert(nLines >= 0);
	SetArraySize(nLines);
	if(nLines == 0)return 0; 

	const char * ps = text.GetString();
	const char * pb = ps; //put pb at beginning of string
	int line = 0;
	while(true){
		const char * pa = pb;
		//Put pb at the next '\n' or '\0'.
		while(*pb && *pb != '\n'){++pb;}
		//A line has been found.
		Debug::Assert(line < nLines);
		CStr & row = m_pType[line];
		row.Create(ps, pa - ps, pb - pa);
		++line;
		if(*pb == '\0')break;
		//Skip the '\n'.
		Debug::Assert(*pb == '\n');
		++pb; //move pb after the `\n'
	}
	Debug::Assert(line == nLines);
	return nLines;
}										//CreateBySeparatingIntoLines
//--------------------------------------------------------------------
int ArrayOfCStr::FindCStr(const CStr & e)const{
//Returns the index of the CStr in this array that is equal to e, or -1 if not found. 
	for(int i = 0; i < m_Size; i++){
		if(e.CompareCaseSensitive(m_pType[i]) == 0)return i; //Compare returns 0 when the two strings are equal.
	}
	return -1;
}										//FindCStr
//--------------------------------------------------------------------
int ArrayOfCStr::FindString(const char * ss)const{
//Returns the index of the CStr in this array that is equal to ss, or -1 if not found. 
//CompareCaseSensitive uses strcmp, which returns 0 only if the two strings have the 
//same length and contain the same characters.
	for(int i = 0; i < m_Size; i++){
		if(m_pType[i].CompareCaseSensitive(ss) == 0)return i; //Compare returns 0 when the two strings are equal.
	}
	return -1;
}										//FindString
//--------------------------------------------------------------------
int ArrayOfCStr::FindCStrBeginningWithString(const char * ss)const{
//Returns the index of the first CStr in this array that begins with ss, or -1 if not found. 
	int L = strlen(ss);
	if(L == 0)return -1;
	CStr cz;
	for(int i = 0; i < m_Size; i++){
		//Extract the initial L chars of m_pType[i] into cz.
		bool bb = cz.Create(0, L - 1, m_pType[i]); //returns false if m_pType[i] has less than L characters.
		if(bb){
			if(cz.CompareCaseSensitive(ss) == 0)return i; //Compare returns 0 when the two strings are equal.
		}
	}
	return -1;
}										//FindCStrBeginningWithString
//--------------------------------------------------------------------
bool ArrayOfCStr::FindEnvironment(const char * key, int & beginLine, int & endLine, Str & options)const{
//Finds an Environment such as "\begin{key}[option1, option2, ...]  ...  \end{key}".

	Debug::Assert(m_Size > 0);
	Str sa, sb;
	sa.Format("\\begin{%s}", key);
	sb.Format("\\end{%s}", key);
	beginLine = FindCStrBeginningWithString(sa.GetString()); //options allowed
	endLine = FindString(sb.GetString());  //no options allowed, must be exact.
	Debug::Assert((beginLine >= 0 && endLine > beginLine) || (beginLine < 0 && endLine < 0));
	if(beginLine < 0)return false;  //Environment not found.

	//Environment found. Read the options.
	Str command = GetAt(beginLine);
	CStr textBetween;
	int k = command.FindTextBetween('[', ']', textBetween); //returns -1 when no options.
	if(k < 0)options.Empty();
	else     options = textBetween;
	return true;
}										//FindEnvironment
//--------------------------------------------------------------------
int ArrayOfCStr::AppendCStrIfNotThere(CStr & e){
//If e is not in the array, extends the array by one unit and stores e.
//Returns the index where e was stored or found. Can be inefficient.
	int index;
	if((index = FindCStr(e)) >= 0)return index;
	SetArraySize(m_Size + 1);
	m_pType[m_Size - 1] = e;
	return m_Size - 1;
}										//AppendCStrIfNotThere
//--------------------------------------------------------------------
int ArrayOfCStr::GetTotalLengthOfStrings()const{
	int L = 0;
	int N = GetSize();
	for(int i = 0; i < N; ++i){
		const CStr & s = GetAt(i);
		int len = s.GetNumberCharacters();
		L += len;
	}
	return L;
}
//--------------------------------------------------------------------
bool ArrayOfCStr::ToStr(Str & result, int nSpaces)const{
//Creates an Str with all the CStr objects, leaving nSpaces spaces in between.
	if(nSpaces < 0)return false;
	result.Empty();
	int N = GetSize(); //Number of CStr objects.
	int L = GetTotalLengthOfStrings() + (N - 1) * nSpaces; //Total length of result.
	result.SetSize(L);
	int p = -1;
	//Scan all CStr objects.
	for(int i = 0; i < N; ++i){
		const CStr & s = GetAt(i);
		//Insert characters from s into result.
		int len = s.GetNumberCharacters();
		for(int j = 0; j < len; ++j){
			++p;
			char cc = s.GetAt(j);
			result.SetAt(p, cc);
		}
		//Insert spaces after s, but not after the last one.
		if(i < (N - 1)){
			for(int b = 0; b < nSpaces; ++b){
				++p;
				result.SetAt(p, ' ');
			}
		}
	}
	return true;
}										//ToStr
//--------------------------------------------------------------------
