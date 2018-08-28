#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include <vector> 

// include input and output archivers
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

// include this header to serialize vectors
#include <boost/serialization/vector.hpp>

// include data class
#include "CbmTofDigiExp.h"

using namespace std;

int main()
{


  std::string* strMsg;
  // serialize vector
  {
    std::vector<CbmTofDigiExp*> v;

    v.emplace_back(new CbmTofDigiExp{1, 1., 1.});
    v.emplace_back(new CbmTofDigiExp{2, 2., 2.});
    v.emplace_back(new CbmTofDigiExp{3, 3., 3.});

    std::stringstream oss;
    boost::archive::binary_oarchive oa(oss);
    oa << v;
    strMsg = new std::string(oss.str());
  }

   std::vector<CbmTofDigiExp*> v2;

   // load serialized vector into vector 2
   {
     std::string msgStr(*strMsg);
     std::istringstream iss(msgStr);
     boost::archive::binary_iarchive inputArchive(iss);
     inputArchive >> v2;
   }

   // printout v2 values
   for (auto &d: v2 ) {
      cout << d->ToString() << endl;
   }

  return 0;
}