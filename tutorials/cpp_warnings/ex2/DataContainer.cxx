#include "DataContainer.h"

#include <iostream>

DataContainer::DataContainer() 
  : fValue(new int[maxIndex]) 
{
  for (int i=0; i < maxIndex; ++i) {
    fValue[i] = i;
  }
} 


int DataContainer::GetValue(int i) 
{ 
  if ( (i >= 0) && (i < maxIndex) ) {
    return fValue[i]; 
  } else {
    std::cout << "Index " << i << " out of bounds. Return 0" << std::endl;
    return 0;
  }
}

void DataContainer::SetValue(int index, int val) 
{ 
  if ( (index >= 0) && (index < maxIndex) ) {
    fValue[index] = val;  
  } else {
    std::cout << "Index " << index << " out of bounds." 
	      << " Can't set the value." << std::endl;
  }
}

void DataContainer::PrintAllValues() 
{ 
  int counter = 0;
  for (int i=0; i < maxIndex; ++i) {
    std::cout << fValue[i] << " ";
    int bla = counter%10; 
    if (9 == bla) {
      std::cout << std::endl;
    }
    counter++;
  }
  std::cout << std::endl;
}
