#include "ExampleClass1.h"

#include <iostream>

ExampleClass1::ExampleClass1()
 : fVal1(1),
   fVal2(2),
   fVal3(),
   fVal4(NULL)
{
  std::cout << "fVal1: " << fVal1 << std::endl;
  std::cout << "fVal2: " << fVal2 << std::endl;
}


