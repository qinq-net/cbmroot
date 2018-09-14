#include "runFairMQDevice.h"
#include "CbmDeviceHitBuilderTof.h"

namespace bpo = boost::program_options;

void addCustomOptions(bpo::options_description& options)
{
  options.add_options() ("OutRootFile", bpo::value<std::string>()->default_value(""),"Root output file name");
  options.add_options() ("MaxEvent", bpo::value<int64_t>()->default_value(-1),"Nr of Event for ROOT file");
  options.add_options() ("RunId",    bpo::value<int64_t>()->default_value(11),"Run identifier for ROOT geometry file");
 
  options.add_options() ("DutType", bpo::value<uint64_t>()->default_value(9),"Dut type identifier");
  options.add_options() ("DutSm",   bpo::value<uint64_t>()->default_value(0),"Dut module number");
  options.add_options() ("DutRpc",  bpo::value<uint64_t>()->default_value(0),"Dut counter number");

  options.add_options() ("SelType", bpo::value<uint64_t>()->default_value(9),"Sel type identifier");
  options.add_options() ("SelSm",   bpo::value<uint64_t>()->default_value(1),"Sel module number");
  options.add_options() ("SelRpc",  bpo::value<uint64_t>()->default_value(0),"Sel counter number");

  options.add_options() ("Sel2Type", bpo::value<uint64_t>()->default_value(9),"Sel2 type identifier");
  options.add_options() ("Sel2Sm",   bpo::value<uint64_t>()->default_value(0),"Sel2 module number");
  options.add_options() ("Sel2Rpc",  bpo::value<uint64_t>()->default_value(1),"Sel2 counter number");

  options.add_options() ("BRefType", bpo::value<uint64_t>()->default_value(9),"Beam Reference type identifier");
  options.add_options() ("BRefSm",   bpo::value<uint64_t>()->default_value(1),"Beam Reference module number");
  options.add_options() ("BRefDet",  bpo::value<uint64_t>()->default_value(1),"Beam Reference counter number");

}

FairMQDevicePtr getDevice(const FairMQProgOptions& /*config*/)
{
    return new CbmDeviceHitBuilderTof();
}
