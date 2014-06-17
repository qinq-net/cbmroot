//////////////////////////////////////////////////////////////////////
/// (C)opyright 2004
/// 
/// Institute of Computer Science V
/// Prof. M�nner
/// University of Mannheim, Germany
/// 
/// *******************************************************************
/// 
/// Designer(s):   Steinle
/// 
/// *******************************************************************
/// 
/// Project:     Trackfinder for CBM-Project at GSI-Darmstadt, Germany
/// 
/// *******************************************************************
/// 
/// Description:
///
///   class:
///     - template class for handling memory for special vectors of objects
///     - because of being a template class the function-bodies must be
///       in this header file, too. There is actually no compiler which
///       can handle a different cxx-source file for the bodies.
///
///
/// *******************************************************************
///
/// $Author: csteinle $
/// $Date: 2008-08-14 12:32:41 $
/// $Revision: 1.2 $
///
//////////////////////////////////////////////////////////////////////


#ifndef _SPECIALMEMCLASS_H
#define _SPECIALMEMCLASS_H


#include "../../MiscLIB/include/errorHandling.h"
#include <vector>
#include <string>


/* **************************************************************
 * TEMPLATE CLASS specialMemClass								*
 * **************************************************************/

template<class objectClass> class specialMemClass {

private:

	typename std::vector<objectClass>           stackMem;				/**< Memory to store all objects. */
	typename std::vector<objectClass>::iterator activeObjectPointer;	/**< Iterator to access one object. */

/**
 * method returns the iterator for the last element in memory
 */

	typename std::vector<objectClass>::iterator getLastIterator();

/**
 * method pushes the element at the end of the memory.
 */

	typename std::vector<objectClass>::iterator pushIntern(objectClass element);

/**
 * method returns the last entry and deletes it
 */

	objectClass popIntern();

public:

/**
 * Default constructor
 */

	specialMemClass();

/**
 * Constructor
 */

	specialMemClass(const specialMemClass<objectClass>& value);

/**
 * Destructor
 */

	~specialMemClass();

/**
 * operator = ()
 */

	specialMemClass<objectClass>& operator =(const specialMemClass<objectClass>& value);

/**
 * method returns the number of entries	
 */

	unsigned long getNumberOfEntries();
	unsigned long size();

/**
 * method pushes the element at the end of the memory.
 */

	void push(objectClass element);

/**
 * method returns the last entry and deletes it
 */

	objectClass pop();

/**
 * method returns the first entry and deletes it
 */

	objectClass popF();

/**
 * method returns if the memory is empty
 */

	bool isEmpty();

/**
 * sets the activeObject to the initial value
 */

	void resetActiveObject();

/**
 * returns a reference of the activeObject
 */

	objectClass readActiveObject();

/**
 * set the next one to the active object.
 */

	void makeNextOneActive();

/**
 * returns a reference of the activeObject and set the next one
 * to the active object.
 */

	objectClass readActiveObjectAndMakeNextOneActive();

/**
 * erases the activeObject
 */

	void eraseActiveObject();

/**
 * method clears the memory
 */

	void clear();

/**
 * This method returns the size of the reserved memory for
 * the source data.
 * @param dimension formats the returnvalue to B, kB, MB or GB
 */

	double getReservedSizeOfData(unsigned short dimension = 0);

/**
 * This method returns the size of the allocated memory for
 * the source data.
 * @param dimension formats the returnvalue to B, kB, MB or GB
 */

	double getAllocatedSizeOfData(unsigned short dimension = 0);

/**
 * This method returns the size of the used memory for
 * the source data.
 * @param dimension formats the returnvalue to B, kB, MB or GB
 */

	double getUsedSizeOfData(unsigned short dimension = 0);

};


/* **************************************************************
 * TEMPLATE CLASS specialMemClass									*
 * **************************************************************/


 /**
 * method returns the iterator for the last element in memory
 */

 template<class objectClass> typename std::vector<objectClass>::iterator specialMemClass<objectClass>::getLastIterator() {

 	typename std::vector<objectClass>::iterator lastIterator;

	lastIterator = stackMem.end();
	
	if (!isEmpty())
		lastIterator--;

	return lastIterator;

 }

 /**
 * method pushes the element at the end of the memory.
 */

template<class objectClass> typename std::vector<objectClass>::iterator specialMemClass<objectClass>::pushIntern(objectClass element) {

	typename std::vector<objectClass>::iterator returnValue;

	stackMem.push_back(element);
	returnValue = getLastIterator();

	return returnValue;

}


/**
 * method returns the last entry and deletes it
 */

template<class objectClass> objectClass specialMemClass<objectClass>::popIntern() {

	objectClass returnValue;

	returnValue = stackMem.back();
	stackMem.pop_back();

	return returnValue;

}

/**
 * Default constructor
 */

template<class objectClass> specialMemClass<objectClass>::specialMemClass() : stackMem(), activeObjectPointer() {

	stackMem.clear();
	resetActiveObject();

}

/**
 * Constructor
 */

template<class objectClass> specialMemClass<objectClass>::specialMemClass(const specialMemClass<objectClass>& value) : stackMem(value.stackMem), activeObjectPointer(value.activeObjectPointer) {

  //	*this = value;

}

/**
 * Destructor
 */

template<class objectClass> specialMemClass<objectClass>::~specialMemClass() {

	stackMem.clear();

}

/**
 * operator = ()
 */

