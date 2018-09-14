#include "runFairMQDevice.h"
#include "CbmDeviceUnpackTofMcbm2018.h"

#include <string>
#include <iomanip>

namespace bpo = boost::program_options;
using namespace std;

void addCustomOptions(bpo::options_description& options)
{
  options.add_options() ("ReqMode", bpo::value<uint64_t>()->default_value(0), "Time intervall selector");
   
 int iNDet=25;
  for (int i=0; i<iNDet; i++)
    options.add_options() (Form("ReqDet%d",i), bpo::value<uint64_t>()->default_value(0),Form("ReqDet%d",i));
  /*
  options.add_options() ("ReqDet0", bpo::value<uint64_t>()->default_value(0),"ReqDet0");
  options.add_options() ("ReqDet1", bpo::value<uint64_t>()->default_value(0),"ReqDet1");
  options.add_options() ("ReqDet2", bpo::value<uint64_t>()->default_value(0),"ReqDet2");
  options.add_options() ("ReqDet3", bpo::value<uint64_t>()->default_value(0),"ReqDet3");
  options.add_options() ("ReqDet4", bpo::value<uint64_t>()->default_value(0),"ReqDet4");
  options.add_options() ("ReqDet5", bpo::value<uint64_t>()->default_value(0),"ReqDet5");
  options.add_options() ("ReqDet6", bpo::value<uint64_t>()->default_value(0),"ReqDet6");
  options.add_options() ("ReqDet7", bpo::value<uint64_t>()->default_value(0),"ReqDet7");
  options.add_options() ("ReqDet8", bpo::value<uint64_t>()->default_value(0),"ReqDet8");
  options.add_options() ("ReqDet9", bpo::value<uint64_t>()->default_value(0),"ReqDet9");
  */
}

FairMQDevicePtr getDevice(const FairMQProgOptions& /*config*/)
{
    return new CbmDeviceUnpackTofMcbm2018();
}
