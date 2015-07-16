#include <vector>
 
#include <iostream>
 
class ExampleClass1
{

 public:
 
  void PrintValues()
  {
    std::cout << "fVal1: " << fVal1 << std::endl;
    std::cout << "fVal2: " << fVal2 << std::endl;
  }

 private:

  int fVal1;
  int fVal2;

  std::vector<int> fVal3;
  
};

