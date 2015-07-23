#include <vector>
 
class ExampleClass1
{

 public:

  ExampleClass1();

 private:

  int fVal1;
  int fVal2;

  std::vector<int> fVal3;

  int* fVal4;  
  
  ExampleClass1(const ExampleClass1&);
  ExampleClass1& operator=(const ExampleClass1&);
  
};

