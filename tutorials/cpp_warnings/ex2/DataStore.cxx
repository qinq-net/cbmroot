#include "DataStore.h"

#include <iostream>

DataStore::DataStore() 
  : fDataContainer(new DataContainer())
{
}

void DataStore::SetValue(int index, int val) 
{
  fDataContainer->SetValue(index, val);
}
