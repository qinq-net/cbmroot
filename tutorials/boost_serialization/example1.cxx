#include <iostream>
#include <fstream>
#include <vector>

// include input and output archivers
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

// include this header to serialize vectors
#include <boost/serialization/vector.hpp>

using namespace std;



int main()
{

  std::vector<double> v = {1,2,3.4, 5.6};

  // serialize vector
  {
    std::ofstream ofs("/tmp/copy.ser");
    boost::archive::text_oarchive oa(ofs);
    oa & v;
  }

   std::vector<double> v2;

   // load serialized vector into vector 2
   {
     std::ifstream ifs("/tmp/copy.ser");
     boost::archive::text_iarchive ia(ifs);
     ia & v2;
   }

   // printout v2 values
   for (auto &d: v2 ) {
      std::cout << d << endl;
   }


  return 0;
}