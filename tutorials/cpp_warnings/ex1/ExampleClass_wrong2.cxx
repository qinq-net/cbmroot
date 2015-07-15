#include "ExampleClass.h"

#include <iostream>

ExampleClass::ExampleClass()
 : fVal2(2),
   fVal1(1),
   fVal3()
{
  std::cout << "fVal1: " << fVal1 << std::endl;
  std::cout << "fVal2: " << fVal2 << std::endl;
}


