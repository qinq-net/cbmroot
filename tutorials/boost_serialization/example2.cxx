#include <iostream>
#include <fstream>
#include <vector>

// include input and output archivers
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

// include this header to serialize vectors
#include <boost/serialization/vector.hpp>

// include data class
#include "CbmTofDigiExp.h"

using namespace std;

int main()
{

  // serialize vector
  {
    std::vector<CbmTofDigiExp> v;

    v.emplace_back(CbmTofDigiExp{1, 1., 1.});
    v.emplace_back(CbmTofDigiExp{2, 2., 2.});
    v.emplace_back(CbmTofDigiExp{3, 3., 3.});

    std::ofstream ofs("/tmp/copy.ser");
    boost::archive::text_oarchive oa(ofs);
    oa & v;
  }

   std::vector<CbmTofDigiExp> v2;

   // load serialized vector into vector 2
   {
     std::ifstream ifs("/tmp/copy.ser");
     boost::archive::text_iarchive ia(ifs);
     ia & v2;
   }

   // printout v2 values
   for (auto &d: v2 ) {
      cout << d.ToString() << endl;
   }

  return 0;
}