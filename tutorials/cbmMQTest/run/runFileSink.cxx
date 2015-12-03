
// FairRoot - FairMQ
#include "GenericFileSink.h"
#include "runSimpleMQStateMachine.h"
#include "FairMQProgOptions.h"
#include "FairMQLogger.h"

// FairRoot - base/MQ
#include "RootOutFileManager.h"
#include "RootSerializer.h"

// CbmRoot
#include "CbmStsCluster.h"


typedef RootOutFileManager<CbmStsCluster> CbmStsClusterStorage_t;
typedef GenericFileSink<RootDeSerializer, CbmStsClusterStorage_t> StsClusterFileSink;


inline int InitConfig(FairMQProgOptions& config,int argc, char** argv)
{
    namespace po = boost::program_options;
    po::options_description sink_options("File Sink options");
    sink_options.add_options()
        ("output.file.name",    po::value<std::string>(),                                 "Path to the input file")
        ("output.file.tree",    po::value<std::string>()->default_value("Cbmout"),        "Name of the output tree")
        ("output.file.branch",  po::value<std::string>()->default_value("CbmStsCluster"), "Name of the output Branch")
        ("output.file.option",  po::value<std::string>()->default_value("RECREATE"),      "Root file option : UPDATE, RECREATE etc.")
        ("cluster-classname",   po::value<std::string>()->default_value("CbmStsCluster"), "Cluster class name for initializing TClonesArray")
    ;

    config.AddToCmdLineOptions(sink_options);
    config.AddToCfgFileOptions(sink_options, false);

    if (config.ParseAll(argc, argv, true))
    {
        return 1;
    }

    return 0;
}


int main(int argc, char** argv)
{
    try
    {
        FairMQProgOptions config;
        InitConfig(config, argc, argv);

        std::string filename = config.GetValue<std::string>("output.file.name");
        std::string treename = config.GetValue<std::string>("output.file.tree");
        std::string branchname = config.GetValue<std::string>("output.file.branch");
        std::string fileoption = config.GetValue<std::string>("output.file.option");
        std::string clustername = config.GetValue<std::string>("cluster-classname");

        StsClusterFileSink sink;
        // call function member from storage policy
        sink.SetFileProperties(filename, treename, branchname, clustername, fileoption, true);
        // call function member from deserialization policy
        sink.InitInputContainer(clustername);
        runStateMachine(sink, config);
    }
    catch (std::exception& e)
    {
        MQLOG(ERROR)  << "Unhandled Exception : " 
                    << e.what() << ", application will now exit";
        return 1;
    }

    return 0;
}

