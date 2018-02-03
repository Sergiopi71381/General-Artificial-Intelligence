#include "stdafx.h"
#include "Str.h"
#include "CStr.h"
#include "Formatter.h"
#include "Services.h"
#include <string.h>
#include <math.h>
#include "vcclr.h"  //PtrToStringChars
#include <errno.h>   //defines "errno"
//--------------------------------------------------------------------
Str::Str(){
	m_Size = 0;
	m_pC = new char[1];
	m_pC[0] = '\0';
}										//Default constructor
//--------------------------------------------------------------------
Str::Str(int size){
	Debug::Assert(size >= 0, "01142013,8:04");
	m_Size = size;
	m_pC = new char[m_Size + 1];
	m_pC[m_Size] = '\0';
}										//Size constructor
//--------------------------------------------------------------------
Str::Str(const Str & other){
	m_Size = other.GetLength();
	m_pC = new char[m_Size + 1];
	const char * p = other.GetString();
//	memcpy(m_pC, other.GetString(), m_Size);
	errno_t e = strcpy_s(m_pC, m_Size + 1, p);
	Debug::Assert(!e, "01142013,8:05");
}										//Copy constructor
//--------------------------------------------------------------------
Str::Str(const char * ch){ //ch is a nullptr-terminated string.
	Debug::Assert(ch != nullptr, "01142013,8:06");
	m_Size = strlen(ch);
	m_pC = new char[m_Size + 1];
	//strcpy(m_pC, ch);
	errno_t e = strcpy_s(m_pC, m_Size + 1, ch);
	Debug::Assert(!e, "01142013,8:07");
}										//nullptr-terminated string constructor
//--------------------------------------------------------------------
Str::Str(int n, char ch){
	Debug::Assert(n >= 0, "01142013,8:08");
	m_Size = n;
	m_pC = new char[m_Size + 1];
	for(int i = 0; i < m_Size; ++i){
		m_pC[i] = ch;
	}
	m_pC[m_Size] = '\0';
}										//Repeated character constructor
//--------------------------------------------------------------------
Str::Str(const CStr & cs){
	Debug::Assert(cs.FindChar('\0') == -1, "01142013,8:09");
	m_Size = cs.GetNumberCharacters();
	m_pC = new char[m_Size + 1];
	memcpy(m_pC, cs.GetRangeString(), m_Size);
	m_pC[m_Size] = '\0';
}										//CStr constructor
//--------------------------------------------------------------------
Str::Str(const char * ch, int start, int nChars) : m_pC(nullptr){//nChars characters starting at start.
	Create(ch, start, nChars);
}										//Constructor from a substring
//--------------------------------------------------------------------
Str::Str(const char * types, const char * ch, ...){
//"ch" can be followed by 0 or more arguments of type either "nullptr-terminated const char *" or "const Str *".
//"types" is a char array such as "sSsCSSs", where  s=const char *   S=const Str *   C=const CStr *.
//The named argument "ch" is actually the first argument considered by va_list, va_start to
//be part of the variable list of arguments. This avoids confusion with the nullptr-terminated
//string constructor.
//IMPORTANT!!  Do not pass Str or CStr objects directly, pass their addresses  &Str, &CStr !!!
	Debug::Assert(types != nullptr && types[0] == 's', "01142013,8:10");

	//Read all arguments and determine the total length.
    va_list vl;
	va_start(vl, types);
	m_Size = vLength(vl, types);
    va_end(vl);

	//Do the actual concatenation.
	m_pC = new char[m_Size + 1];
    va_start(vl, types);
	vCat(vl, types);
	va_end(vl);
}										//Concatenation constructor
//--------------------------------------------------------------------
Str::Str(const char * types, const Str * st, ...){
	Debug::Assert(types != nullptr && types[0] == 'S', "01142013,8:11");

	//Read all arguments and determine the total length.
    va_list vl;
	va_start(vl, types);
	m_Size = vLength(vl, types);
    va_end(vl);

	//Do the actual concatenation.
	m_pC = new char[m_Size + 1];
    va_start(vl, types);
	vCat(vl, types);
	va_end(vl);
}										//Concatenation constructor
//--------------------------------------------------------------------
//Str::Str(BSTR bs){
//	m_pC = _com_util::ConvertBSTRToString(bs); //Creates an array of char in memory!
//	m_Size = strlen(m_pC);
//}										//BSTR constructor
//--------------------------------------------------------------------
Str::Str(const System::String ^ ss){
	pin_ptr<const wchar_t> wch = PtrToStringChars(ss);
	m_Size = wcslen(wch);
	size_t convertedChars = 0;
	m_pC = new char[m_Size + 1];
	errno_t e = wcstombs_s(&convertedChars, m_pC, m_Size + 1, wch, _TRUNCATE);
	Debug::Assert(!e && convertedChars == (m_Size + 1), "01142013,8:12"); //Make sure there has been no truncation.
}										//System::String constructor
//--------------------------------------------------------------------
void Str::vCat(va_list & vl, const char * types){
//Used by constructors to concatenate the char arrays from a variable list of arguments.
	int nStrings = strlen(types);
	Debug::Assert(nStrings > 0, "01142013,8:13");
	const char * p = m_pC;
	for(int j = 0; j < nStrings; j++){
		const char * cc;
		int L;
		if(types[j] == 's'){  //const char *
			cc = (const char *)va_arg(vl, const char *); // Return from va_arg requires explicit cast!!!
			L = strlen(cc);
		}
		else if(types[j] == 'S'){ //const Str *
			const Str * qq = (const Str *)va_arg(vl, const Str *);  // Return from va_arg requires explicit cast!!!
			cc = qq->GetString();
			L = qq->GetLength();
		}
		else if(types[j] == 'C'){ //const CStr *
			const CStr * cq = (const CStr *)va_arg(vl, const CStr *);
			cc = cq->GetMainString() + cq->GetOffset();
			L = cq->GetNumberCharacters();
		}
		else{
			Debug::Assert(false, "01142013,8:14");
		}
		memcpy((void *)p, cc, L);
		p += L;
	}
	m_pC[m_Size] = '\0';
}										//vCat
//--------------------------------------------------------------------
int Str::vLength(va_list & vl, const char * types){
//Used by constructors to calculate the length of char arrays in a variable list of arguments.
	int nStrings = strlen(types);
	Debug::Assert(nStrings > 0, "01142013,8:15");
	int L = 0;
	for(int j = 0; j < nStrings; j++){
		if(types[j] == 's'){  //const char *
			const char * cc = (const char *)va_arg(vl, const char *); // Return from va_arg requires explicit cast!!!
			L += strlen(cc);
		}
		else if(types[j] == 'S'){ //const Str *
			const Str * qq = (const Str *)va_arg(vl, const Str *);
			Debug::Assert(qq->Verify(), "01142013,8:16");
			L += qq->GetLength();
		}
		else if(types[j] == 'C'){ //const CStr *
			const CStr * cq = (const CStr *)va_arg(vl, const CStr *);
			Debug::Assert(cq->Verify(), "01142013,8:17");
			L += cq->GetNumberCharacters();
		}
		else{
			Debug::Assert(false, "01142013,8:18");
		}
	}
	return L;
}										//vLength
//--------------------------------------------------------------------
Str::~Str(){
	delete [] m_pC;
	m_pC = nullptr;
}										//Destructor
//--------------------------------------------------------------------
bool Str::SetSize(int size){
	Debug::Assert(size >= 0, "01142013,8:19");
	if(size < 0)return false;
	m_Size = size;
	delete [] m_pC;
	m_pC = new char[m_Size + 1];
	m_pC[m_Size] = '\0';
	return true;
}										//SetSize
//--------------------------------------------------------------------
bool Str::Create(const char * ch){ //ch is a null-terminated string.
	Debug::Assert(ch != nullptr, "01142013,8:20");
	if(ch == nullptr)return false;
	m_Size = strlen(ch);
	delete [] m_pC;
	m_pC = new char[m_Size + 1]; //large enough for the source string, including the terminating null.
	//strcpy(m_pC, ch);
	errno_t e = strcpy_s(m_pC, m_Size + 1, ch);
	Debug::Assert(!e, "01142013,8:21");
	return true;
}										//Create
//--------------------------------------------------------------------
bool Str::Create(const char * ch, int size){ //Disregards null teminator, if any.
	Debug::Assert(ch != nullptr && size >= 0, "01142013,8:22");
	if(ch == nullptr || size < 0)return false;
	m_Size = size;
	delete [] m_pC;
	m_pC = new char[m_Size + 1];
	for(int i = 0; i < m_Size; i++){
		Debug::Assert(ch[i] != '\0', "01142013,8:24"); //No '\0' allowed inside string.
		m_pC[i] = ch[i];
	}
	m_pC[m_Size] = '\0';
	return true;
}										//Create
//--------------------------------------------------------------------
bool Str::Create(const char * ch, int start, int nChars){ //Use nChars characters from ch starting at start.
	Debug::Assert(ch != nullptr && start >= 0 && nChars >= 0, "01142013,8:25");
	if(ch == nullptr || start < 0 || nChars < 0)return false;
	m_Size = nChars; //could be 0
	delete [] m_pC;
	m_pC = new char[m_Size + 1];
	for(int i = 0; i < m_Size; i++){
		Debug::Assert(ch[i + start] != '\0', "01142013,8:26"); //No '\0' allowed inside string.
		if(!ch[i + start])return false;
		m_pC[i] = ch[i + start];
	}
	m_pC[m_Size] = '\0';
	return true;
}										//Create
//--------------------------------------------------------------------
void Str::RightJustifyInFieldOfSize(int size) { //this Str is right-justified or left-trimmed into a field of size "size"
	if(size == m_Size)return; //leave as-is
	Debug::Assert(size >= 1);	
	char * ss = new char[size + 1];
	if (size > m_Size) {//pad with spaces from  0  to  size - m_Size - 1, both included.
		for (int i = 0; i < (size - m_Size); ++i) {
			ss[i] = ' ';
		}
		for (int i = (size - m_Size); i <= size; ++i) {
			int j = i - (size - m_Size); // j goes from 0 to m_Size, both included, so the terminating '\0' is copied too.
			ss[i] = m_pC[j];
		}
	}
	else { //size < m_Size. Trim this string on the left so it fits in the new, smaller size.
		for (int i = (m_Size - size); i <= m_Size; ++i) {
			int j = i - (m_Size - size); //j goes from 0 to size, both included, so the terminating '\0' is copied too.
			ss[j] = m_pC[i];
		}
	}
	m_Size = size;
	delete m_pC;
	m_pC = ss;
}										//RightJustifyInFieldOfSize
//--------------------------------------------------------------------
bool Str::Create(int n, char ch){
//n times the same character.
	Debug::Assert(n >= 0, "01142013,8:27");
	m_Size = n;  //Could be 0.
	delete [] m_pC;
	m_pC = new char[m_Size + 1];
	for(int i = 0; i < m_Size; ++i){
		m_pC[i] = ch;
	}
	m_pC[m_Size] = '\0';
	return true;
}										//Create
//--------------------------------------------------------------------
bool Str::Create(const CStr & ct){
	m_Size = ct.GetNumberCharacters();
	delete [] m_pC;
	m_pC = new char[m_Size + 1];
	for(int i = 0; i < m_Size; i++){
		m_pC[i] = ct[i];
	}
	m_pC[m_Size] = '\0';
	return true;
}										//Create
//--------------------------------------------------------------------
bool Str::Create(const Str & st){
	return Create(st.GetString());
}										//Create
//--------------------------------------------------------------------
bool Str::Create(int nn){
//Converts the number into characters.
	char buf[12];
	errno_t e = _itoa_s(nn, buf, 12, 10);
	Debug::Assert(errno == 0, "Str.Overflow or other error condition.");
	int len = strlen(buf);
	Debug::Assert(len >= 1, "Str.There must be at least one digit.");
	bool bb = Create(buf);
	return bb;
}										//Create
//--------------------------------------------------------------------
const Str & Str::operator=(const Str & other){
	Create(other.GetString());
	return *this;
}										//operator=
//--------------------------------------------------------------------
const Str & Str::operator=(const char * ch){
	Create(ch);
	return *this;
}										//operator=
//--------------------------------------------------------------------
//const Str & Str::operator=(BSTR bs){
//	delete [] m_pC;
//	m_pC = _com_util::ConvertBSTRToString(bs); //Creates an array of char in memory!
//	m_Size = strlen(m_pC);
//	return *this;
//}										//operator=
//--------------------------------------------------------------------
const Str & Str::operator+=(const Str & other){
	int k2(other.m_Size);
	char * cc = new char[m_Size + k2 + 1];
	memcpy(cc, m_pC, m_Size);
	memcpy(cc + m_Size, other.m_pC, k2);
	m_Size += k2;
	cc[m_Size] = '\0';
	delete [] m_pC;
	m_pC = cc;
	return *this;
}										//operator+=
//--------------------------------------------------------------------
const Str & Str::operator+=(const char * ch){
	int k2(strlen(ch)); //strlen excludes the terminal '\0'.
	char * cc = new char[m_Size + k2 + 1];
	memcpy(cc, m_pC, m_Size);
	memcpy(cc + m_Size, ch, k2);
	m_Size += k2;
	cc[m_Size] = '\0';
	delete [] m_pC;
	m_pC = cc;
	return *this;
}										//operator+=
//--------------------------------------------------------------------
//const Str & Str::operator+=(BSTR bs){
//	char * ch = _com_util::ConvertBSTRToString(bs); //Creates an array of char in memory!
//	operator+=(ch);
//	delete [] ch;
//	return *this;
//}										//operator+=
//--------------------------------------------------------------------
const Str & Str::operator+=(char ch){
	char * cc = new char[m_Size + 2];
	memcpy(cc, m_pC, m_Size);
	cc[m_Size] = ch;
	m_Size += 1;
	cc[m_Size] = '\0';
	delete [] m_pC;
	m_pC = cc;
	return *this;
}										//operator+=
//--------------------------------------------------------------------
Str Str::operator+(char ch)const{
	Str rr(m_Size + 1);
	memcpy(rr.m_pC, m_pC, m_Size);
	rr[m_Size] = ch;
	return rr;
}										//operator+
//--------------------------------------------------------------------
Str Str::operator+(const char * ch)const{
	Debug::Assert(ch != nullptr, "01142013,8:30");
	int k2(strlen(ch)); //strlen excludes the terminal '\0'.
	Str rr(m_Size + k2);
	memcpy(rr.m_pC, m_pC, m_Size);
	memcpy(rr.m_pC + m_Size, ch, k2);
	return rr;
}										//operator+
//--------------------------------------------------------------------
Str Str::operator+(const Str & s)const{
	int k2(s.m_Size);
	Str rr(m_Size + k2);
	memcpy(rr.m_pC, m_pC, m_Size);
	memcpy(rr.m_pC + m_Size, s.m_pC, k2);
	return rr;
}										//operator+
//--------------------------------------------------------------------
//Str Str::operator+(BSTR bs)const{
//	char * ch = _com_util::ConvertBSTRToString(bs); //Creates an array of char in memory!
//	Str rr = operator+(ch);
//	delete [] ch;
//	return rr;
//}										//operator+
//--------------------------------------------------------------------
Str operator+(const char * ch, const Str & s2){ //Friend (non-member).
	Debug::Assert(ch != nullptr, "01142013,8:31");
	int k1(strlen(ch)), k2(s2.m_Size);  //strlen excludes the terminal '\0'.
	Str rr(k1 + k2);
	memcpy(rr.m_pC, ch, k1);
	memcpy(rr.m_pC + k1, s2.m_pC, k2);
	return rr;
}										//operator+
//--------------------------------------------------------------------
//Str operator+(BSTR bs, const Str & s2){         //Friend (non-member)
//	char * ch = _com_util::ConvertBSTRToString(bs); //Creates an array of char in memory!
//	Str rr = operator+(ch, s2);
//	delete [] ch;
//	return rr;
//}										//operator+
//--------------------------------------------------------------------
void Str::TrimRight(){
	char * q = m_pC + m_Size - 1;
	while(q >= m_pC && isspace(*q)){--q;}
	m_Size = q - m_pC + 1;
	char * cc = new char[m_Size + 1];
	errno_t e = strncpy_s(cc, m_Size + 1, m_pC, m_Size);
	Debug::Assert(!e, "01142013,8:32");
	cc[m_Size] = '\0';
	delete [] m_pC;
	m_pC = cc;
}										//TrimRight
//--------------------------------------------------------------------
void Str::TrimLeft(){
	char * p = m_pC;
	while(*p && isspace(*p)){++p;}
	m_Size -= (p - m_pC);
	char * cc = new char[m_Size + 1];
	errno_t e = strncpy_s(cc, m_Size + 1, p, m_Size);
	Debug::Assert(!e, "01142013,8:34");
	cc[m_Size] = '\0';
	delete [] m_pC;
	m_pC = cc;
}										//TrimLeft
//--------------------------------------------------------------------
void Str::Trim(){
	char * q = m_pC + m_Size - 1;
	while(q >= m_pC && isspace(*q)){--q;}
	char * p = m_pC;
	while(p <= q && isspace(*p)){++p;}
	m_Size = q - p + 1;
	char * cc = new char[m_Size + 1];
	errno_t e = strncpy_s(cc, m_Size + 1, p, m_Size);
	Debug::Assert(!e, "01142013,8:35");
	cc[m_Size] = '\0';
	delete [] m_pC;
	m_pC = cc;
}										//Trim
//--------------------------------------------------------------------
void Str::StripLeft(int nChars){
	int m = m_Size - nChars;
	Debug::Assert(m >= 0, "01142013,8:36");
	char * cc = new char[m + 1]; //destination string
	char * p = m_pC + nChars;  //source string
	errno_t e = strncpy_s(cc, m + 1, p, m + 1);
	Debug::Assert(!e, "01142013,8:36");
	m_Size = m;
	delete [] m_pC;
	m_pC = cc;
}										//StripLeft
//--------------------------------------------------------------------
void Str::StripRight(int nChars){
	int m = m_Size - nChars;
	Debug::Assert(m >= 0, "01142013,8:37");
	char * cc = new char[m + 1]; //destination string
	errno_t e = strncpy_s(cc, m + 1, m_pC, m);
	Debug::Assert(!e, "01142013,8:38");
	cc[m] = '\0';
	m_Size = m;
	delete [] m_pC;
	m_pC = cc;
}										//StripRight
//--------------------------------------------------------------------
Str Str::Left(int nChars)const{
	Debug::Assert(nChars >= 0, "01142013,8:39");
	if(nChars > m_Size)nChars = m_Size;
	return Str(m_pC, 0, nChars);
}										//Left
//--------------------------------------------------------------------
Str Str::Right(int nChars)const{
	Debug::Assert(nChars >= 0, "01142013,8:40");
	if(nChars > m_Size)nChars = m_Size;
	return Str(m_pC, m_Size - nChars, nChars);
}										//Right
//--------------------------------------------------------------------
Str Str::Mid(int nStart)const{
	Debug::Assert(nStart >= 0, "01142013,8:41");
	if(nStart > m_Size)nStart = m_Size;
	return Str(m_pC, nStart, m_Size - nStart);
}										//Mid
//--------------------------------------------------------------------
Str Str::Mid(int nStart, int nChars)const{
	Debug::Assert(nStart >= 0 && nChars >= 0, "01142013,8:42");
	Debug::Assert(nStart <= m_Size, "01142013,8:43");
	if(nStart > m_Size)nStart = m_Size;
	Debug::Assert((nStart + nChars) <= m_Size, "01142013,8:44");
	if(nChars > (m_Size - nStart))nChars = m_Size - nStart;
	Str sz(m_pC, nStart, nChars);
	return sz;
//	return Str(m_pC, nStart, nChars);
}										//Mid
//--------------------------------------------------------------------
CStr Str::CLeft(int nChars)const{
	Debug::Assert(nChars >= 0, "01142013,8:45");
	if(nChars > m_Size)nChars = m_Size;
	return CStr(m_pC, 0, nChars);
}										//CLeft
//--------------------------------------------------------------------
CStr Str::CRight(int nChars)const{
	Debug::Assert(nChars >= 0, "01142013,8:46");
	if(nChars > m_Size)nChars = m_Size;
	return CStr(m_pC, m_Size - nChars, nChars);
}										//CRight
//--------------------------------------------------------------------
CStr Str::CMid(int nStart)const{
	Debug::Assert(nStart >= 0, "01142013,8:47");
	if(nStart > m_Size)nStart = m_Size;
	return CStr(m_pC, nStart, m_Size - nStart);
}										//CMid
//--------------------------------------------------------------------
CStr Str::CMid(int nStart, int nChars)const{
	Debug::Assert(nStart >= 0 && nChars >= 0, "01142013,8:48");
	if(nStart > m_Size)nStart = m_Size;
	if(nChars > (m_Size - nStart))nChars = m_Size - nStart;
	return CStr(m_pC, nStart, nChars);
}										//CMid
//--------------------------------------------------------------------
int Str::GetTextBetweenBrackets(int openBracketIndex, CStr & text)const{
//"openBracketIndex" is the index of the open bracket, one of  ( < [ {.
//Returns the index of the matching closing bracket, and the text between 
//them, trimmed left and right. Returns -1 if not found.
	int closeBracketIndex = FindMatchingBracket(openBracketIndex);
	if(closeBracketIndex <= openBracketIndex)return -1;
	text.Create(m_pC, openBracketIndex + 1, closeBracketIndex - openBracketIndex - 1);
	return closeBracketIndex;
}										//GetTextBetweenBrackets
//--------------------------------------------------------------------
int Str::GetTextBetweenBracketsExclusive(char exclude, int openBracketIndex, CStr & text)const{
//"openBracketIndex" is the index of the open bracket, one of  ( < [ {.
//Returns the index of the matching closing bracket that is NOT preceded by 
//character "exclude", and the text between them, trimmed left and right. 
//Returns -1 if none found.
	int closeBracketIndex = FindMatchingBracketExclusive(exclude, openBracketIndex);
	if(closeBracketIndex <= openBracketIndex)return -1;
	text.Create(m_pC, openBracketIndex + 1, closeBracketIndex - openBracketIndex - 1);
	return closeBracketIndex;
}										//GetTextBetweenBracketsExclusive
//--------------------------------------------------------------------
void Str::Insert(int nStart, const char * ch){
	Debug::Assert(nStart >= 0 && nStart < m_Size, "01142013,8:49");
	int n = strlen(ch);
	if(n == 0)return;
	int newSize = m_Size + n;
	char * temp = new char[newSize + 1];
	memcpy(temp, m_pC, nStart);
	memcpy(temp + nStart, ch, n);
	memcpy(temp + nStart + n, m_pC + nStart, m_Size - nStart);
	temp[newSize] = '\0';
	m_Size = newSize;
	delete [] m_pC;
	m_pC = temp;
}										//Insert
//--------------------------------------------------------------------
void Str::Remove(int nStart, int nChars){
	Debug::Assert(nStart >= 0 && nStart < m_Size && nChars >= 0 && nStart + nChars <= m_Size, "01142013,8:50");
	if(nChars == 0)return;
	int newSize = m_Size - nChars;
	char * temp = new char[newSize];
	memcpy(temp, m_pC, nStart);               // <==  USE THE NEW SECURE VERSION  memcpy_s!!!  See Overwrite, below.
	memcpy(temp + nStart, m_pC + nStart + nChars, newSize - nStart);
	temp[newSize] = '\0';
	m_Size = newSize;
	delete [] m_pC;
	m_pC = temp;
}										//Remove
//--------------------------------------------------------------------
void Str::Overwrite(const char * text, char justify){
//Without changing the size of this Str, overwrites the given text with 'L', 'R', or 'C' 
//justification, and fills the rest with spaces, or truncates text if it is too long to fit.

	//Nothing to overwrite.
	int K = strlen(text);
	if(K < 1)return;

	//Too long to justify. Just overwrite.
	if(K >= m_Size){
		errno_t e = memcpy_s(m_pC, m_Size, text, m_Size);
		Debug::Assert(!e, "01142013,8:51");
		return;
	}

	//Case 0 < K < m_Size. Must justify.
	if(justify == 'L'){
		int M = m_Size - K;
		errno_t e = memcpy_s(m_pC, m_Size, text, K);
		Debug::Assert(!e, "01142013,8:52");
		for(int j = K; j < m_Size; ++j)m_pC[j] = ' ';
		return;
	}
	else if(justify == 'R'){
		int M = m_Size - K;
		errno_t e = memcpy_s(m_pC + M, K, text, K);
		Debug::Assert(!e, "01142013,8:53");
		for(int j = 0; j < M; ++j)m_pC[j] = ' ';
		return;
	}
	else if(justify == 'C'){
		int M = (m_Size - K) / 2;
		errno_t e = memcpy_s(m_pC + M, m_Size - M, text, K);
		for(int j = 0; j < M; ++ j)m_pC[j] = ' ';
		for(int j = M + K; j < m_Size; ++j)m_pC[j] = ' ';
		return;
	}
	else{
		Debug::Assert(false, "01142013,8:54");
	}
}										//Overwrite
//--------------------------------------------------------------------
void Str::CompactSpaces(){
//Tabs and multiple spaces are converted to a single space, and the Str is trimmed both sides.
	Debug::Assert(Verify(), "01142013,8:55");
	char * temp = new char[m_Size + 1];
	int j = 0;
	bool previousSpace = true;
	for(int i = 0; i < m_Size; ++i){
		char cc = m_pC[i];
		int space = isspace(cc);
		if(space){
			if(!previousSpace){
				temp[j++] = ' ';
				previousSpace = true;
			}
		}
		else{
			temp[j++] = cc;
			previousSpace = false;
		}
	}
	if(j > 0 && temp[j] == ' '){
		--j;
	}
	temp[j] = '\0';
	delete [] m_pC;
	m_pC = temp;
	m_Size = j;
	Debug::Assert(Verify(), "01142013,8:56");
}										//CompactSpaces
//--------------------------------------------------------------------
void Str::LeftJustifyLines(){
//Eliminates blank lines and leading spaces and tabs from non-blank lines.
//A "line" starts at the beginning of m_pC or after each '\n' character,
//and ends at the following '\n' character or at the terminating '\0'.
//For efficiency, array m_pC is left with its original size.
	Debug::Assert(m_pC[m_Size] == '\0', "01142013,8:57");
	int ia = -1; //The accepted part of the string.
	int ib = 0;  // ib > ia.
	char cc;
	while(ib < m_Size){
		//Set ib at first non-space and non-tab character in current line.
		while((cc = m_pC[ib]) == ' ' || cc == '\t'){++ib;}
		//Accept all characters until '\0' or '\n' found.
		while((cc = m_pC[ib]) && cc != '\n'){
			m_pC[++ia] =  cc;
			++ib;
		}
		//Accept character '\n' if one has been found and is not repeated.
		if(cc == '\n'){
			if(ia >= 0 && m_pC[ia] != '\n')m_pC[++ia] = '\n';
			++ib;
		}
	}
	m_Size = ia + 1;
	m_pC[m_Size] = '\0';
}										//LeftJustifyLines
//--------------------------------------------------------------------
void Str::JustifyLines(){
//Eliminates blank lines and leading and trailing spaces and tabs from non-blank lines.
//A "line" starts at the beginning of m_pC or after each '\n' character,
//and ends at the following '\n' character or at the terminating '\0'.
//For efficiency, array m_pC is left with its original size.
	Debug::Assert(m_pC[m_Size] == '\0', "01142013,8:58");
	int ia = -1; //The accepted part of the string.
	int ib = 0;  // ib > ia.
	char cc, cc1;
	while(ib < m_Size){
		//Set ib at first non-space and non-tab character in current line.
		while((cc = m_pC[ib]) == ' ' || cc == '\t'){++ib;}
		//Accept all characters until '\0' or '\n' found.
		while((cc = m_pC[ib]) && cc != '\n'){
			m_pC[++ia] =  cc;
			++ib;
		}
		//Remove spaces and tabs at end of line.
		while((cc1 = m_pC[ia]) == ' ' || cc1 == '\t'){--ia;}
		//Accept character '\n' if one has been found and is not repeated.
		if(cc == '\n'){
			if(ia >= 0 && m_pC[ia] != '\n')m_pC[++ia] = '\n';
			++ib;
		}
	}
	m_Size = ia + 1;
	m_pC[m_Size] = '\0';
}										//JustifyLines
//--------------------------------------------------------------------
void Str::RemoveBlankLines(){
//A blank line is one that is empty or contains only isspace characters.
//For efficiency, array m_pC is left with its original size.

	if(m_Size == 0)return;
	Debug::Assert(*m_pC != '\0', "01142013,8:59");

	//The accepted part of the string is left of index.
	int index = 0;
	const char * pa;
	const char * pb = m_pC;
	Str sz;
	while(*pb){
		//Put pa at the next non-'\n' or '\0\.
		pa = pb;
		while(*pa && *pa == '\n'){++pa;}
		if(*pa == '\0'){
			m_Size = index - 1; //there is always a '\n' at the end, but do not include it.
			m_pC[m_Size] = '\0';
			break;
		}

		//Put pb at the next '\n' or '\0' after pa.
		pb = pa;
		while(*pb && *pb != '\n'){++pb;}
		Debug::Assert(*pa != '\n' && *pa != '\0', "01142013,8:60");
		Debug::Assert(*pb == '\n' || *pb == '\0', "01142013,8:61");

		//Look for any non-space chars in range pa included to pb not included.
		bool isBlank = true;
		int k = pb - pa;
		Debug::Assert(k > 0, "01142013,8:62");
		int offset = pa - m_pC;
		for(int i = 0; i < k; ++i){ //the terminating \n or \0 is not considered
			char c = GetAt(offset + i);
			if(!isspace(c))isBlank = false; //space, formfeed, newline, carriage return,tab, vertical tab.
		}

		//Accept this line only if it is not blank.
		if(!isBlank){
			for(int i = 0; i <= k; ++i){ //this time, the terminating \n or \0 is copied
				char c0 = m_pC[offset + k - 6];
				char c1 = m_pC[offset + k - 5];
				char c2 = m_pC[offset + k - 4];
				char c3 = m_pC[offset + k - 3];
				char c4 = m_pC[offset + k - 2];
				char c5 = m_pC[offset + k - 1];

//				bool bb = IsInRange(offset + i);
//				if(!bb){
//					int jjdc = 8;
//				}
//				if(i == k){
//					int riit = 9;
//				}

				char c = m_pC[offset + i];
				if(i == k)Debug::Assert(c == '\n' || c == '\0', "01142013,8:64");
//				if(i == k && c != '\n' && c != '\0'){
//					int uguaa = 6;
//				}
				SetAt(index++, c);
			}
		}
		char cc = m_pC[index - 1];
		Debug::Assert(cc == '\n' || cc == '\0', "01142013,8:65"); //there is always a '\n' or a '\0' just before position "index".
	}
}										//RemoveBlankLines
//--------------------------------------------------------------------
void Str::RemoveComments(const char * commentMark){
//A comment is text starting with "commentMark" and ending just before the first '\n' or '\0'.
//For efficiency, array m_pC is left with its original size.
	Debug::Assert(commentMark != nullptr && m_pC[m_Size] == '\0', "01142013,8:66");
	if(commentMark == nullptr)return;
	int L = strlen(commentMark);
	int m = m_Size - L;
	if(m < 0)return; //the line is shorter than the comment mark
	Debug::Assert(L >= 1, "01142013,8:67");
	int ia = -1; //The accepted part of the string.
	int ib = 0;  // ib > ia.

	while(ib <= m){
		//Accept all characters until "commentMark" found, or too close to end.
		while(ib <= m && strncmp(m_pC + ib, commentMark, L) != 0){
			m_pC[++ia] = m_pC[ib++];
		}
		//At this point:
		//either ib  = m + 1  AND there is no comment mark starting at m
		//or     ib <= m      AND a comment mark starts at ib.
		Debug::Assert(ib <= (m + 1), "01142013,8:68");
		if(ib == (m + 1)){
			//Accept the remaining characters, if any.
			while(ib < m_Size){m_pC[++ia] = m_pC[ib++];}
		}
		else{
			//Skip all characters until next '\n' or '\0'.
			while(ib < m_Size && m_pC[ib] != '\n'){++ib;}
			Debug::Assert(m_pC[ib] == '\n' || m_pC[ib] == '\0', "01142013,8:69");
			//Remove commented blank line.
			if((ia == -1 || m_pC[ia] == '\n') && m_pC[ib] == '\n'){++ib;}
		}
	}
	Debug::Assert(ia < m_Size, "01142013,8:70");
	m_Size = ia + 1;
	m_pC[m_Size] = '\0';
}										//RemoveComments
//--------------------------------------------------------------------
void Str::RemoveBetweenMarks(const char * startMark, const char * endMark){
//Given are two marks such as "/*" and "*/". All text between pairs of marks, including the
//marks themselves, is removed. For efficiency, array m_pC is left with its original size.
	Debug::Assert(startMark != nullptr && endMark != nullptr, "01142013,8:71");
	int ia = -1; //The accepted part of the string.
	int ib = 0;  // ib > ia.
	int LA = strlen(startMark);
	int LB = strlen(endMark);
	while(ib < m_Size){
		int ka = Find(startMark, ib);
		if(ka == -1)ka = m_Size;
		for(int i = ib; i < ka; ++i){m_pC[++ia] = m_pC[i];}
		
		int kb = Find(endMark, ka + LA);
		if(kb == -1)break; //"endMark" missing, remove all text to the end.
		ib = kb + LB;
	}	
	Debug::Assert(ia < m_Size, "01142013,8:72");
	m_Size = ia + 1;
	m_pC[m_Size] = '\0';
}										//RemoveBetweenMarks
//--------------------------------------------------------------------
void Str::SpliceContinuationLines(){
//Any line ending with '\' is continued with the following line by 
//eliminating the '\' and the terminating '\n' character.

	int ia = -1; //The accepted part of the string.
	int ib = 0;  // ib > ia.
	while(ib < m_Size){
		int ka = Find("\\\n", ib);
		if(ka == -1)ka = m_Size;
		for(int i = ib; i < ka; ++i){m_pC[++ia] = m_pC[i];}
		ib = ka + 2;
	}
	Debug::Assert(ia < m_Size, "01142013,8:73");
	m_Size = ia + 1;
	m_pC[m_Size] = '\0';
}										//SpliceContinuationLines
//--------------------------------------------------------------------
/*
int Str::SeparateIntoLines(ArrayOfCStr & Lines)const{
//A line starts at the beginning of m_pC or just after each '\n' character.
//The line ends just before the following '\n' character or terminating '\0'.
//Returns the number of lines and the array "Lines" of CStr objects.
//The CStr objects contain no '\n' characters.
	int nLines = CountLines();
	Debug::Assert(nLines >= 0, "01142013,8:74");
	Lines.SetArraySize(nLines);
	if(nLines == 0)return 0; 
	const char * pb = m_pC;
	int line = 0;
	while(true){
		const char * pa = pb;
		//Put pb at the next '\n' or '\0'.
		while(*pb && *pb != '\n'){++pb;}
		//A line has been found.
		Debug::Assert(line < nLines, "01142013,8:75");
		Lines[line].Create(m_pC, pa - m_pC, pb - pa);
		++line;
		if(*pb == '\0')break;
		//Skip the '\n'.
		Debug::Assert(*pb == '\n', "01142013,8:76");
		++pb;
	}
	Debug::Assert(line == nLines, "01142013,8:77");
	return nLines;
}										//SeparateIntoLines
*/
//--------------------------------------------------------------------
/*
int Str::SeparateIntoArray(ArrayOfStr & Lines)const{
//A line starts at the beginning of m_pC or just after each '\n' character.
//The line ends just before the following '\n' character or terminating '\0'.
//Returns the number of lines and the array "Lines" of CStr objects.
//The CStr objects contain no '\n' characters.
	int nLines = CountLines();
	Debug::Assert(nLines >= 0, "01142013,8:78");
	Lines.SetArraySize(nLines);
	if(nLines == 0)return 0;
	const char * pb = m_pC;
	int line = 0;
	while(true){
		const char * pa = pb;
		//Put pb at the next '\n' or '\0'.
		while(*pb && *pb != '\n'){++pb;}
		//A line has been found.
		Debug::Assert(line < nLines, "01142013,8:79");
		bool bb = Lines[line].Create(pa, 0, pb - pa);
		Debug::Assert(bb, "01142013,8:80");
		++line;
		if(*pb == '\0')break;
		//Skip the '\n'.
		Debug::Assert(*pb == '\n', "01142013,8:81");
		++pb;
	}
	Debug::Assert(line == nLines, "01142013,8:82");
	return nLines;
}										//SeparateIntoArray
*/
//--------------------------------------------------------------------
/*
int Str::SeparateIntoNonemptyLines(ArrayOfCStr & Lines)const{
//A line starts at the beginning of m_pC or just after each '\n' character.
//The line ends just before the following '\n' character or terminating '\0'.
//Returns the number of nonempty lines and the array "Lines" of CStr objects.
//The CStr objects contain no '\n' characters.
	int nLines = CountNonemptyLines();
	Lines.SetArraySize(nLines);
	if(nLines == 0)return 0;
	const char * pb = m_pC;
	int line = 0;
	while(true){
		const char * pa = pb;
		//Put pb at the next '\n' or '\0'.
		while(*pb && *pb != '\n'){++pb;}
		if(pb > pa){
			Debug::Assert(line < nLines, "01142013,8:89");
			Lines[line].Create(m_pC, pa - m_pC, pb - pa);
			++line;
		}
		if(*pb == '\0')break;
		//Skip the '\n'.
		Debug::Assert(*pb == '\n', "01142013,8:90");
		++pb;
	}
	Debug::Assert(line == nLines, "01142013,8:91");
	return nLines;
}										//SeparateIntoNonemptyLines
*/
//--------------------------------------------------------------------
/*
int Str::SeparateIntoNonemptyWords(ArrayOfCStr & Words)const{
//A word begins at the beginning of m_pC or just after each group of isspace characters
//(isspace characters are space, formfeed, newline, carriage return, tab, vertical tab).
//The word ends just before the following isspace or terminating '\0' character. 
//Returns the number of non-empty words and the array "Words" of CStr objects.
//The CStr objects contain no isspace characters and are all non-empty.
	int nWords = CountNonemptyWords();
	Words.SetArraySize(nWords);
	if(nWords == 0)return 0;
	const char * pb = m_pC;
	int word = 0;
	while(true){
		//Put pa at the next nonspace character or '\0'.
		const char * pa = pb;
		while(*pa && isspace(*pa)){++pa;}
		if(!(*pa))break;
		// pa is now at a nonspace character. Put pb at the next isspace character or '\0'.
		pb = pa;
		while(*pb && !isspace(*pb)){++pb;}
		//Count the word between pa and pb - 1.
		Debug::Assert(pb > pa && word < nWords, "01142013,8:92");
		Words[word++].Create(m_pC, int(pa - m_pC), int(pb - pa));
		if(!(*pb))break;
	}
	Debug::Assert(word == nWords, "01142013,8:93");
	return nWords;
}										//SeparateIntoNonemptyWords
*/
//--------------------------------------------------------------------
/*
int Str::SeparateIntoWordsUsingSeparator(char separator, ArrayOfCStr & Words)const{
//A word contains no "separator" characters. Returns the number of words. 
//Words are trimmed left and right. Some words may be empty.
	int nWords = CountOccurrences(separator) + 1;
	Words.SetArraySize(nWords);
	const char * pb = m_pC;
	int word = 0;
	while(true){
		//Put pa at beginning of a word
		const char * pa = pb;
		//Put pb at the next "separator" character or just after end of range.
		while(*pb && (*pb) != separator){++pb;}
		//A word has been found. Could be empty.
		if(pb > pa){
			Debug::Assert(word < nWords, "01142013,8:96");
			Words[word].Create(m_pC, pa - m_pC, pb - pa);
			Words[word].Trim();
			++word;
		}
		//Break if at the terminating '\0'.
		if(!(*pb))break;
		//Skip the "separator" character.
		Debug::Assert((*pb) == separator, "01142013,8:97");
		++pb;
	}
	Debug::Assert(word == nWords, "01142013,8:98");
	return nWords;
}										//SeparateIntoWordsUsingSeparator
*/
//--------------------------------------------------------------------
/*
int Str::SeparateIntoWordsUsingSeparator(char separator, ArrayOfStr & Words)const{
//A word contains no "separator" characters. Returns the number of words. 
//Words are trimmed left and right. Some words may be empty.
	int nWords = CountOccurrences(separator) + 1;
	Words.SetArraySize(nWords);
	const char * pb = m_pC;
	int word = 0;
	while(true){
		//Put pa at beginning of a word
		const char * pa = pb;
		//Put pb at the next "separator" character or just after end of range.
		while(*pb && (*pb) != separator){++pb;}
		//A word has been found. Could be empty.
		if(pb > pa){
			Debug::Assert(word < nWords, "02262013,8:39");
//bool Create(const char * ch, int start, int nChars); //nChars characters starting at start.
//			Words[word].Create(m_pC, pa - m_pC, pb - pa);
			Words[word].Create(pa, 0, pb - pa);
			Words[word].Trim();
			++word;
		}
		//Break if at the terminating '\0'.
		if(!(*pb))break;
		//Skip the "separator" character.
		Debug::Assert((*pb) == separator, "02262013,8:46");
		++pb;
	}
	Debug::Assert(word == nWords, "02262013,8:47");
	return nWords;
}										//SeparateIntoWordsUsingSeparator
*/
//--------------------------------------------------------------------
int Str::FindNextIsspace(int start)const{
//Finds position of first isspace character at or after "start". Returns -1 if not found.
	Debug::Assert(start >= 0 && start < m_Size, "01142013,8:99");
	const char * p = m_pC + start;
	while(*p){
		if(isspace(*p))return int(p - m_pC);
		++p;
	}
	return -1;
}										//FindNextIsspace
//--------------------------------------------------------------------
int Str::FindPreviousIsspace(int start)const{
//Finds position of last isspace character at or before "start". Returns -1 if not found.
	Debug::Assert(start >= 0 && start < m_Size, "01142013,7:00");
	const char * p = m_pC + start;
	while(p >= m_pC){
		if(isspace(*p))return int(p - m_pC);
		--p;
	}
	return -1;
}										//FindPreviousIsspace
//--------------------------------------------------------------------
int Str::FindNextNonspace(int start)const{
//Finds position of first non-isspace character at or after "start". Returns -1 if not found.
	Debug::Assert(start >= 0 && start < m_Size, "01142013,7:01");
	const char * p = m_pC + start;
	while(*p){
		if(!isspace(*p))return int(p - m_pC);
		++p;
	}
	return -1;
}										//FindNextNonspace
//--------------------------------------------------------------------
int Str::FindPreviousNonspace(int start)const{
//Finds position of last non-isspace character at or before "start". Returns -1 if not found.
	Debug::Assert(start >= 0 && start < m_Size, "01142013,7:02");
	const char * p = m_pC + start;
	while(p >= m_pC){
		if(!isspace(*p))return int(p - m_pC);
		--p;
	}
	return -1;
}										//FindPreviousNonspace
//--------------------------------------------------------------------
int Str::FindNextAlpha(int start)const{
	Debug::Assert(start >= 0 && start < m_Size, "01142013,7:03");
	const char * p = m_pC + start;
	while(*p){
		if(isalpha(*p))return int(p - m_pC);
		++p;
	}
	return -1;
}										//FindNextAlpha
//--------------------------------------------------------------------
int Str::FindNextNonalpha(int start)const{
	Debug::Assert(start >= 0 && start < m_Size, "01142013,7:04");
	const char * p = m_pC + start;
	while(*p){
		if(!isalpha(*p))return int(p - m_pC);
		++p;
	}
	return -1;
}										//FindNextNonalpha
//--------------------------------------------------------------------
int Str::FindNextDigit(int start)const{
//Returns -1 if not found.
	Debug::Assert(start >= 0 && start < m_Size, "01142013,7:05");
	const char * p = m_pC + start;
	while(*p){
		if(isdigit(*p))return int(p - m_pC);
		++p;
	}
	return -1;
}										//FindNextDigit
//--------------------------------------------------------------------
int Str::FindNextDigit(int startSearch, int & value)const{
//Calculates value and returns index, or -1 if not found.
	Debug::Assert(startSearch >= 0 && startSearch < m_Size, "01142013,7:05");
	const char * p = m_pC + startSearch;
	while(*p){
		if(isdigit(*p)){
			//Calculate value.
			char c[2];
			c[0] = *p;
			c[1] = '\0';
			value = atoi(c);
			return int(p - m_pC);
		}
		++p;
	}
	return -1;
}										//FindNextDigit
//--------------------------------------------------------------------
int Str::FindNextNondigit(int start)const{
	Debug::Assert(start >= 0 && start < m_Size, "01142013,7:06");
	const char * p = m_pC + start;
	while(*p){
		if(!isdigit(*p))return int(p - m_pC);
		++p;
	}
	return -1;
}										//FindNextNondigit
//--------------------------------------------------------------------
int Str::FindNextBlockOfDigits(int startSearch, int & nDigits, int & value)const{
//Returns index, or -1 if none found, and calculates number of digits and 
//value (0 or positive). The block can have 1 or more digits. 
	int ia = FindNextDigit(startSearch);
	if(ia == -1)return -1; 
	int ib = FindNextNondigit(ia + 1);
	if(ib == -1)ib = m_Size;
	nDigits =  ib - ia;
	Debug::Assert(nDigits > 0);

	//Copy to string.
	Str sz(m_pC, ia, nDigits);
	const char * cp = sz.GetString();
	value = atoi(cp);
	return ia;
}										//FindNextBlockOfDigits
//--------------------------------------------------------------------
int Str::FindNextOneOf(const char * theChars, int start)const{
//"theChars" contains a list of chars. Find any of them at or after "start".
//Returns the position where any of the given characters is found first, or -1 if not found.
	Debug::Assert(0 <= start && start< m_Size && theChars != nullptr, "01142013,7:07");
	int L = strlen(theChars);
	Debug::Assert(L > 0, "01142013,7:08");
	const char * p = m_pC + start;
	while(*p){
		const char * q = strchr(theChars, *p);
		if(q != nullptr){
			return int(p - m_pC);
		}
		++p;
	}
	return -1;
}										//FindNextOneOf
//--------------------------------------------------------------------
int Str::FindNextOneOfSkipEnclosures(const char * theChars, int start)const{
//An enclosure is (...), or [...], or {...}. "theChars" contains a list of chars. 
//Find any of them at or after "start", but skip enclosures because they may 
//also contain characters present in "theChars". Returns the position where 
//any of the given characters is found first, or -1 if none is found.
	Debug::Assert(0 <= start && start< m_Size && theChars != nullptr, "01142013,7:09");
	int L = strlen(theChars);
	Debug::Assert(L > 0, "01142013,7:10");
	const char * p = m_pC + start;
	while(*p){
		if(*p == '(' || *p == '[' || *p == '{'){
			int k = FindMatchingBracket(p - m_pC);
			if(k != -1)p = m_pC + k + 1;
		}
		else{
			const char * q = strchr(theChars, *p);
			if(q != nullptr){
				return int(p - m_pC);
			}
			++p;
		}
	}
	return -1;
}										//FindNextOneOfSkipEnclosures
//--------------------------------------------------------------------
int Str::FindPreviousOneOfSkipEnclosures(const char * theChars, int start)const{
//An enclosure is (...), or [...], or {...}. "theChars" contains a list of chars. 
//Find any of them at or before "start", but skip enclosures because they may 
//also contain characters present in "theChars". Returns the position where 
//any of the given characters is found first, or -1 if none is found.
	Debug::Assert(0 <= start && start< m_Size && theChars != nullptr, "01142013,7:11");
	int L = strlen(theChars);
	Debug::Assert(L > 0, "01142013,7:12");
	const char * p = m_pC + start;
	while(p >= m_pC){
		if(*p == ')' || *p == ']' || *p == '}'){
			int k = FindMatchingBracket(p - m_pC);
			if(k != -1)p = m_pC + k - 1;
		}
		else{
			const char * q = strchr(theChars, *p);
			if(q != nullptr){
				return int(p - m_pC);
			}
			--p;
		}
	}
	return -1;
}										//FindPreviousOneOfSkipEnclosures
//--------------------------------------------------------------------
/*
int Str::FindNextOneOf(const ArrayOfStr & strings, int start, int & which)const{
//Finds the first occurrence of one of the given strings, starting at or after "start".
//Returns the position where any of the given strings is found first, or -1 if not found.
//Also returns "which"=the index of the string that was found.
//The strings are taken in the order they are given. If a string of length n has its first 
//m < n chars the same as another string of length m, the longest string must be given first.
//For example, to search for "->" and "->*", give them in the order  "->*", "->", otherwise
//"->*" will never be found.
	Debug::Assert(start >= 0, "01142013,7:13");
	if(start >= m_Size)return -1;
	int n = strings.GetSize();
	if(n < 1)return -1;
	const char * p = m_pC + start;
	while(*p){
		for(int i = 0; i < n; ++i){
			const char * q = strings[i].GetString();
			int m = strings[i].GetLength();
			if((start + m) > m_Size)continue; //String is too long, doesn't fit.
			for(int j = 0; j < m; ++j){
				if(p[j] != q[j])break;
				if(j == (m - 1)){
					which = i;
					return int(p - m_pC);
				}
			}
		}
		++p;
	}
	return -1;
}										//FindNextOneOf
*/
//--------------------------------------------------------------------
int Str::Find(char cc)const{
//Find character starting at left, returns its position, or -1 if not found.
	if(!cc)return -1;
	const char * p = m_pC;
	while(*p){
		if(*p == cc)return int(p - m_pC);
		++p;
	}
	return -1;
}										//Find
//--------------------------------------------------------------------
int Str::ReverseFind(char cc)const{
//Find character starting at right, return -1 if error or not found.
	if(!cc)return -1;
	const char * p = m_pC + m_Size - 1;
	while(p >= m_pC){
		if(*p == cc)return (p - m_pC);
		--p;
	}
	return -1;
}										//ReverseFind
//--------------------------------------------------------------------
int Str::Find(char cc, int nStart)const{
//Find character starting at zero-based index and going right, return -1 if error or not found.
	Debug::Assert(nStart >= 0, "01142013,7:14");
	if(nStart >= m_Size || cc == '\0')return -1;
	const char * p = m_pC + nStart;
	while(*p){
		if(*p == cc)return int(p - m_pC);
		++p;
	}
	return -1;
}										//Find
//--------------------------------------------------------------------
int Str::ReverseFind(char cc, int nStart)const{
//Find character starting at nStart (zero-based) and going left, return -1 if error or not found.
	if(nStart >= m_Size || cc == '\0')return -1;
	const char * p = m_pC + nStart;
	while(p >= m_pC){
		if(*p == cc)return int(p - m_pC);
		--p;
	}
	return -1;
}										//ReverseFind
//--------------------------------------------------------------------
int Str::Find(const char * s)const{
//Return index where first instance of "s" starts, or -1 if not found.
	char * p = strstr(m_pC, s);
	if(p == nullptr)return -1;
	return int(p - m_pC);
}										//Find
//--------------------------------------------------------------------
int Str::Find(const char * s, int nStart)const{
//Return index where first instance of substring starts at or after nStart, or -1 if not found.
	if(nStart >= m_Size)return -1;
	char * p = strstr(m_pC + nStart, s);
	if(p == nullptr)return -1;
	return int(p - m_pC);
}										//Find
//--------------------------------------------------------------------
int Str::FindExclusive(const char * cc, int nStart)const{
//Starting from "nStart" and searching right, finds the second character in "cc", but 
//only if not preceded by first character in "cc". Intended for parsing languages such 
//as TeX or C++ that assign a special meaning to some characters. Examples: "\{",  "\}".
//Returns the position of the second character, or -1 if not found.
	Debug::Assert(strlen(cc) == 2 && nStart >= 0, "01142013,7:15");
	char ca = cc[0];
	char cb = cc[1];
	int ia = nStart;
	while(true){
		ia = Find(cb, ia);
		if(ia == -1)return -1;
		if(m_pC[ia - 1] != ca)return ia;
		++ia;
	}
	return -1; //Just to satisfy the compiler.
}										//FindExclusive
//--------------------------------------------------------------------
int Str::ReverseFindExclusive(const char * cc, int nStart)const{
//Starting from "nStart" and searching left, finds the second character in "cc", but 
//only if not preceded by first character in "cc". Intended for parsing languages such 
//as TeX or C++ that assign a special meaning to some characters. Examples: "\{",  "\}".
//Returns the position of the second character, or -1 if not found.
	Debug::Assert(strlen(cc) == 2 && nStart > 0, "01142013,7:16");
	char ca = cc[0];
	char cb = cc[1];
	int ia = nStart;
	while(true){
		ia = ReverseFind(cb, ia);
		if(ia == -1)return -1;
		if(ia == 0 || m_pC[ia - 1] != ca)return ia;
		//Here ia > 0.
		--ia;
	}
	return -1;
}										//ReverseFindExclusive
//--------------------------------------------------------------------
int Str::FindTextBetween(char ca, char cb, CStr & textBetween, int start)const{
//Find text between two given characters, search from left to right. If search is successful, 
//return index just after cb (which could be beyond end). Otherwise return -1.
	char sa[2], sb[2];
	sa[0] = ca;
	sa[1] = '\0';
	sb[0] = cb;
	sb[1] = '\0';
	int j = FindTextBetween(sa, sb, textBetween, start);
	if(j == -1)return -1;
	return j - 1;
}										//FindTextBetween
//--------------------------------------------------------------------
int Str::FindTextBetween(const char * s1, const char * s2, CStr & textBetween, int start)const{
//Find text between two given strings, search from left to right. If search is successful, 
//return index just after s2 (which could be beyond end). Otherwise return -1.
	Debug::Assert(s1 != nullptr && s2 != nullptr, "01142013,7:17");
	int L1 = (int)strlen(s1);
	int L2 = (int)strlen(s2);
	if(L1 < 1 ||  L2 < 1)return -1;
	if(start > (m_Size - L1 - L2) || start < 0)return -1;
	int ka = Find(s1, start);
	if(ka < 0)return -1;
	ka += L1; //ka now points to character just after firstString.
	int kb = Find(s2, ka);
	if(!(kb > ka))return -1;
	textBetween = CStr(m_pC, ka, kb - ka);
	return kb + L2;
}										//FindTextBetween
//--------------------------------------------------------------------
int Str::FindMatchingBracket(int index)const{
//Given any of ( [ { < ) ] } >, find the match. Returns -1 if not found.
	Debug::Assert(index >= 0 && index < m_Size, "01142013,7:18");
	char ca = m_pC[index];
	char cb;
	int D;
	switch(ca){
	case '(': cb = ')'; D =  1; break;
	case '[': cb = ']'; D =  1; break;
	case '{': cb = '}'; D =  1; break;
	case '<': cb = '>'; D =  1; break;
	case ')': cb = '('; D = -1; break;
	case ']': cb = '['; D = -1; break;
	case '}': cb = '{'; D = -1; break;
	case '>': cb = '<'; D = -1; break;
	default: return -1;
	}
	int count = 1;
	int k = index;
	char c;
	while(count > 0){
		k += D;
		if(k < 0 || k >= m_Size)return -1;
		c = *(m_pC + k);
		if     (c == ca)++count;
		else if(c == cb)--count;
	}
	return k;
}										//FindMatchingBracket
//--------------------------------------------------------------------
int Str::FindMatchingBracketExclusive(char exclude, int index)const{
//Given any of ( [ { < ) ] } >, find the match that is NOT preceded by 
//character "exclude". Returns -1 if none found.
	Debug::Assert(index >= 0 && index < m_Size, "01142013,7:19");
	char ca = m_pC[index];
	char cb;
	int D;
	switch(ca){
	case '(': cb = ')'; D =  1; break;
	case '[': cb = ']'; D =  1; break;
	case '{': cb = '}'; D =  1; break;
	case '<': cb = '>'; D =  1; break;
	case ')': cb = '('; D = -1; break;
	case ']': cb = '['; D = -1; break;
	case '}': cb = '{'; D = -1; break;
	case '>': cb = '<'; D = -1; break;
	default: return -1;
	}
	int count = 1;
	int k = index;
	char c;
	while(count > 0){
		k += D;
		if(k < 0 || k >= m_Size){
			return -1;
		}
		c = *(m_pC + k);
		if(c == ca && (k == 0 || (*(m_pC + k - 1)) != exclude)){
			++count;
		}
		else if(c == cb && (k == 0 || (*(m_pC + k - 1)) != exclude)){
			--count;
		}
	}
	return k;
}										//FindMatchingBracketExclusive
//--------------------------------------------------------------------
/*bool Str::FindNextLine(int start, CStr & line)const{
//Find the first line that starts at or after "start", even if the line is empty.
//A "line" starts at BOF (position 0) or just after a '\n' and ends at the following '\n' or just before EOF.

	
//The returned CStr ranges from just after the preceding '\n' up to just before
//the terminating '\n' or '\0'. It has 0 or more characters. It has no '\n' chars.
//The position of the '\n' or '\0' following the line is "line.GetOffsetPastEnd()"
	Debug::Assert(start >= 0, "01142013,7:20");
	if(start >= m_Size)return false;
	//Put p on the next '\n'. This is where the next line begins.
	const char * p = m_pC + start;
	while(*p && *p != '\n'){++p;}
	if(!(*p))return false; //'\0' reached without finding '\n'.
	Debug::Assert(*p == '\n', "01142013,7:21");
	//Put q just after the end of the line.
	const char * q = p + 1;
	while(*q && *q != '\n'){++q;}
	line.Create(m_pC, int(p - m_pC) + 1, int(q - p) - 1);
	return true;
}										//FindNextLine
//--------------------------------------------------------------------
bool Str::FindPreviousLine(int end, CStr & line)const{
//Find the last line that ends at or before "end", even if the line is empty.
//A "line" starts just after a '\n' and ends at the next '\n' or '\0'.
//The returned CStr has no '\n' characters in its range.
	Debug::Assert(end >= 0 && end < m_Size, "01142013,7:22");
	//Put q just after the end of the previous line.
	const char * q = m_pC + end;
	while(q >= m_pC && *q != '\n'){--q;}
	if(q < m_pC)return false; //m_pC passed without finding a '\n'.
	Debug::Assert(*q == '\n', "01142013,7:23");
	//Put p on the previous '\n' or just before m_pC.
	const char * p = q - 1;
	while(p >= m_pC && *p != '\n'){--p;}
	line.Create(m_pC, int(p - m_pC) + 1, int(q - p) - 1);
	return true;
}										//FindPreviousLine*/
//--------------------------------------------------------------------
int Str::FindFirstLine(CStr & line)const{
//A line starts at position 0 or just after any '\n', and ends at the following '\n' or '\0'.
//This method finds the first line and returns the position of its terminating '\n' or '\0'.
//"line" does not include the ending '\n' or '\0', contains no '\n' characters, and can be empty.
//Always returns >= 0.
	const char * q = m_pC;
	while(*q && *q != '\n'){
		++q;
	}
	int k = int(q - m_pC); //Position of '\n' or '\0'.
	line.Create(m_pC, 0, k);
	return k;
}										//FindFirstLine
//--------------------------------------------------------------------
int Str::FindNextLine(int newline, CStr & line)const{
//A line starts at position 0 or just after any '\n', and ends at the following '\n' or '\0'.
//This method finds the line starting after position "newline" and returns the position of its terminating '\n' or '\0'.
//"newline" must be -1 or the position of a '\n' character. The value '\0' is not allowed.
//"line" does not include the ending '\n' or '\0', contains no '\n' characters, and can be empty.
//Always returns >= 0.
	Debug::Assert(newline == -1 || (IsInRange(newline) && m_pC[newline] == '\n'), "01142013,7:24");
	const char * p = m_pC + newline + 1; //good even if newline == -1.
	const char * q = p;
	while(*q && *q != '\n'){++q;}
	line.Create(m_pC, int(p - m_pC), int(q - p));
	return int(q - m_pC);
}										//FindNextLine
//--------------------------------------------------------------------
int Str::FindFirstLineExclusive(char cc, CStr & line)const{
//A "true '\n'" is a '\n' character not preceded by a continuation or line splice character cc.
//An exclusive line starts at position 0 or just after any true '\n', and ends at the following true '\n' or '\0'.
//This method finds the first exclusive line and returns the position of its terminating true '\n' or '\0'.
//"line" does not include the ending true '\n' or '\0', contains no true '\n' characters, 
//can contain any number of '\n' preceded by cc, and can be empty. Always returns >= 0.
	const char * q = m_pC;
	while(*q && (*q != '\n' || *(q - 1) == cc)){++q;}
	int k = int(q - m_pC); //Position of '\n' or '\0'.
	line.Create(m_pC, 0, k);
	return k;
}										//FindFirstLineExclusive
//--------------------------------------------------------------------
int Str::FindNextLineExclusive(char cc, int newline, CStr & line)const{
//A "true '\n'" is a '\n' character not preceded by a continuation or line splice character cc.
//An exclusive line starts at position 0 or just after any true '\n', and ends just before the next true '\n' or '\0'.
//This method finds the first exclusive line and returns the position of its terminating true '\n' or '\0'.
//"newline" must be -1 or the position of a true '\n' character. The value '\0' is not allowed.
//"line" does not include the ending true '\n' or '\0', contains no true '\n' characters, 
//can contain any number of '\n' preceded by cc, and can be empty. Always returns >= 0.
	Debug::Assert(newline == -1 || (IsInRange(newline) && m_pC[newline] == '\n'), "01142013,7:25");
	if(newline > 0)Debug::Assert(m_pC[newline - 1] != cc, "01142013,7:26");
	// p points to first character after BOF or after a true '\n'.
	const char * p = m_pC + newline + 1; //good even if newline == -1.
	// q stops at terminating '\0' or first true '\n' after p.
	const char * q = p;
	while(*q && (*q != '\n' || *(q - 1) == cc)){++q;}
	line.Create(m_pC, int(p - m_pC), int(q - p));
	return int(q - m_pC);
}										//FindNextLineExclusive
//--------------------------------------------------------------------
bool Str::FindNthLineStartingWithWord(int Nth, const char * word, CStr & Line)const{
//Find the Nth line that starts with a given word. See SeparateIntoNonemptyLines for definition of Line.
	Debug::Assert(Nth > 0 && strlen(word) > 0, "01142013,7:27");
	int L = strlen(word);
	const char * pb = m_pC;
	int line = 0;
	while(true){
		//Put pa at the beginning of m_pC or just after a '\n'.
		const char * pa = pb;
		//Put pb at the next '\n' or '\0'.
		while(*pb && *pb != '\n'){++pb;}
		if((pb - pa) >= L && strncmp(word, pa, L) == 0){
			Line.Create(m_pC, pa - m_pC + 1, pb - pa);
			return true;
		}
		if(!(*pb))break;
		//Skip the '\n'.
		++pb;
	}
	return false;
}										//FindNthLineStartingWithWord
//--------------------------------------------------------------------
int Str::FindNextCommand(int start, CStr & title, CStr & content)const{
//Search this Str for the first command beginning at or after position "start".
//A command is a string of the form  "title{content}" where "title" contains
//no spaces, braces or newlines, but "content" can contain anything providing the
//braces, if any, are paired. Returns the index of the terminating "}", or -1 if 
//not found. "title" and "content" are space-trimmed.
	Debug::Assert(start >= 0 && start < m_Size, "01142013,7:28");
	int ib = Find('{', start); //ib=index of '{'
	if(ib < 0){
		return -1;
	}
	int ic = GetTextBetweenBrackets(ib, content); //ic=index of '}'
	if(ic < 0){
		return -1;
	}
	int id = FindPreviousSpaceDelimitedWord(ib - 1, title); //id=index where "title" starts.
	if(id < 0){
		return -1;
	}
	content.Trim();
	return ic;
}										//FindNextCommand
//--------------------------------------------------------------------
int Str::FindNextCommandExclusive(char exclude, int start, CStr & title, CStr & content)const{
//Similar to FindNextCommand but "{" and "}" are invalid if preceded by character "exclude".

if(start == 64986){
	int hhh = 8;
}

	Debug::Assert(start >= 0 && start < m_Size, "01142013,7:29");
	char cc[3];
	cc[0] = exclude;
	cc[1] = '{';
	cc[2] = '\0';
	int ib = FindExclusive(cc, start); //ib=index of '{' not preceded by '\'.
	if(ib < 0){
		return -1;
	}
	int ic = GetTextBetweenBracketsExclusive(exclude, ib, content); //ic=index of '}' not preceded by '\'.
	if(ic < 0){
		return -1;
	}
	int id = FindPreviousSpaceDelimitedWord(ib - 1, title); //id=index where "title" starts.
	if(id < 0){
		return -1;
	}
	content.Trim();
	return ic;
}										//FindNextCommandExclusive
//--------------------------------------------------------------------
bool Str::FindNthCommandWithTitle(int Nth, const Str & title, CStr & content)const{
//The Nth command "title{content}" is found and "content" is returned.
	Debug::Assert(Nth > 0, "01142013,7:30");
	CStr t, c;
	int ia = 0;
	int count = 0;
	while((ia = FindNextCommand(ia, t, c)) > 0){
		if(t == title){
			if((++count) == Nth){
				content = c;
				return true;
			}
		}
	}
	return false;
}										//FindNthCommandWithTitle
//--------------------------------------------------------------------
bool Str::FindNthCommandWithTitleExclusive(char exclude, int Nth, const Str & title, CStr & content)const{
//Similar to FindNthCommandWithTitle but "{" and "}" are invalid if preceded by character "exclude".
	Debug::Assert(Nth > 0, "01142013,7:31");
	CStr t, c;
	int ia = 0;
	int count = 0;
	while((ia = FindNextCommandExclusive(exclude, ia, t, c)) > 0){
		if(t == title){
			++count;
			if(count == Nth){
				content = c;
				return true;
			}
		}
	}
	return false;
}										//FindNthCommandWithTitleExclusive
//--------------------------------------------------------------------
int Str::FindNextSpaceDelimitedWord(int start, CStr & word)const{
//A Word is text enclosed by isspace characters. A Word is non-empty and contains
//no isspace characters. Finds the first word that starts at or after "start".
//Returns index in m_pC where the word ends, or -1 if not found.
	if(start >= m_Size || start < 0)return -1;
	//Put p at the beginning of the next word.
	const char * p = m_pC + start;
	while(*p && isspace(*p)){++p;}
	if(!(*p))return -1;
	//Put q just after the end of the word.
	const char * q = p;
	while(*q && !isspace(*q)){++q;}
	word.Create(m_pC, int(p - m_pC), int(q - p));
	return int(q - m_pC - 1); //Returns within m_pC.
}										//FindNextSpaceDelimitedWord
//--------------------------------------------------------------------
int Str::FindPreviousSpaceDelimitedWord(int end, CStr & word)const{
//A Word is text enclosed by isspace characters. A Word is non-empty and contains
//no isspace characters. Finds the last word that ends at or before "end".
//Returns index in m_pC where the word starts, or -1 if not found.
	if(end >= m_Size || end < 0)return -1;
	//Put q at the end of the previous word.
	const char * q = m_pC + end;
	while(q >= m_pC && isspace(*q)){--q;}
	if(q < m_pC)return -1;
	//Put p just before the beginning of the word.
	const char * p = q;
	while(p >= m_pC && !isspace(*p)){--p;}
	word.Create(m_pC, int(p - m_pC + 1), int(q - p));
	return int(p - m_pC + 1); //Returns >= 0.
}										//FindPreviousSpaceDelimitedWord
//--------------------------------------------------------------------
int Str::FindNextNonalnumDelimitedWord(int start, CStr & word)const{
//A Word is text enclosed by non-alpha-numeric characters. A Word is non-empty and contains
//no alphanumeric characters. Finds the first word that starts at or after "start".
//Returns index in m_pC where the word ends, or -1 if not found.
	if(start >= m_Size || start < 0)return -1;
	//Put p at the beginning of the next word.
	const char * p = m_pC + start;
	while(*p && !isalnum(*p)){++p;}
	if(!(*p))return -1;
	//Put q just after the end of the word.
	const char * q = p;
	while(*q && isalnum(*q)){++q;}
	word.Create(m_pC, int(p - m_pC), int(q - p));
	return int(q - m_pC - 1); //Returns within m_pC.
}										//FindNextNonalnumDelimitedWord
//--------------------------------------------------------------------
int Str::FindPreviousNonalnumDelimitedWord(int end, CStr & word)const{
//A Word is text enclosed by non-alpha-numeric characters. A Word is non-empty and contains
//no non-alpha-numeric characters. Finds the last word that ends at or before "end".
//Returns index in m_pC where the word starts, or -1 if not found.
	if(end >= m_Size || end < 0)return -1;
	//Put q at the end of the previous word.
	const char * q = m_pC + end;
	while(q >= m_pC && !isalnum(*q)){--q;}
	if(q < m_pC)return -1;
	//Put p just before the beginning of the word.
	const char * p = q;
	while(p >= m_pC && isalnum(*p)){--p;}
	word.Create(m_pC, int(p - m_pC + 1), int(q - p));
	return int(p - m_pC + 1); //Returns >= 0.
}										//FindPreviousNonalnumDelimitedWord
//--------------------------------------------------------------------
int Str::CountOccurrences(char c)const{
//Return the number of occurrences of character "c" in this entire Str.
	const char * p = m_pC;
	int count = 0;
	while(*p){
		if(*(p++) == c)++count;
	}
	return count;
}										//CountOccurrences
//--------------------------------------------------------------------
int Str::CountOccurrences(char c, int position)const{
//Count the occurrences of 'c' prior to "position". Range of "position" is 0 to m_Size, both included.
//position = m_Size means the terminating '\0'.
	Debug::Assert(position >= 0 && position <= m_Size, "01142013,7:32");
	const char * p = m_pC;
	int count = 0;
	while(int(p - m_pC) < position){ //will not reach the terminating '\0'
		if(*(p++) == c)++count;
	}
	return count;
}										//CountOccurrences
//--------------------------------------------------------------------
int Str::FindLineNumberWithPosition(int position, int & column)const{
//Given the 0-based position of a character in this Str, find its 1-based line and 1-based column.
//The character is to right of the caret.
	Debug::Assert(position >= 0 && position <= m_Size, "01142013,7:33"); //caret position ranges 0 to m_Size, both included.
	int n = CountOccurrences('\n', position); //a '\n' character belongs to the line it ends.
	int ia = ReverseFind('\n', position - 1);
	column = position - ia; //correct even if ia = -1 (the first line).
	return n + 1;
}										//FindLineNumberWithPosition
//--------------------------------------------------------------------
int Str::CountOccurrences(const char * s)const{
//Return the number of occurrences of nullptr-terminated character array "s" in this Str.
	Debug::Assert(s != nullptr && strlen(s) > 0, "01142013,7:34");
	int count = 0;
	int index = -1;
	while(true){
		index = Find(s, index + 1);
		if(index == -1)break;
		++count;
	}
	return count;
}										//CountOccurrences
//--------------------------------------------------------------------
int Str::CountLines()const{
//A line starts at the beginning of m_pC or just after each '\n' character.
//The line ends just before the following '\n' character or terminating '\0'.
//The line is empty when the '\n' characters are consecutive. In particular,
//the last line is empty if '\n' is the last character of m_pC.
//A line contains no '\n' characters. Returns the number of lines.
	if(m_Size == 0)return 0;
	return CountOccurrences('\n') + 1;
}										//CountLines
//--------------------------------------------------------------------
int Str::CountLinesExclusive(char cc)const{
//A line starts at positon 0 or after '\n' not preceded by cc, and ends at '\n'
//not preceded by cc, or at last position before '\0'. 
	int count = 0;
	const char * p = m_pC;
	while(*p){
		const char * q = p;
		while(*q && (*q != '\n' || *(q - 1) == cc)){++q;};
		//At this point, q points to a true '\n' or a '\0'.
		Debug::Assert(*q == '\n' || *q == '\0', "01142013,7:35");
		if(*q == '\n' && q != m_pC)Debug::Assert(*(q - 1) != cc, "01142013,7:36");
		++count;
		if(*q =='\0')break;
		p = q + 1;
	}
	return count;
}										//CountLinesExclusive
//--------------------------------------------------------------------
int Str::CountNonemptyLines()const{
//Count lines as before, but excluding empty lines.
//A line is nonempty when it contains any characters other than \n, even spaces or tabs.
	int nLines = 0;
	const char * pb = m_pC;
	while(true){
		const char * pa = pb;
		//Put pb at the next '\n' or '\0'.
		while(*pb && *pb != '\n'){++pb;}
		if(pb > pa){++nLines;} //This accepts blank lines that contain only isspace characters.
		if(!(*pb))break;
		//Skip the '\n'.
		++pb;
	}
	return nLines;
}										//CountNonemptyLines
//--------------------------------------------------------------------
int Str::CountNonblankLines()const{
//Count only lines that contain non-isspace characters.

	Debug::Assert(false, "01142013,7:37"); //complete, but never tested!!!!

	int nLines = 0;
	const char * pb = m_pC;
	while(true){
		const char * pa = pb;
		//Put pa at the next non-isspace character ('\n' is an isspace char).
		while(*pa && isspace(*pa)){++pa;}
		if(!(*pa))break; //reached '\0'
		//Put pb at the next '\n' or '\0'.
		pb = pa + 1;
		while(*pb && *pb != '\n'){++pb;}
		++nLines;
		if(!(*pb))break;
	}
	return nLines;
}										//CountNonblankLines
//--------------------------------------------------------------------
int Str::CountNonemptyWords()const{
//A word begins at the beginning of m_pC or just after each group of consecutive isspace characters
//(isspace characters are: space, formfeed, newline, carriage return, tab, vertical tab).
//The word ends just before the following isspace or terminating '\0' character.
//A word is never empty. A word contains no isspace characters. Returns the number of words.
	int nWords = 0;
	const char * pb = m_pC;
	while(true){
		//Put pa at the next nonspace character or '\0'.
		const char * pa = pb;
		while(*pa && isspace(*pa)){++pa;}
		if(!(*pa))break;
		// pa is now at a nonspace character. Put pb at the next isspace character or '\0'.
		pb = pa;
		while(*pb && !isspace(*pb)){++pb;}
		//Count the word between pa and pb - 1.
		Debug::Assert(pb > pa, "01142013,7:38");
		++nWords;
		if(!(*pb))break;
	}
	return nWords;
}										//CountNonemptyWords
//--------------------------------------------------------------------
//PRIOR TO 03-14-2009, ALL COMPARE WAS CASE-INSENSITIVE!!!
int Str::CompareCaseInsensitive(const char * bs)const{
//Return  <0 if this<bs,  =0 if this==bs,  >0 if this>bs. Upper case comparison.
	Str Sme(*this);
	Str Sother(bs);
	const char * me    = (Sme.ToUpper()).GetString();
	const char * other = (Sother.ToUpper()).GetString();
	int jj = strcmp(me, other);
	return jj;
}										//CompareCaseInsensitive
//--------------------------------------------------------------------
int Str::CompareCaseInsensitive(const Str & bs)const{
//Return  <0 if this<bs,  =0 if this==bs,  >0 if this>bs. Upper case comparison.
	return CompareCaseInsensitive(bs.GetString());
}										//CompareCaseInsensitive
//--------------------------------------------------------------------
int Str::CompareCaseInsensitive(const CStr & bs)const{
//Return  <0 if this<bs,  =0 if this==bs,  >0 if this>bs. Upper case comparison.
	Str Sme(*this);
	Str Sother(bs);
	const char * me    = (Sme.ToUpper()).GetString();
	const char * other = (Sother.ToUpper()).GetString();
	int jj = strcmp(me, other);
	return jj;
}										//CompareCaseInsensitive
//--------------------------------------------------------------------
int Str::CompareCaseSensitive(const char * bs)const{
//Return  <0 if this<bs,  =0 if this==bs,  >0 if this>bs.
	int jj = strcmp(m_pC, bs);
	return jj;
}										//CompareCaseSensitive
//--------------------------------------------------------------------
int Str::CompareCaseSensitive(const Str & bs)const{
//Return  <0 if this<bs,  =0 if this==bs,  >0 if this>bs. Upper case comparison.
	return CompareCaseSensitive(bs.GetString());
}										//CompareCaseSensitive
//--------------------------------------------------------------------
int Str::CompareCaseSensitive(const CStr & bs)const{
//Return  <0 if this<bs,  =0 if this==bs,  >0 if this>bs. Upper case comparison.
	Str Sother(bs);
	const char * other = Sother.GetString();
	int jj = strcmp(m_pC, other);
	return jj;
}										//CompareCaseSensitive
//--------------------------------------------------------------------
bool Str::StartsWith(const char * wrd)const{
	int L = strlen(wrd);
	if(L > m_Size)return false;
	for(int i = 0; i < L; ++i){
		if(m_pC[i] != wrd[i])return false;
	}
	return true;
}										//StartsWith
//--------------------------------------------------------------------
bool Str::EndsWith(const char * wrd)const{
	int L = strlen(wrd);
	if(L > m_Size)return false;
	for(int i = 0; i < L; ++i){
		if(m_pC[m_Size - L + i] != wrd[i])return false;
	}
	return true;
}										//EndsWith
//--------------------------------------------------------------------
//BSTR Str::GetBSTR()const{
//	BSTR zbs = _com_util::ConvertStringToBSTR(m_pC);
//	return zbs;
//}										//GetBSTR
//--------------------------------------------------------------------
System::String ^ Str::GetSystemString()const{
	String ^ sa = gcnew String(*this);
	return sa;
}										//GetSystemString
//--------------------------------------------------------------------
void Str::Format(const char * format, ...){
	Debug::Assert(format != nullptr, "01142013,7:39");
	Formatter F;
	va_list argList;
	va_start(argList, format);
	va_list argListSave = argList;
	int L = F.EstimateLengthOfFormattedString(format, argList);
	va_end(argList);

	delete [] m_pC;
	m_pC = new char[L + 1];
	//m_Size = vsprintf(m_pC, format, argListSave);
	m_Size = vsprintf_s(m_pC, L + 1, format, argListSave);
	Debug::Assert(m_Size <= L, "01142013,7:40");
	va_end(argListSave);
}										//Format
//--------------------------------------------------------------------
bool Str::WriteMyStringToFile(FILE * file)const{
//Writes the string m_pC to "file". Returns true if correct, false if error.
//"file" is open as a text file for writing.
	Debug::Assert(file != nullptr, "01142013,7:40");
	if(file == nullptr)return false;
	if(m_Size > 0){
		int sc = sizeof(char);
		int count = fwrite(m_pC, sc, m_Size, file);
		Debug::Assert(count == m_Size, "01142013,7:42");
		if(count != m_Size)return false;
	}
	return true;
}										//WriteMyStringToFile
//--------------------------------------------------------------------
bool Str::WriteMeToFile(FILE * file)const{
//Writes the Str object to "file". Returns true if correct, false if error.
//"file" is open as a text file for writing.
	if(file == nullptr)return false;
	int si = sizeof(int);
	int count = fwrite(&m_Size, si, 1, file); //This is the difference with WriteMyStringToFile!
	if(count != 1)return false;
	if(m_Size > 0){
		int sc = sizeof(char);
		count = fwrite(m_pC, sc, m_Size, file);
		if(count != m_Size)return false;
	}
	return true;
}										//WriteMeToFile
//--------------------------------------------------------------------
bool Str::ReadMeFromFile(FILE * file){
//Reads the Str from "file". Returns true if correct, false if error.
//"file" is open as a text file for reading.
	if(file == nullptr)return false;
	int si = sizeof(int);
	int L;
	int count = fread(&L, si, 1, file);
	if(count != 1)return false;
	m_Size = L;
	delete [] m_pC;
	m_pC = new char[m_Size + 1];
	if(m_Size > 0){
		int sc = sizeof(char);
		count = fread(m_pC, sc, m_Size, file);
		if(count != m_Size)return false;
	}
	m_pC[m_Size] = '\0';
	return true;
}										//ReadMeFromFile
//--------------------------------------------------------------------
bool Str::FindMeInFile(FILE * file)const{
//Searches for this Str in file "file" starting at the current file position.
//Leaves the file positioned just after the first occurrence of this Str, or EOF if not found.
//Returns true if successful, otherwise false.
	Debug::Assert(file != nullptr, "01142013,7:43");
	char cc;
	if(m_Size == 0)return true;
	else if(m_Size == 1){
		do{if((cc = fgetc(file)) == EOF)return false;}while(m_pC[0] != cc);
		return true;
	}
	else{ //m_Size > 1.
		int i;
		int ia = 0;
		int ib = m_Size - 1;
		char * b = new char[m_Size];
		for(i = 0; i < m_Size; ++i){
			if((b[i] = fgetc(file)) == EOF){delete [] b; return false;}
		}
		i = 0;
		while(true){
			int j = i + ia;
			if(j >= m_Size)j -= m_Size;
			if(m_pC[i] == b[j]){
				if(++i == m_Size){delete [] b; return true;}
			}
			else{
				ib = ia++;
				if(ia == m_Size)ia = 0;
				if((b[ib] = fgetc(file)) == EOF){delete [] b; return false;}
				i = 0;
			}
		}
	}
}										//FindMeInFile
//--------------------------------------------------------------------
bool Str::ReadFileUntilMeFound(FILE * file, char * buff)const{
//Reads characters into "buff" starting at current file position and until just passed
//the given string "s", or until EOF. Leaves "file" positioned just after given string,
//or at EOF. Returns true if successful.
	Debug::Assert(file != nullptr && buff != nullptr, "01142013,7:44");
	int LS = m_Size;
	int LB = strlen(buff);
	if(LS == 0 || LB < LS)return false;

	//Read initial LS characters.
	char * p  = buff;
	char * pz = buff + LB;
	for(int i = 0; i < LS; i++){
		*(p++) = fgetc(file);
	}
	bool ba, bb, bc;
	while((ba = (p < pz)) && (bb = (strncmp(p - LS, m_pC, LS)) != 0) && (bc = ((*p = fgetc(file)) != EOF))){
		p++;
	}
	*p = '\0';
	return (p != pz && *p != EOF);
}										//ReadFileUntilMeFound
//--------------------------------------------------------------------
bool Str::ReadFileIntoMe(FILE * file){
//Creates this Str object by reading the entire file. Returns true if successfull.
	Debug::Assert(file != nullptr, "01142013,7:45");
	if(file == nullptr)return false;
	//Determine the size of the file.
	int count = 0;
	char cc;
	rewind(file);
	while((cc = fgetc(file)) != EOF)++count;
	m_Size = count;
	delete [] m_pC;
	m_pC = new char[m_Size + 1];
	count = 0;
	rewind(file);
	while((m_pC[count++] = fgetc(file)) != EOF);
	Debug::Assert(count == (m_Size + 1), "01142013,7:46");
	m_pC[m_Size] = '\0';
	return true;
}										//ReadFileIntoMe
//--------------------------------------------------------------------
bool Str::isAlnum()const{
//isAlpha or isDigit is true.
	for(const char * p = m_pC; *p; ++p){if(!isalnum(*p))return false;}
	return true;
}										//isAlnum
//--------------------------------------------------------------------
bool Str::isAlpha()const{
//isLower or isUpper is true.
	for(const char * p = m_pC; *p; ++p){if(!isalpha(*p))return false;}
	return true;
}										//isAlpha
//--------------------------------------------------------------------
bool Str::isDigit()const{
//decimal digit.
	for(const char * p = m_pC; *p; ++p){if(!isdigit(*p))return false;}
	return true;
}										//isDigit
//--------------------------------------------------------------------
bool Str::isGraph()const{
//printing character except space.
	for(const char * p = m_pC; *p; ++p){if(!isgraph(*p))return false;}
	return true;
}										//isGraph
//--------------------------------------------------------------------
bool Str::isLower()const{
//lower-case letter.
	for(const char * p = m_pC; *p; ++p){if(!islower(*p))return false;}
	return true;
}										//isLower
//--------------------------------------------------------------------
bool Str::isUpper()const{
//upper-case letter.
	for(const char * p = m_pC; *p; ++p){if(!isupper(*p))return false;}
	return true;
}										//isUpper
//--------------------------------------------------------------------
bool Str::isPrint()const{
//printing character including space.
	for(const char * p = m_pC; *p; ++p){if(!isprint(*p))return false;}
	return true;
}										//isPrint
//--------------------------------------------------------------------
bool Str::isPunct()const{
//printing character except space, letter or digit.
	for(const char * p = m_pC; *p; ++p){if(!ispunct(*p))return false;}
	return true;
}										//isPunct
//--------------------------------------------------------------------
bool Str::isSpace()const{
//space, formfeed, newline, carriage return, tab, vertical tab.
	for(const char * p = m_pC; *p; ++p){if(!isspace(*p))return false;}
	return true;
}										//isSpace
//--------------------------------------------------------------------
int Str::ConvertMeToNumber(double & d, long & i)const{
//int pasnum(const char * s, double & d, long & i)const
//To use:  int kod = pasnum(sz, uu, nn), where const char * sz, double uu, int nn.
//Returns: 0=error  1=given number "sz" is integer   2=given number "sz" is real.
//When no error: d=the number as a double   i=the same number as an integer.
//From  C:\\...\MSDev\projects\PhysicsSolver1.1\Geom\Geoma.cpp
//sz is of the form "bbb-DDb"  " or "bbbDD.DDe-DDbbb"
	const int MAXDIG(8);
	const int MAXEXP(2);
	if(m_Size == 0)return 0;

	//Read sign into ia.
	int p = 0;
	int ia = 1;
	while((p < m_Size) && (m_pC[p] == ' ')){
		++p;
	}
	if(p == m_Size)return 0;

	//Octal or exadecimal format.
	if(m_pC[p] == '0'){
		i = strtol(m_pC, (char**)NULL, 0);
		d = i;
		if(errno != 0)return 0; //Overflow or other error condition.
		return 1;  //Conversion to integer suceeded.
	}

	//Integer or real.
	if(m_pC[p] == '+'){
		++p;
	}
	if(m_pC[p] == '-'){
		++p;
		ia = -1;
	}
	while((p < m_Size) && (m_pC[p] == ' ')){
		++p;
	}
	if(p == m_Size)return 0;

	//Read integer part as "countb" digits into "ib".
	char c[2];
	c[1] = '\0';
	unsigned long ib = 0;
	int countb = 0;
	while((p < m_Size) && isdigit(m_pC[p])){
		c[0] = m_pC[p];
		if(MAXDIG == countb++)return 0;
		ib = 10 * ib + atoi(c);
		++p;
	}
	if((p == m_Size) || (m_pC[p] == ' ')){
		if(countb == 0)return 0;
		i = ib;
		if(ia == -1)i = -i;
		d = double(i);
		return 1;
	}

	//Read decimal point, and decimal part as "countc" digits into "ic".
	if(m_pC[p++] != '.')return 0;
	unsigned long ic = 0;
	int countc = 0;
	while((p < m_Size) && isdigit(m_pC[p])){
		c[0] = m_pC[p];
		if(MAXDIG == countc++)return 0;
		ic = 10 * ic + atoi(c);
		++p;
	}
	if((p == m_Size) || (m_pC[p] == ' ')){
		if(countb == 0 && countc == 0)return 0;
		d = pow(10.0, -countc) * (double)ic + (double)ib;
		if(ia == -1)d = -d;
		i = (long)d;
		return 2;
	}

	//Read "e" or "E", and exponent sign into "id".
	if((m_pC[p] != 'e') && (m_pC[p] != 'E'))return 0;
	p++;
	if(p == m_Size)return 0;
	int id = 1;
	if(m_pC[p] == '+'){
		++p;
	}
	if(m_pC[p] == '-'){
		id = -1;
		++p;
	}
	if(p == m_Size)return 0;

	//Read exponent value as "counte" digits into "ie".
	int ie = 0;
	int counte = 0;
	while((p < m_Size) && isdigit(m_pC[p])){
		c[0] = m_pC[p];
		if(MAXEXP == counte++)return 0;
		ie = 10 * ie + atoi(c);
		++p;
	}
	if((p < m_Size) && (m_pC[p] != ' '))return 0;
	if(counte == 0)return 0;
	
	//Synthesize number.
	d = pow(10.0, -countc) * (double)ic + (double)ib;
	if(ia == -1)d = -d;
	int ied = ie;
	if(id == -1)ied = -ie;
	d = d * pow(10.0, ied);
	i = long(d);
	return 2;
}										//ConvertMeToNumber
//--------------------------------------------------------------------
bool Str::AtoI(long & ii)const{
	double dd;
	int kod = ConvertMeToNumber(dd, ii);
	return (kod == 1);
}										//AtoI
//--------------------------------------------------------------------
bool Str::AtoF(double & dd)const{
	long ii;
	int kod = ConvertMeToNumber(dd, ii);
	return(kod == 1 || kod == 2);
}										//AtoF
//--------------------------------------------------------------------
bool Str::Verify()const{
	if(m_pC == nullptr || (const unsigned int)m_pC == 0xcccccccc)return false;
	if(m_Size < 0)return false;
	if(m_pC[m_Size] != '\0')return false;
	if(m_Size != int(strlen(m_pC)))return false;
	return true;
}										//Verify
//--------------------------------------------------------------------
Str Str::TSQ()const{
//Replace each single-quotation mark in this STR with a pair of quotation marks.
	char c = '\'';
	int n = CountOccurrences(c);
	if(n == 0)return *this;
	Str rr(m_Size + n);
	char * h = rr.m_pC;
	int k = 0;
	for(int i = 0; i < m_Size; ++i){
		h[k++] = m_pC[i];
		if(m_pC[i] == c)h[k++] = c;
	}
	Debug::Assert(k == m_Size + n, "01142013,7:47");
	return rr;
}										//TSQ
//--------------------------------------------------------------------
Str & Str::ToLower(){
	for(int i = 0; i < m_Size; ++i){
		m_pC[i] = tolower(m_pC[i]);
	}
	return *this;
}										//ToLower
//--------------------------------------------------------------------
Str & Str::ToUpper(){
	for(int i = 0; i < m_Size; ++i){
		m_pC[i] = toupper(m_pC[i]);
	}
	return *this;
}										//ToUpper
//--------------------------------------------------------------------
Str & Str::FillSpaces(char fillChar){
//All spaces, but not tabs, are filled with "fillChar".
	for(int k = 0; k < m_Size; ++k){
		if(m_pC[k] == ' ')m_pC[k] = fillChar;
	}
	return *this;
}										//FillSpaces
//--------------------------------------------------------------------
void Str::ReplaceCharacterWithString(char c, const char * text){
//All occurrences of "c" in this Str are replaced with "text".
//"text" can be empty, in which case all occurrences of "c" are removed.
	int n = CountOccurrences(c);
	if(n == 0)return;
	int L = strlen(text); // L=0 is OK.
	int LP = m_Size + (L - 1) * n; //new length for this Str.
	Str sa(LP);
	int j = 0;
	for(int i = 0; i < m_Size; ++i){
		char z = m_pC[i];
		if(z != c){
			sa.SetAt(j++, z);
		}
		else{
			for(int k = 0; k < L; ++k){
				sa.SetAt(j++, text[k]);
			}
		}
	}
	Debug::Assert(j == LP, "01142013,7:48");
	Create(sa.GetString());
}										//ReplaceCharacterWithString
//--------------------------------------------------------------------
void Str::RemoveAllOccurencesOfCharacter(char c){
//All occurrences of "c" in this Str are removed.
	int n = CountOccurrences(c);
	if(n == 0)return;
	int LP = m_Size - n; //new length for this Str.
	Str sa(LP);
	int j = 0;
	for(int i = 0; i < m_Size; ++i){
		char z = m_pC[i];
		if(z != c){
			sa.SetAt(j++, z);
		}
	}
	Debug::Assert(j == LP, "01142013,7:49");
	Create(sa);
}										//RemoveAllOccurencesOfCharacter
//--------------------------------------------------------------------
bool Str::RightJustify(const Str & data){
//Sets "data" right-justified into this Str, beginning from the left of "data". 
//Any overflow to the right is discarded. Any leftover is filled with spaces.
//"data" can have 0 length.
	int N = data.GetLength();
	if(N < 0)return false;
	Fill(' ');
	int fieldWidth = GetLength();
	int k = fieldWidth - N;
	if(k < 0)k = 0;
	int i = 0;
	while(k < fieldWidth){
		char cc = data.GetAt(i);
		SetAt(k, cc);
		++i;
		++k;
	}
	return true;
}										//RightJustify
//--------------------------------------------------------------------
bool Str::RightJustify(int nn){
//Sets "nn" right-justified into this string. Any leftover on the left is filled with spaces.
//This Str must be large enough to accomodate the number.
	int fieldWidth = GetLength();
	Str sz;
	Debug::Assert(false, "01142013,7:50");  //esto esta mal!!!
	sz.Create(nn);
	int m = sz.GetLength();
	if(m > fieldWidth)return false;
	bool bb = RightJustify(sz);
	return bb;
}										//RightJustify
//--------------------------------------------------------------------
void Str::Fill(char cc)const{
//Fills the Str with cc without changing m_Size.
	Debug::Assert(cc != '\0', "01142013,7:51");
	for(int i = 0; i < m_Size; ++i){
		m_pC[i] = cc;
	}
}										//Fill
//--------------------------------------------------------------------

