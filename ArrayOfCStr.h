#pragma once
#include "MyIncludes.h"
#include "PArray.h"
#include "Str.h" //needed for the copy constructor in PArray.
class ArrayOfIntegers;
//--------------------------------------------------------------------
class ArrayOfCStr : public PArray<CStr>{
public:
	ArrayOfCStr() : PArray<CStr>() {} //Default constructor
	ArrayOfCStr(int size) : PArray<CStr>(size) {} //Constructor
	ArrayOfCStr(const ArrayOfStr & ARR);
	virtual ~ArrayOfCStr(){} //Destructor
	int CreateBySeparatingIntoNonemptyWords(const Str & phrase);
	int CreateBySeparatingIntoNonemptyLines(const Str & text);
	int CreateBySeparatingIntoNonemptyWords(const CStr & phrase);
	int CreateBySeparatingIntoLines(const Str & text);
	int GetTotalLengthOfStrings()const;
	int FindCStr(const CStr & e)const; //exactly same characters and same length.
	int FindString(const char * ss)const; //exactly same characters and same length.
	int FindCStrBeginningWithString(const char * ss)const;
	bool FindEnvironment(const char * key, int & beginLine, int & endLine, Str & options)const;
	int AppendCStrIfNotThere(CStr & e);
	bool ToStr(Str & result, int nSpaces = 0)const;
};										//Class ArrayOfCStr
//--------------------------------------------------------------------
