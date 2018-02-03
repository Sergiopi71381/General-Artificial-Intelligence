#pragma once
#include "MyIncludes.h"
class CStr;
class ArrayOfCStr;
class ArrayOfStr;
using namespace System; //String.
//--------------------------------------------------------------------
class Str{
	//A string of chars that owns array m_pC and deletes it when the 
	//object is destructed.  m_pC is a nullptr-terminated array of char.
protected:
	int m_Size; //m_Size does not include the terminating '\0'.
	char * m_pC;
private:
	int vLength(va_list & vl, const char * types);
	void vCat(va_list & vl, const char * types);
public:
	//Empty Str.
	Str();
	//Given size and undefined content.
	Str(int size);
	//Copy constructor.
	Str(const Str & other);
	//From a nullptr-terminated string.
	Str(const char * ch);
	//n times the same character.
	Str(int n, char ch);
	//From a CStr.
	Str(const CStr & cs); //The entire CStr.
	//From a substring.
	Str(const char * ch, int start, int nChars); //nChars characters starting at start.
	//Concatenate many nullptr-terminated strings.
	Str(const char * types, const char * ch, ...);
	Str(const char * types, const Str * st, ...);
	Str(const System::String ^ ss);
	virtual ~Str();

	//Get the internal string.
	inline int GetLength()const{return m_Size;}
	inline const char * GetString()const{return m_pC;}
	//True if zero length.
	inline bool IsEmpty() const{return (m_Size == 0);}
	inline bool IsInRange(int index)const{return(index >= 0 && index < m_Size);}
	inline bool IsInRange(const char * p)const{return IsInRange(int(p - m_pC));}
	//Clear contents to empty.
	inline void Empty(){delete [] m_pC; m_pC = new char[1]; m_pC[0] = '\0'; m_Size = 0;}
	//Return single character at index.
	inline char GetAt(int index)const{
		Debug::Assert(IsInRange(index)); 
		return m_pC[index];
	}
	//Set a single character at index.
	inline void SetAt(int index, char ch){
		Debug::Assert(IsInRange(index)); 
		m_pC[index] = ch;
	}

	//Create an Str of fixed size and undefined contents.
	bool SetSize(int size);
	//Create this Str from data.
	bool Create(const char * ch);   //nullptr-terminated
	bool Create(const char * ch, int size); //given size.
	bool Create(const char * ch, int start, int nChars); //nChars characters starting at start.
	bool Create(int n, char ch);   //n times the same character.
	bool Create(const CStr & ct);
	bool Create(const Str & st);
	bool Create(int nn);  //Converts the number into characters.
	void RightJustifyInFieldOfSize(int size); //this Str is right-justified or left-trimmed into a field of size "size"

	//Type conversion operator. It allows an object of this class to be used just like an array.
	//Example: const char * cc = Str-object. Also as an argument to a function that takes a const char *.
	inline operator const char * ()const{return m_pC;}

	//Assignment, concatenation and other operators.
	const Str & operator=(const Str & other);
	const Str & operator=(const char * ch);
	const Str & operator+=(const Str & other);
	const Str & operator+=(const char * ch);
	const Str & operator+=(char ch);
	Str operator+(char ch)const;
	Str operator+(const char * ch)const;
	Str operator+(const Str & s)const;
	friend Str operator+(const char * ch, const Str & s2); //(non-member)
	inline char & operator[](int index)const{
		Debug::Assert(IsInRange(index)); 
		return m_pC[index];
	}
	bool operator==(const char * other)const{return (CompareCaseSensitive(other) == 0);}
	bool operator==(const Str & other)const{return (CompareCaseSensitive(other)  == 0);}
	bool operator==(const CStr & other)const{return (CompareCaseSensitive(other) == 0);}
	bool operator!=(const char * other)const{return (CompareCaseSensitive(other) != 0);}
	bool operator!=(const Str & other)const{return (CompareCaseSensitive(other)  != 0);}
	bool operator!=(const CStr & other)const{return (CompareCaseSensitive(other) != 0);}
	bool operator<=(const char * other)const{return (CompareCaseSensitive(other) <= 0);}
	bool operator<=(const Str & other)const{return (CompareCaseSensitive(other)  <= 0);}
	bool operator<=(const CStr & other)const{return (CompareCaseSensitive(other) <= 0);}
	bool operator>=(const char * other)const{return (CompareCaseSensitive(other) >= 0);}
	bool operator>=(const Str & other)const{return (CompareCaseSensitive(other)  >= 0);}
	bool operator>=(const CStr & other)const{return (CompareCaseSensitive(other) >= 0);}
	bool operator<(const char * other)const{return (CompareCaseSensitive(other)  < 0);}
	bool operator<(const Str & other)const{return (CompareCaseSensitive(other)   < 0);}
	bool operator<(const CStr & other)const{return (CompareCaseSensitive(other)  < 0);}
	bool operator>(const char * other)const{return (CompareCaseSensitive(other)  > 0);}
	bool operator>(const Str & other)const{return (CompareCaseSensitive(other)   > 0);}
	bool operator>(const CStr & other)const{return (CompareCaseSensitive(other)  > 0);}

