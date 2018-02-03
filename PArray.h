#pragma once
#include "MyIncludes.h"
/*--------------------------------------------------------------------
This container is a homogeneous, parameterized array of fixed size that keeps 
user objects of type UTYPE in array elements and owns the objects it contains. 
Use it for atomic types such as int, double, char, bool, or for user objects 
that have a default constructor. It is not recommended for large user objects.
For pointers to user objects, prefer BArray, which is a non-owner container.
(However, it should be possible to use a PArray of *pointers* to objects).
Operator= must be defined for TYPE.
//------------------------------------------------------------------*/
template <class TYPE> class PArray{
protected:
	TYPE * m_pType;
	int m_Size; //This is the actual size of the array. 
public:
	PArray();
	PArray(int size);
	PArray(const TYPE * t, int nbrElements);
	PArray(const PArray & other);
	//PArray(const PArray & other, int start, int nElements);
	virtual ~PArray();
	void SetAt(int index, const TYPE & t)const;
	void SetArraySize(int newSize); //NO! Why not?
	bool ExpandArraySize(int delta); //Constructs a longer array and copies all user objects to it.
	bool ReduceArraySize(int delta);
	bool AppendArray(const PArray & other); //Expands this array by attaching "other" to the end.
	int          GetSize()const{return m_Size;}
	const TYPE & GetAt(int index)const;
	const TYPE & GetFirst()const;
	const TYPE & GetLast()const;
	const TYPE * GetArray()const;
	TYPE *       GetNonconstantArray()const;
	bool IsEmpty()const{return (m_Size == 0);}
	bool IsInRange(int index)const{return (index >= 0 && index < m_Size);}
	void Copy(const PArray & other);
	void ReverseMe();
	void RotateMe(int nSteps);
	bool ShiftElement(int fromIndex, int toIndex)const;
	TYPE & operator[](int index)const;
	operator const TYPE * ()const;
};										//Class PArray
//--------------------------------------------------------------------
template <class TYPE>
PArray<TYPE>::PArray() : m_pType(nullptr){
	m_Size = 0;
}										//Default constructor
//--------------------------------------------------------------------
template <class TYPE>
PArray<TYPE>::PArray(int size) : m_pType(nullptr){
	Debug::Assert(size >= 0, "01142013:6:05");
	m_Size = size;
	m_pType = new TYPE[m_Size]; //Assumes TYPE has a default constructor.
}										//Constructor
//--------------------------------------------------------------------
template <class TYPE>
PArray<TYPE>::PArray(const TYPE * t, int nbrElements) : m_pType(nullptr){
	Debug::Assert(nbrElements >= 0, "01142013:6:06");
	m_Size = nbrElements;
	m_pType = new TYPE[m_Size]; //Assumes TYPE has a default constructor.
	for(int i = 0; i < m_Size; i++)m_pType[i] = t[i]; //Assumes TYPE has an operator =.
}										//Constructor
//--------------------------------------------------------------------
template <class TYPE>
PArray<TYPE>::PArray(const PArray & other) : m_pType(nullptr){
//A copy constructor is required for the binary operators +-*/ to work.
	m_Size = other.m_Size;
	m_pType = new TYPE[m_Size]; //Assumes TYPE has a default constructor.
	for(int i = 0; i < m_Size; i++){
		m_pType[i] = other[i]; //Assumes TYPE has an operator =.
	}
}										//Copy constructor
//--------------------------------------------------------------------
template <class TYPE>
PArray<TYPE>::~PArray(){
	delete [] m_pType; //Calls the destructor of m_pType for each array element.
	m_pType = nullptr;
	m_Size = 0;
}										//Destructor
//--------------------------------------------------------------------
template <class TYPE>
void PArray<TYPE>::SetArraySize(int newSize){
//All user objects are deleted, if any, and a new array is constructed.
	Debug::Assert(newSize >= 0, "01142013:6:07");
	m_Size = newSize;
	delete [] m_pType;
	m_pType = new TYPE[m_Size]; //Assumes TYPE has a default constructor.
}										//SetArraySize
//--------------------------------------------------------------------
template <class TYPE>
bool PArray<TYPE>::ExpandArraySize(int delta){
//A longer array is constructed, and all user objects are copied to it.
	if(delta < 0)return false;
	if(delta == 0)return true;
	int newSize = m_Size + delta;
	TYPE * p = new TYPE[newSize];
	for(int i = 0; i < m_Size; ++i){
		p[i] = m_pType[i];
	}
	for (int i = m_Size; i < newSize; ++i) {
		p[i] = nullptr; //Set null pointers in unused spaces.
	}
	delete [] m_pType; //Calls the destructor of m_pType for each array element.
	m_pType = p;
	m_Size = newSize;
	return true;
}										//ExpandArraySize
//--------------------------------------------------------------------
template <class TYPE>
bool PArray<TYPE>::ReduceArraySize(int delta){
//A shorter array is constructed, and all user objects are copied to it.
	if(delta < 0)return false;
	if(delta == 0)return true;
	int newSize = m_Size - delta;
	if(newSize < 0)return false;
	TYPE * p = new TYPE[newSize];
	for(int i = 0; i < newSize; ++i){
		p[i] = m_pType[i];
	}
	delete [] m_pType; //Calls the destructor of m_pType for each array element.
	m_pType = p;
	m_Size = newSize;
	return true;
}										//ReduceArraySize
//--------------------------------------------------------------------
template <class TYPE>
bool PArray<TYPE>::AppendArray(const PArray & other){
//Expands this array by attaching "other" to the end.
	int k = m_Size;
	int n = other.GetSize();
	bool bb = ExpandArraySize(n);
	if(!bb)return false;
	const TYPE * p = other.m_pType;
	for(int i = k; i < m_Size; ++i){
		m_pType[i] = p[i];
	}
	return true;
}										//AppendArray
//--------------------------------------------------------------------
template <class TYPE>
void PArray<TYPE>::Copy(const PArray & other){
	m_Size = other.m_Size;
	delete [] m_pType;
	m_pType = nullptr;
	if(m_Size > 0){
		m_pType = new TYPE[m_Size]; //Assumes TYPE has a default constructor.
		for(int i = 0; i < m_Size; i++)m_pType[i] = other.m_pType[i]; //Assumes TYPE has an operator =.
	}
}										//Copy
//--------------------------------------------------------------------
template <class TYPE>
void PArray<TYPE>::ReverseMe(){
	if(m_Size < 2)return;
	int k = int(m_Size / 2);
	for(int i = 0; i < k; ++i){
		int j = m_Size - 1 - i;
		TYPE tt = m_pType[i];
		m_pType[i] = m_pType[j];
		m_pType[j] = tt;
	}
}										//ReverseMe
//--------------------------------------------------------------------
template <class TYPE>
void PArray<TYPE>::RotateMe(int nSteps){
//Performs a circular rotation of the array by |nSteps| steps.
//Leftwise when nSteps > 0, rightwise when nSteps < 0.
	int nn = fabs(nSteps);
	int k = nn - m_Size * int(nn / m_Size);
	if(k == 0)return;
	Debug::Assert(0 < k && k < m_Size, "01142013:6:08");

	TYPE * temp = new TYPE[m_Size];
	int i, j;
	if(nSteps > 0){ //Leftwise.
		for(i = 0, j = k; i < m_Size; ++i, ++j){
			if(j == m_Size)j = 0;
			temp[i] = m_pType[j];
		}
	}
	else{           //Rightwise
		for(i = 0, j = k; i < m_Size; ++i, ++j){
			if(j == m_Size)j = 0;
			temp[j] = m_pType[i];
		}
	}
	delete [] m_pType;
	m_pType = temp;
}										//RotateMe*/
//--------------------------------------------------------------------
template <class TYPE>
bool PArray<TYPE>::ShiftElement(int fromIndex, int toIndex)const{
//Moves the element in position "fromIndex" to position "toIndex". The elements in between
//move one step as a block to free the destination position and occupy the origin position.
	Debug::Assert(IsInRange(fromIndex) && IsInRange(toIndex), "01142013:6:09");
	if(fromIndex == toIndex)return true;
	TYPE t = m_pType[fromIndex];
	int k = 1;
	if(fromIndex > toIndex)k = -1;
	int i = fromIndex;
	do{
		m_pType[i] = m_pType[i + k];
		i = i + k;
	}while(i != toIndex);
	m_pType[toIndex] = t;
	return true;
}										//ShiftElement
//--------------------------------------------------------------------
template <class TYPE>
TYPE & PArray<TYPE>::operator[](int index)const{
	if(!IsInRange(index)){
		Debug::Assert(false, "PArray Error. Index not in range.");
	}
	return m_pType[index];
}										//operator[]
//--------------------------------------------------------------------
template <class TYPE>
const TYPE & PArray<TYPE>::GetAt(int index)const{
                                                                                                                                                                                                                                                                                                                                                                                                                                   	if(!IsInRange(index)){
		Debug::Assert(false, "PArray Error. Negative index.");
	}
	return m_pType[index];
}										//GetAt
//--------------------------------------------------------------------
template <class TYPE>
const TYPE & PArray<TYPE>::GetFirst()const{
	Debug::Assert(m_Size > 0, "01142013:6:00");
	return m_pType[0];
}										//GetFirst
//--------------------------------------------------------------------
template <class TYPE>
const TYPE & PArray<TYPE>::GetLast()const{
	Debug::Assert(m_Size > 0, "01142013:6:01");
	return m_pType[m_Size - 1];
}										//GetLast
//--------------------------------------------------------------------
template <class TYPE>
void PArray<TYPE>::SetAt(int index, const TYPE & t)const{
	if(!IsInRange(index))Debug::Assert(false, "PArray Error. Negative index.");
	m_pType[index] = t;
}										//SetAt
//--------------------------------------------------------------------
template <class TYPE>
const TYPE * PArray<TYPE>::GetArray()const{
	Debug::Assert(m_pType != nullptr, "01142013:6:02");
	return m_pType;
}										//GetArray
//--------------------------------------------------------------------
template <class TYPE>
TYPE * PArray<TYPE>::GetNonconstantArray()const{
	Debug::Assert(m_pType != nullptr, "01142013:6:03");
	return m_pType;
}										//GetNonconstantArray
//--------------------------------------------------------------------
template <class TYPE>
PArray<TYPE>::operator const TYPE * ()const{
//Type conversion operator. It allows an object of this class to be used just like an array.
//Example: const TYPE * cc = TYPE-object. Also as argument to function that takes a const TYPE *.
	Debug::Assert(m_pType != nullptr, "01142013:6:04");
	return m_pType;
}										//operator const TYPE *
//--------------------------------------------------------------------



