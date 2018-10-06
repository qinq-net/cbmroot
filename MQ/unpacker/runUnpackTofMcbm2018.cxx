#include "runFairMQDevice.h"
#include "CbmDeviceUnpackTofMcbm2018.h"

#include <string>
#include <iomanip>

namespace bpo = boost::program_options;
using namespace std;

void addCustomOptions(bpo::options_description& options)
{
  options.add_options() ("ReqMode", bpo::value<uint64_t>()->default_value(0),   "Time intervall selector");
  options.add_options() ("ReqTint", bpo::value<uint64_t>()->default_value(100), "Time intervall length in ns");
   
 int iNDet=36;
  for (int i=0; i<iNDet; i++)
    options.add_options() (Form("ReqDet%d",i), bpo::value<uint64_t>()->default_value(0),Form("ReqDet%d",i));
 
}

FairMQDevicePtr getDevice(const FairMQProgOptions& /*config*/)
{
    return new CbmDeviceUnpackTofMcbm2018();
}
