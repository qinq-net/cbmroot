#include "DataContainer.h"

class DataStore
{

 public:

  DataStore();

  virtual ~DataStore() { delete fDataContainer; }

  int GetValue(int index) { return fDataContainer->GetValue(index); }
  void PrintAllValues() { fDataContainer->PrintAllValues(); }
  
  void SetValue(int index, int val); 

 private:

  DataContainer* fDataContainer;
};

