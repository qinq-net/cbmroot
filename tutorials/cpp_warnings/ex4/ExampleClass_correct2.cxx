#include "ExampleClass.h"

#include <iostream>

ExampleClass::ExampleClass()
 : ExampleClass(1,2) 
{
}

ExampleClass::ExampleClass(int _val1)
 : ExampleClass(_val1,2)
{
}

ExampleClass::ExampleClass(int _val1, int _val2)
 : fVal1(_val1),
   fVal2(_val2),
   fVal3()
{
  std::cout << "fVal1: " << fVal1 << std::endl;
  std::cout << "fVal2: " << fVal2 << std::endl;
}


