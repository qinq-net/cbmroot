#include <vector>
 
class ExampleClass
{

 public:

  ExampleClass();
  ExampleClass(int _val1);
  ExampleClass(int _val1, int _val2);


 private:

  int fVal1;
  int fVal2;

  std::vector<int> fVal3;
  
};