	//Formatting and Editing
	void TrimRight();
	void TrimLeft();
	void Trim();
	void StripLeft(int nChars);
	void StripRight(int nChars);
	Str Left(int nChars)const;
	Str Right(int nChars)const;
	Str Mid(int nStart)const;
	Str Mid(int nStart, int nChars)const;
	CStr CLeft(int nChars)const;
	CStr CRight(int nChars)const;
	CStr CMid(int nStart)const;
	CStr CMid(int nStart, int nChars)const;
	int GetTextBetweenBrackets(int openBracketIndex, CStr & text)const;
	int GetTextBetweenBracketsExclusive(char exclude, int openBracketIndex, CStr & text)const;
	void Insert(int nStart, const char * ch);
	void Remove(int nStart, int nChars);
	void Overwrite(const char * text, char justify);
	void Format(const char * format, ...);
	void CompactSpaces();
	Str TSQ()const; //Replace each single-quotation mark with a pair of quotation marks.
	Str & ToLower();
	Str & ToUpper();
	Str & FillSpaces(char fillChar);
	void Fill(char cc)const; //Fills with cc without changing m_Size.
	void ReplaceCharacterWithString(char c, const char * text);
	void RemoveAllOccurencesOfCharacter(char c);

	//Tabulation.
	bool RightJustify(const Str & data); //Right-justifies data into this Str, and fills with spaces on the left.
	bool RightJustify(int nn); //Right-justifies the number nn into this Str, and fills with spaces on the left.

	//Line operations and tabulation.
	void LeftJustifyLines(); //Eliminates blank lines and spaces on the left of lines.
	void JustifyLines(); //Eliminates blank lines and spaces on the left and right of lines.
	void RemoveComments(const char * commentMark);
	void RemoveBetweenMarks(const char * startMark, const char * endMark);
	void RemoveBlankLines();
	void SpliceContinuationLines();
	//int SeparateIntoLines(ArrayOfCStr & Lines)const;
	//int SeparateIntoArray(ArrayOfStr & Lines)const;
	//int SeparateIntoNonemptyLines(ArrayOfStr & Lines)const;
	//int SeparateIntoNonemptyLines(ArrayOfCStr & Lines)const;
	//int SeparateIntoNonemptyWords(ArrayOfCStr & Words)const;
	//int SeparateIntoNonemptyWords(ArrayOfStr & Words)const; //became ArrayOfStr::CreateBySeparatingIntoNonemptyWords
	//int SeparateIntoWordsUsingSeparator(char separator, ArrayOfCStr & Words)const;
	//int SeparateIntoWordsUsingSeparator(char separator, ArrayOfStr & Words)const;
	//Find the first line that starts at or after "start", even if the line is empty.
	//Find the last line that ends at or before "end", even if the line is empty.
	int FindFirstLine(CStr & line)const;
	int FindNextLine(int newline, CStr & line)const;
	int FindFirstLineExclusive(char cc, CStr & line)const;
	int FindNextLineExclusive(char cc, int newline, CStr & line)const;

	//Find the Nth line that starts with a given word.
	bool FindNthLineStartingWithWord(int Nth, const char * word, CStr & Line)const;
	int FindLineNumberWithPosition(int position, int & column)const;

	//Command operations.
	int  FindNextCommand(int start, CStr & title, CStr & content)const;
	bool FindNthCommandWithTitle(int Nth, const Str & title, CStr & content)const;
	int  FindNextCommandExclusive(char exclude, int start, CStr & title, CStr & content)const;
	bool FindNthCommandWithTitleExclusive(char exclude, int Nth, const Str & title, CStr & content)const;

	//Search operations.
	//Find next or previous isspace or nonspace character.
	int FindNextIsspace(int start)const;
	int FindPreviousIsspace(int start)const;
	int FindNextNonspace(int start)const;
	int FindPreviousNonspace(int start)const;
	int FindNextAlpha(int start)const;
	int FindNextNonalpha(int start)const;

