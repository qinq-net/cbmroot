#include "DataStore.h"

#include <iostream>

int main() 
{

  DataStore a;

  std::cout << std::endl;
  std::cout << "---- After Constructor -----" << std::endl;
  std::cout << "Value of a: " << a.GetValue() << std::endl;
  std::cout << "PointerValue of a: " << a.GetPointerValue() << std::endl;
  std::cout << std::endl;
  
  std::cout << "---- After first setter -----" << std::endl;
  a.SetValue(111);
  a.SetPointerValue(42);
  std::cout << "Value of a: " << a.GetValue() << std::endl;
  std::cout << "PointerValue of a: " << a.GetPointerValue() << std::endl;
  std::cout << std::endl;
  
  std::cout << "---- After second setter -----" << std::endl;
  a.SetPointerValueCorrect(43);
  std::cout << "PointerValue of a: " << a.GetPointerValue() << std::endl;
  std::cout << std::endl;

  std::cout << "---- After copy constructor -----" << std::endl;
  DataStore b(a);
  std::cout << "Value of b: " << b.GetValue() << std::endl;
  std::cout << "PointerValue of b: " << b.GetPointerValue() << std::endl;
  std::cout << std::endl;

  std::cout << "---- After assignment operator -----" << std::endl;
  DataStore c = a;
  std::cout << "Value of c: " << c.GetValue() << std::endl;
  std::cout << "PointerValue of c: " << c.GetPointerValue() << std::endl;
  std::cout << std::endl;

  std::cout << "---- After changing value of a -----" << std::endl;
  a.SetPointerValueCorrect(-111);
  a.SetValue(-42);
  std::cout << "Value of a: " << a.GetValue() << std::endl;
  std::cout << "PointerValue of a: " << a.GetPointerValue() << std::endl;
  std::cout << std::endl;

  std::cout << "Value of b: " << b.GetValue() << std::endl;
  std::cout << "PointerValue of b: " << b.GetPointerValue() << std::endl;
  std::cout << std::endl;

  std::cout << "Value of c: " << c.GetValue() << std::endl;
  std::cout << "PointerValue of c: " << c.GetPointerValue() << std::endl;
 
}
