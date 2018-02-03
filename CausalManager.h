#pragma once
#include "MyIncludes.h"
#include "CausalExamples.h"
#include "CausalRelations.h"
#include "Str.h"

//--------------------------------------------------------------
class CausalManager{
private:
	CausalExamples CE;
	CausalRelations CR;
	Str WorkingDirectory;
	Str CommandFile, OutputFile;
	ArrayOfIntegers InitialPermutation;
	FILE * pCommandFile, * pOutputFile;
public:
	CausalManager();
	virtual ~CausalManager(void);
	void CausalManagerProceed();
	void MenuTests(const char * text)const;
	void ReadAndExecuteCommands();
};										//Class CausalManager
//--------------------------------------------------------------


