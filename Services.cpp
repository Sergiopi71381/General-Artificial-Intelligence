#include "stdafx.h"
#include "Services.h"
#include "Formatter.h"
#include "MyIncludes.h"
#include "Str.h"
#include "CStr.h"
#include "ArrayOfStr.h"
#include "ArrayOfCStr.h"
#include "ArrayOfIntegers.h"
#include "PMatrix.h"
#include <math.h>
#include <float.h>
#include <time.h>
#include <stdlib.h>
using namespace System;  //String
using namespace System::Windows::Forms;
using namespace System::Diagnostics;

const double Services::AccelerationOfGravity(9.81);
const double Services::DegToRadians(0.0174532925199433);
const double Services::pi(3.14159265358979323);
//--------------------------------------------------------------------
Services::Services(){

}										//Constructor
//--------------------------------------------------------------------
Services::~Services(){

}										//Destructor
//--------------------------------------------------------------------
int Services::Factorial(int N){
	if(N == 0 || N == 1)return 1;
	Debug::Assert(N > 1);
	Debug::Assert(N <= 10); //10! = 3,628,800.
	int F = 1;
	for(int k = 2; k <= N; ++k){
		F *= k;
	}
	return F;
}										//Factorial
//--------------------------------------------------------------------
Str Services::cnv(const Str & ss){
//Given an Str, inserts a "\" before every "_" or "\" or "&".
//Also, replace every '~' with "$\sim$".
	int L = ss.GetLength();
	if(L == 0)return ss;

	//Count number of occurrences.
	int ia = ss.CountOccurrences('_');
	int ib = ss.CountOccurrences('\\');
	int ic = ss.CountOccurrences('&');
	int id = ss.CountOccurrences('~');

	int n = L + ia + ib + ic + 5 * id; //The new length.

	//Insert the back-slashes.
	Str sa(n);
	int j = 0;
	for(int i = 0; i < L; ++i){
		char cc = ss[i];
		if(cc == '_' || cc == '\\' || cc == '&'){
			sa[j++] = '\\';
			sa[j++] = cc;
		}
		else if(cc == '~'){
			sa[j++] = '$';
			sa[j++] = '\\';
			sa[j++] = 's';
			sa[j++] = 'i';
			sa[j++] = 'm';
			sa[j++] = '$';
		}
		else{
			sa[j++] = cc;
		}
	}
	Debug::Assert(j == n);
	return sa;
}										//cnv
//--------------------------------------------------------------------
Str Services::cnv(const CStr & ss){
	Str sz(ss);
	return cnv(sz);
}										//cnv
//--------------------------------------------------------------------
FILE * Services::OpenFileForReading(const char * path){
	Debug::Assert(path != nullptr);
	FILE * file;
	errno_t err = fopen_s(&file, path, "rt");
	if(err){
		WriteLine("Services::OpenFileForReading. Cannot open the file. Path=%s", path);
		return nullptr;
	}
	Debug::Assert(!err);
	Debug::Assert(file != nullptr);
	return file;
}								//OpenFileForReading
//--------------------------------------------------------------------
FILE * Services::OpenFileForWriting(const char * path){
	Debug::Assert(path != nullptr);
	FILE * file;
	errno_t err = fopen_s(&file, path, "wt");
	Debug::Assert(!err);
	Debug::Assert(file != nullptr);
	return file;
}								//OpenFileForWriting
//--------------------------------------------------------------------
FILE * Services::OpenFileForAppending(const char * path){
//Opens the file and positions at its end.
	Debug::Assert(path != nullptr);
	FILE * file;
	errno_t err = fopen_s(&file, path, "a+");
	Debug::Assert(!err);
	Debug::Assert(file != nullptr);
//	int ee = fseek(file, 0L, SEEK_END);
//	Debug::Assert(ee == 0);
	return file;
}								//OpenFileForAppending
//--------------------------------------------------------------------
void Services::SaveToFile(const Str & ss, const char * path){
//"path" is the complete file path. Example: "C:\temporary\AA.txt".
	FILE * file = Services::OpenFileForReading(path);
	SaveToFile(ss, file);
	fclose(file);
}										//SaveToFile
//--------------------------------------------------------------------
void Services::SaveToFile(const Str & ss, FILE * file){
//Saves the Str object as text.
	Debug::Assert(file != nullptr);
	int sc = sizeof(char);
	int len = ss.GetLength();
	if(len == 0)return;       //Does not save empty strings!!!
	int n = fwrite(ss.GetString(), sc, len, file);
	Debug::Assert(n == len);
}										//SaveToFile
//--------------------------------------------------------------------
void Services::AppendFileToFile(const Str & pathA, const Str & pathB){
//Appends file B to file A. File B is not modified. If fileA does not exist, it will be created.
//Note fseek does not work in C++.
	//Open and read fileB.
	FILE * fileB = Services::OpenFileForReading(pathB);
	Str textB;
	bool bb = ReadFileIntoString(fileB, textB);
	Debug::Assert(bb);
	fclose(fileB);

	//Open fileA and position at its end.
	FILE * fileA = Services::OpenFileForAppending(pathA);
	
	//Append fileB to the end of fileA.
	SaveToFile(textB, fileA);
	fclose(fileA);
}										//AppendFileToFile
//--------------------------------------------------------------------
void Services::TraceHorizontalSequence(int from, int to, const char* heading){
	ArrayOfIntegers AI;
	//if (heading != nullptr)WriteLine("%s", heading);
	//else WriteLine("      ");
	AI.CreateWithSequence(to - from + 1, from);
	Str sz;
	sz = AI.ReportHorizontal();
	if (heading != nullptr)WriteLine("%s%s", heading, sz.GetString());
	else WriteLine("%s", sz.GetString());
}										//TraceHorizontalSequence
//--------------------------------------------------------------------
void Services::Trace(char c){
	String ^ ss = gcnew String(&c);
	Debug::WriteLine(ss);
}										//Trace
//--------------------------------------------------------------------
void Services::Trace(char c, const char * heading){
	Str sa(1);
	sa[0] = c;
	Str sz;
	if(heading == nullptr)sz = sa;
	else sz.Format("%s %s", heading, sa.GetString());
	TraceStr(sz);
}										//Trace
//--------------------------------------------------------------------
void Services::Trace(int i, const char * heading){
	Str sz;
	if(heading == nullptr)sz.Format("%d", i);
	else                  sz.Format("%s%d", heading, i);
	TraceStr(sz);
}
//--------------------------------------------------------------------
void Services::TraceStr(const Str & sz, const char * heading){
//Note TRACE is limited to 512 characters, including the terminating nullptr.
	const char * p = sz.GetString();
	if(heading == nullptr)WriteLine("%s", p);
	else WriteLine("%s%s\n", heading, p);
}										//TraceStr
//--------------------------------------------------------------------
void Services::TraceStr(const Str & sz, int from, int to, const char * heading){
//both included
	const char * mainstring = sz.GetString();
	int nchars = to - from + 1;
	CStr cc(mainstring, from, nchars);
	TraceCStr(cc, heading);
}										//TraceStr
//--------------------------------------------------------------------
void Services::TraceCStr(const CStr & sz, const char * heading){
//Note TRACE is limited to 512 characters, including the terminating nullptr.
	int n = sz.GetNumberCharacters();
	const char * p = sz.GetRangeString();
	char * cc = new char[n + 1];
	memcpy(cc, p, n);
	cc[n] = '\0';
	if(heading == nullptr){
		WriteLine("%s", cc);
	}
	else{
		WriteLine("%s%s", heading, cc);
	}
	delete [] cc;
	cc = nullptr;
}										//TraceCStr
//--------------------------------------------------------------------
void Services::Trace(const char * p, const char * heading){
	if(heading == nullptr)WriteLine("%s", p);
	else WriteLine("%s%s\n", heading, p);
}										//Trace
//--------------------------------------------------------------------
void Services::WriteLine(const char * format, ...){
	Debug::Assert(format != nullptr);
	Formatter F;
	va_list argList;
	va_start(argList, format);
	va_list argListSave = argList;
	int L = F.EstimateLengthOfFormattedString(format, argList);
	va_end(argList);

	char * pc = new char[L + 1];
	int n = vsprintf_s(pc, L + 1, format, argListSave);
	Debug::Assert(n >= 0);
	va_end(argListSave);
	String ^ ss = gcnew String(pc);
	Debug::WriteLine(ss);
	delete [] pc;
	pc = nullptr;
}										//WriteLine
//--------------------------------------------------------------------
void Services::TraceByLine(const Str & sz, const char * heading){
	ArrayOfCStr Lines;
	//int n = sz.SeparateIntoLines(Lines);
	int n = Lines.CreateBySeparatingIntoLines(sz);
	if(heading != nullptr){
		WriteLine("%s", heading);
	}
	Str sa;
	for(int i = 0; i < n; i++){
		sa.Create(Lines[i]);
		//The effect of SeparateIntoLines is to strip all \n. For proper
		//display by TRACE, the \n must be re-inserted.
		WriteLine("%s", sa.GetString());
	}
}										//TraceByLine
//--------------------------------------------------------------------
void Services::TraceByLine(const CStr & sz, const char * heading){
	ArrayOfCStr Lines;
	//int n = sz.SeparateIntoLines(Lines);
	int n = Lines.CreateBySeparatingIntoLines(sz);
	if(heading != nullptr){
		WriteLine("%s", heading);
	}
	Str sa;
	for(int i = 0; i < n; i++){
		sa.Create(Lines[i]);
		WriteLine("%s", sa.GetString());
	}
}										//TraceByLine
//--------------------------------------------------------------------
void Services::TraceMatrix(const PMatrix & m, const Str & title, const char * format){
//Example of format: "%12.4g" or "13.6g"
	WriteLine(title.GetString());
	int nR = m.GetNumberRows();
	int nC = m.GetNumberCols();
	WriteLine("  %d X %d", nR, nC);
	Str sz;
	for(int row = 0; row < nR; row++){
		sz.Format("%2d ", row);
		WriteLine(sz.GetString());
		if(format != nullptr){
			TraceDoubleArrayHorizontally(nC, m.GetRow(row), "", format);
		}
		else{
			TraceDoubleArrayHorizontally(nC, m.GetRow(row), "", "13.6g");
		}
	}
}										//TraceMatrix
//--------------------------------------------------------------------
void Services::TraceDoubleArray(int n, const double * array){
	if(n <= 0){
		WriteLine("\nThe double array has no elements,");
		return;
	}
	Str sa;
	for(int i = 0; i < n; i++){
		sa.Format("%5d  %.10g\n", i, array[i]);
		WriteLine(sa.GetString());
	}
}										//TraceDoubleArray
//--------------------------------------------------------------------
void Services::TraceDoubleArrayHorizontally(int n, const double * array, const char * title, const char * format){
	if(title == nullptr)title = "";
	if(format == nullptr)format = "%13.6g";
	if(n > 60){
		WriteLine("Array %s has %d elements, too large.", title, n);
		return;
	}
	WriteLine("%s  ", title);
	for(int i = 0; i < n; i++){
		//"g" format needs 7 extra spaces for: sign, decimal period,
		//letter "e", exponent sign, and 3 spaces for the exponent.
		Str sa;
		sa.Format(format, array[i]);
		WriteLine(sa.GetString());
	}
	WriteLine("");
}										//TraceDoubleArrayHorizontally
//--------------------------------------------------------------------
void Services::TraceIntegerArrayHorizontally(int n, const int * array, const char * title, const char * format){
	if(title == nullptr)title = "";
	if(format == nullptr)format = "%4d";
	if(n > 100){
		WriteLine("Array %s has %d elements, too large.", title, n);
		return;
	}
	WriteLine("%s  ", title);
	for(int i = 0; i < n; i++){
		//"d" format needs 1 extra spaces for sign, if any.
		//letter "e", exponent sign, and 3 spaces for the exponent.
		Str sa;
		sa.Format(format, array[i]);
		WriteLine(sa.GetString());
	}
	WriteLine("");
}										//TraceIntegerArrayHorizontally
//--------------------------------------------------------------------
bool Services::ReadFileIntoString(const char * path, Str & text){
//Reads a file byte by byte into an Str string.
	if(path == nullptr){
		WriteLine("Services::ReadFileIntoString. The path is nullptr");
		return false;
	}
	int L = strlen(path);
	if(L < 1){
		WriteLine("Services::ReadFileIntoString. The path is empty");
		return false;
	}
	FILE * file = Services::OpenFileForReading(path);
	bool bb = ReadFileIntoString(file, text);
	fclose(file);
	if(!bb)WriteLine("Services::ReadFileIntoString. Cannot read the file. Path=%s", path);
	return bb;
}										//ReadFileIntoString
//--------------------------------------------------------------------
bool Services::ReadFileIntoString(FILE * file, Str & text){
//Reads a file byte by byte into an Str string.
	if(file == nullptr){
		WriteLine("Services::ReadFileIntoString. The path is nullptr");
		return false;
	}

	//Measure the number of characters in the file.
	rewind(file);
	long L = 0; //number of chars in the file.
	int k;
	while((k = getc(file)) && k != EOF){
		++L;
	}
	if(k != EOF || L < 0){
		WriteLine("Services::ReadFileIntoStringThe file is empty or bad");
		return false;
	}

	//Read the file into the Str.
	rewind(file);
	bool bb = text.SetSize(L); //sets 0 to L-1 for the L chars, and text[L] to '\n'.
	if(!bb)return false;
	int count = 0;
	while((k = getc(file)) && k != EOF){
		text.SetAt(count++, (char)k);
	}
	if(k != EOF || count != L)return false;
	return true;
}										//ReadFileIntoString
//--------------------------------------------------------------------
bool Services::SelectFileFromDialog(Str & filename, const char * initialDirectory, const char * filter, const char * title){ //This works but has problems.
	OpenFileDialog ^ dlg = gcnew OpenFileDialog;
	dlg->InitialDirectory = gcnew String(initialDirectory); //Example: "C:\\AMine\\MSDev\\projects\\CppRefactoring2\\DataFiles";
	dlg->Filter = gcnew String(filter); //Example: "RMC Definition (*.txt)|*.txt|All Files (*.*)|*.*";
	dlg->FilterIndex = 1;  //Nbr. 1 filter option initially selected: "AF Movie".
	dlg->RestoreDirectory = true;
	dlg->Title = gcnew String(title);  //Example: "Select an example";
	if(dlg->ShowDialog() != ::DialogResult::OK)return false;
	String ^ fn = dlg->FileName;
	filename.Create(fn);
	return true;
}										//SelectFileFromDialog
//--------------------------------------------------------------------
bool Services::SelectFileFromDialogNew(Str & filename){ //This runs too slow
	using namespace System::IO;
	//Note: SetCurrentDirectory does not allow using the parameters list. Instead, better set the 
	//current directory in the calling method, before calling this one. For now, I hardcode it:
	Directory::SetCurrentDirectory("C:\\AMine\\MSDev\\projects\\CppRefactoring5\\CausalSetExamples\\"); //Sets the system's current directory.
	OpenFileDialog dlg;
	dlg.ShowDialog();
	dlg.OpenFile(); //Opens the file selected by the user with read-only permission, as specified in the FileName property. 
	const String ^ pp = dlg.FileName;
	filename.Create(pp);
	return true;
}										//SelectFileFromDialogNew
//--------------------------------------------------------------------
void Services::SeedRandom(){
//Seed rand once with the time so the numbers will be different each time we run.
	//Find a seed that has not been used before.
	//Range of unisgned int: 0 to UINT_MAX=65535.
	unsigned int currenttime = (unsigned int)time(nullptr);
	unsigned int newtime = currenttime;
	while(newtime == currenttime){
		newtime = (unsigned int)time(nullptr);
	}
	//Use the unique seed to seed the random number generator.
	srand(newtime);  // <== takes an  unsigned int
}										//SeedRandom
//--------------------------------------------------------------------
double Services::GetRandomInRange(double min, double max){
//Returns a double in the range (min, max), both included. (not very good, probability smaller at ends).
	Debug::Assert(min < max);
	//rand() returns integers in the range (0, RAND_MAX), both included.  RAND_MAX = 0x7fff = 32767.
	double r = (max - min) * rand() / RAND_MAX + min;
	if(r < min)r = min;
	if(r > max)r = max;
	return r;
}										//GetRandomInRange
//--------------------------------------------------------------------
int Services::GetRandomInRange(int min, int max){
//Returns an integer in the range (min, max), both included, with a uniform probability distribution.
	if(max == min)return min;
	Debug::Assert(max > min);
	int n = max - min + 1;
	//rand() returns integers in the range (0, RAND_MAX), both included.  RAND_MAX = 0x7fff = 32767.
	int r = rand();
	Debug::Assert(0 <= r && r <= RAND_MAX);
	int s = int(double(r) * double(n) / double(RAND_MAX)) + min;
	if(r == RAND_MAX)s = max;
	Debug::Assert(min <= s && s <= max);
	return s;
//	double r = GetRandomInRange((double)min, (double)max);
//	return (int)(r + 0.5);
}										//GetRandomInRange
//--------------------------------------------------------------------
bool Services::TestDouble(double x){
	int cl = _fpclass(x);
	if(cl == _FPCLASS_SNAN){  //Not a Number (NaN).
		return false;
	}
	if(cl == _FPCLASS_QNAN){  //Quite NaN 
		return false;
	}
	if(cl == _FPCLASS_NINF){  //Negative Infinity.
		return false;
	}
	if(cl == _FPCLASS_ND){    //Negative Denormalized.
		return false;
	}
	if(cl == _FPCLASS_PD){    //Positive Denormalized.
		return false;
	}
	if(cl == _FPCLASS_PINF){  //Positive Infinity.
		return false;
	}

	//NN=Negative Normalized Nonzero  PN=Positive Normalized Nonzero
	//NZ=Negative Zero  PZ=Postive Zero.
	Debug::Assert(cl == _FPCLASS_NN || cl == _FPCLASS_NZ || cl == _FPCLASS_PZ || cl == _FPCLASS_PN);
	return true;
}										//TestDouble
//--------------------------------------------------------------------
int Services::GetPrimaryKey(){
	static int PK = 1;
	return PK++;
}
//--------------------------------------------------------------------

