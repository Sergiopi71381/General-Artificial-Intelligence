#pragma once

//This class provides basic services.
#include <stdio.h>
class Str;
class CStr;
class PMatrix;
//--------------------------------------------------------------------
class Services{
public:
//Updated stuff.
	static const double AccelerationOfGravity;
	static const double DegToRadians;
	static const double pi;
	static int Factorial(int N);
	Services();
	virtual ~Services();
//File operations. A Text File consists of strings separated by '\n'.
	static FILE * OpenFileForReading(const char * path);
	static FILE * OpenFileForWriting(const char * path);
	static FILE * OpenFileForAppending(const char * path);
	static void SaveToFile(const Str & ss, const char * path);
	static void SaveToFile(const Str & ss, FILE * file);
	static void AppendFileToFile(const Str & pathA, const Str & pathB);
	static bool ReadFileIntoString(const char * path, Str & text);
	static bool ReadFileIntoString(FILE * file, Str & text);
	static bool SelectFileFromDialog(Str & filename, const char * initialDirectory, const char * filter, const char * title); //This works but has problems.
	static bool SelectFileFromDialogNew(Str & filename); //This works but is very slow. 
//Trace and others.
	static void WriteLine(const char * format, ...);
	static bool TestDouble(double x);
	static void TraceHorizontalSequence(int from, int to, const char* heading = nullptr);
	static void TraceStr(const Str & sz, const char * heading = nullptr);
	static void TraceStr(const Str & sz, int from, int to, const char * heading = nullptr); //both included
	static void TraceCStr(const CStr & sz, const char * heading = nullptr);
	static void Trace(char c);
	static void Trace(char c, const char * heading = nullptr);
	static void Trace(int i, const char * heading = nullptr);
	static void Trace(const char * p, const char * heading = nullptr);
	static void TraceByLine(const Str & sz, const char * heading = nullptr);
	static void TraceByLine(const CStr & sz, const char * heading = nullptr);
	static void TraceMatrix(const PMatrix & m, const Str & title, const char * format = nullptr);
	static Str cnv(const Str & ss);
	static Str cnv(const CStr & ss);
	static void TraceDoubleArray(int n, const double * array);
	static void TraceDoubleArrayHorizontally(int n, const double * array, const char * title = "", const char * format = "%13.6g");
	static void TraceIntegerArrayHorizontally(int n, const int * array, const char * title = "", const char * format = "%4d");
//Random numbers.
	static int GetPrimaryKey();
	static void SeedRandom();
	static double GetRandomInRange(double min, double max);
	static int    GetRandomInRange(int min, int max);
};
//--------------------------------------------------------------------