	int FindNextDigit(int start)const; //return -1 if not found.
	int FindNextDigit(int startSearch, int & value)const; //calculates value and returns index, or -1 if not found.
	int FindNextNondigit(int start)const;
	int FindNextBlockOfDigits(int startSearch, int & nDigits, int & value)const; //returns index, or -1 if not found.

	int FindNextOneOf(const char * theChars, int start)const;
	int FindNextOneOf(const ArrayOfStr & strings, int start, int & which)const;
	int FindNextOneOfSkipEnclosures(const char * theChars, int start)const;
	int FindPreviousOneOfSkipEnclosures(const char * theChars, int start)const;
	//Find character starting at left, -1 if not found.
	int Find(char cc)const;
	//Find character starting at right.
	int ReverseFind(char cc)const;
	//Find character at or after nStart and going right.
	int Find(char cc, int nStart)const;
	//Find character at or before nStart and going left.
	int ReverseFind(char cc, int nStart)const;
	//Find first instance of substring.
	int Find(const char * s)const;
	//Find first instance of substring starting at zero-based index.
	int Find(const char * s, int nStart)const;
	//Find second character in given in "cc", but only if first character is not present.
	int FindExclusive(const char * cc, int nStart)const;
	int ReverseFindExclusive(const char * cc, int nStart)const;
	//Find text between two chars or strings, search from left to right.
	int FindTextBetween(char ca, char cb, CStr & textBetween, int start = 0)const;
	int FindTextBetween(const char * s1, const char * s2, CStr & textBetween, int start = 0)const;
	//Given any of ( [ { < ) ] } >, find the match.
	int FindMatchingBracket(int index)const;
	//Given any of ( [ { < ) ] } >, find the match that is NOT preceded by "exclude".
	int FindMatchingBracketExclusive(char exclude, int index)const;
	//Find the first space-delimited word that starts at or after "start".
	int FindNextSpaceDelimitedWord(int start, CStr & word)const;
	//Find the last space-delimited word that ends at or before "end".
	int FindPreviousSpaceDelimitedWord(int end, CStr & word)const;
	//Find the first nonAlphaNumeric-delimited word that starts at or after "start".
	int FindNextNonalnumDelimitedWord(int start, CStr & word)const;
	//Find the last nonAlphaNumeric-delimited word that ends at or before "end".
	int FindPreviousNonalnumDelimitedWord(int end, CStr & word)const;

	//Counting.
	int CountOccurrences(char c)const;
	int CountOccurrences(char c, int position)const;
	int CountOccurrences(const char * s)const;
	int CountLines()const;
	int CountLinesExclusive(char cc)const;
	int CountNonemptyLines()const;
	int CountNonblankLines()const;
	int CountNonemptyWords()const;

	//Compare. Return  <0 if this<bs,  =0 if this==bs,  >0 if this>bs.
	int CompareCaseInsensitive(const char * bs)const; //Prior to 03-14-2009, all Compare were Case-insensitive.
	int CompareCaseInsensitive(const Str & bs)const;
	int CompareCaseInsensitive(const CStr & bs)const;
	int CompareCaseSensitive(const char * bs)const;
	int CompareCaseSensitive(const Str & bs)const;
	int CompareCaseSensitive(const CStr & bs)const;
	bool StartsWith(const char * wrd)const;
	bool EndsWith(const char * wrd)const;

	//File operations.
	bool WriteMyStringToFile(FILE * file)const;
	bool WriteMeToFile(FILE * file)const;
	bool ReadMeFromFile(FILE * file);
	bool FindMeInFile(FILE * file)const;
	bool ReadFileUntilMeFound(FILE * file, char * buff)const;
	bool ReadFileIntoMe(FILE * file);

	//Character class tests.
	bool isAlnum()const; //isAlpha or isDigit is true.
	bool isAlpha()const; //isLower or isUpper is true.
	bool isDigit()const; //decimal digit.
	bool isGraph()const; //printing character except space.
	bool isLower()const; //lower-case letter.
	bool isUpper()const; //upper-case letter.
	bool isPrint()const; //printing character including space.
	bool isPunct()const; //printing character except space, letter or digit.
	bool isSpace()const; //space, formfeed, newline, carriage return, tab, vertical tab.

	//Conversions and COM support.
	int ConvertMeToNumber(double & d, long & i)const;
	bool AtoI(long & ii)const;
	bool AtoF(double & dd)const;
//	BSTR GetBSTR()const;
	System::String ^ GetSystemString()const;

	//Testing.
	bool Verify()const;
};										//Class Str
//--------------------------------------------------------------------


