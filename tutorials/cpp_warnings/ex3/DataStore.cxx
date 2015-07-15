#include "DataStore.h"

#include <iostream>

DataStore::DataStore() 
  : fValue(1), 
    fSecondValue(fValue+4), 
    fPointerValue(&fSecondValue)
{
}

/*
  DataStore::DataStore() 
  : fSecondValue(fValue+4), 
    fValue(1), 
    fPointerValue(&fSecondValue) 
    {
    }
*/

void DataStore::SetPointerValue(int _val) 
{ 
    fPointerValue = &_val; 
    std::cout << "PointerValue in Setter: " << *fPointerValue << std::endl;
}

void DataStore::SetPointerValueCorrect(int _val) 
{ 
  fSecondValue = _val; 
  fPointerValue = &fSecondValue; 
  std::cout << "PointerValue in Setter: " << *fPointerValue << std::endl;
}