template<class objectClass> specialMemClass<objectClass>& specialMemClass<objectClass>::operator=(const specialMemClass<objectClass>& value) {

	typename std::vector<objectClass>::iterator iterator;
	unsigned long                               difference;

	stackMem.clear();
	stackMem                = value.stackMem;

	iterator                = value.activeObjectPointer;
	difference              = 0;

	while (iterator != value.stackMem.begin()) {

		difference++;
		iterator--;

	}

	activeObjectPointer     = stackMem.begin();

	for (unsigned long i = 0; i < difference; i++)
		makeNextOneActive();

	return *this;

}

/**
 * method returns the number of entries
 */

template<class objectClass> unsigned long specialMemClass<objectClass>::getNumberOfEntries() {

	typename std::vector<objectClass>::size_type numberOfEntries;

	numberOfEntries = stackMem.size();

	return (unsigned long) numberOfEntries;

}
template<class objectClass> unsigned long specialMemClass<objectClass>::size() {

	return getNumberOfEntries();

}

/**
 * method pushes the element at the end of the memory.
 */

template<class objectClass> void specialMemClass<objectClass>::push(objectClass element) {

	activeObjectPointer = pushIntern(element);

}


/**
 * method returns the last entry and deletes it
 */

template<class objectClass> objectClass specialMemClass<objectClass>::pop() {

	objectClass returnValue;

	if ((activeObjectPointer == getLastIterator()) && (!isEmpty()))
		activeObjectPointer--;

	returnValue = popIntern();

	return returnValue;

}

/**
 * method returns the first entry and deletes it
 */

template<class objectClass> objectClass specialMemClass<objectClass>::popF() {

	objectClass returnValue;

	if ((activeObjectPointer == stackMem.begin()) && (!isEmpty()))
		makeNextOneActive();

	returnValue = stackMem.front();
	stackMem.pop_front();

	return returnValue;

}

/**
 * method returns if the memory is empty
 */

template<class objectClass> bool specialMemClass<objectClass>::isEmpty() {

	return stackMem.empty();

}

/**
 * sets the activeObject to the initial value
 */

template<class objectClass> void specialMemClass<objectClass>::resetActiveObject() {

	activeObjectPointer = stackMem.begin();

}

/**
 * returns a reference of the activeObject
 */

template<class objectClass> objectClass specialMemClass<objectClass>::readActiveObject() {

	if (!isEmpty())
		return *activeObjectPointer;
	else
		throw cannotReadEmptyMemoryError(DATAOBJECTLIB);

}

/**
 * set the next one to the active object.
 */

template<class objectClass> void specialMemClass<objectClass>::makeNextOneActive() {

	if (activeObjectPointer == getLastIterator())
		resetActiveObject();
	else
		activeObjectPointer++;

}

/**
 * returns a reference of the activeObject and set the next one
 * to the active object.
 */

template<class objectClass> objectClass specialMemClass<objectClass>::readActiveObjectAndMakeNextOneActive() {

	objectClass returnValue;

	returnValue = readActiveObject();

	makeNextOneActive();

	return returnValue;

}


/**
 * erases the activeObject
 */

template<class objectClass> void specialMemClass<objectClass>::eraseActiveObject() {

	typename std::vector<objectClass>::iterator position;

	if (activeObjectPointer != stackMem.end()) {

		position = activeObjectPointer;

		makeNextOneActive();
	
		stackMem.erase(position);

	}

}

/**
 * method clears the memory
 */

template<class objectClass> void specialMemClass<objectClass>::clear() {

	stackMem.clear();
	resetActiveObject();

}

/**
 * This method returns the size of the reserved memory for
 * the source data.
 */

template<class objectClass> double specialMemClass<objectClass>::getReservedSizeOfData(unsigned short dimension) {

	double returnValue;

	returnValue  = sizeof(stackMem);
	returnValue += sizeof(activeObjectPointer);

	returnValue  = (returnValue / (1 << (10 * dimension)));

	return returnValue;

}

/**
 * This method returns the size of the allocated memory for
 * the source data.
 */

template<class objectClass> double specialMemClass<objectClass>::getAllocatedSizeOfData(unsigned short dimension) {

	typename    std::vector<objectClass>::iterator restorePointer;
	objectClass object;
	double      returnValue;

	returnValue  = 0;

	restorePointer = activeObjectPointer;

	resetActiveObject();
	for (unsigned long i = 0; i < getNumberOfEntries(); i++) {
			
		object       = readActiveObjectAndMakeNextOneActive();
		returnValue += object.getReservedSizeOfData(0);
		returnValue += object.getAllocatedSizeOfData(0);

	}

	returnValue += (stackMem.capacity() - getNumberOfEntries()) * sizeof(objectClass);

	activeObjectPointer = restorePointer;

	returnValue  = (returnValue / (1 << (10 * dimension)));

	return returnValue;

}

/**
 * This method returns the size of the used memory for
 * the source data.
 */

template<class objectClass> double specialMemClass<objectClass>::getUsedSizeOfData(unsigned short dimension) {

	typename    std::vector<objectClass>::iterator restorePointer;
	objectClass object;
	double      returnValue;

	returnValue  = 0;

	restorePointer = activeObjectPointer;

	resetActiveObject();
	for (unsigned long i = 0; i < getNumberOfEntries(); i++) {
			
		object       = readActiveObjectAndMakeNextOneActive();
		returnValue += object.getUsedSizeOfData(0);

	}

	activeObjectPointer = restorePointer;

	returnValue += sizeof(activeObjectPointer);

	returnValue  = (returnValue / (1 << (10 * dimension)));

	return returnValue;

}

#endif

