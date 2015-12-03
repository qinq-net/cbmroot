

/// FairRoot - FairMQ 
#include "FairMQLogger.h"
#include "FairMQProgOptions.h"
#include "GenericProcessor.h"
#include "runSimpleMQStateMachine.h"

/// FairRoot - base/MQ
#include "BoostSerializer.h"
#include "RootSerializer.h"

#include "CbmClusterFinderTask.h"
#include "CbmStsFindClusters.h"
#include "CbmTimeSlice.h"


// ////////////////////////////////////////////////////////////////////////
inline int InitConfig(FairMQProgOptions& config, int argc, char** argv)
{
    namespace po = boost::program_options;
    po::options_description processor_options("processor options");
    processor_options.add_options()
        ("sts-geo-filename", po::value<std::string>()->required(), "path to the the STS geometry file.")
    ;

    config.AddToCmdLineOptions(processor_options);
    config.AddToCfgFileOptions(processor_options, false);
    if (config.ParseAll(argc, argv, true))
    {
        return 1;
    }
    return 0;
}

typedef BoostDeSerializer<CbmTimeSlice,CbmTimeSlice> BoostDeSerializer_t;
typedef CbmClusterFinderTask<CbmStsFindClusters> CbmStsClusterFinderTask_t;
typedef GenericProcessor<BoostDeSerializer_t,RootSerializer,CbmStsClusterFinderTask_t> CbmClusterFinderDevice;

int main(int argc, char** argv)
{
    try
    {
        CbmClusterFinderDevice processor;
        FairMQProgOptions config;
        InitConfig(config, argc, argv);
        std::string geo_filename = config.GetValue<std::string>("sts-geo-filename");
        processor.InitClusterFinderTask(geo_filename);
        runStateMachine(processor, config);

    }
    catch (std::exception& e)
    {
        MQLOG(ERROR)  << "Unhandled Exception : " 
                    << e.what() << ", application will now exit";
        return 1;
    }

    return 0;
}
