/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             *
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
/*
 * File:   runSamplerRoot.cxx
 * Author: winckler
 *
 * Created on January 15, 2015, 1:57 PM
 */

// FairRoot - FairMQ
#include "FairMQLogger.h"
#include "FairMQProgOptions.h"
#include "GenericSampler.h"
#include "runSimpleMQStateMachine.h" 

// FairRoot - Base/MQ
#include "BoostSerializer.h"
#include "SimpleTreeReader.h"

// boost
 #include <boost/filesystem.hpp>

// CbmRoot
#include "CbmTimeSlice.h"

int InitConfig(FairMQProgOptions& config, int argc, char** argv)
{
    namespace po = boost::program_options;
    po::options_description sampler_options("Sampler options");
    sampler_options.add_options()
        ("event-rate",              po::value<int>()->default_value(0),                    "Event rate limit in maximum number of events per second")
        ("input.file.name",         po::value<std::string>(),                              "Path to the input file")
        ("input.file.tree",         po::value<std::string>()->default_value("cbmsim"),     "Name of the tree")
        ("input.file.branch",       po::value<std::string>()->default_value("TimeSlice."), "Name of the Branch")
    ;

    config.AddToCmdLineOptions(sampler_options);
    config.AddToCfgFileOptions(sampler_options, false);

    if (config.ParseAll(argc, argv, true))
    {
        return 1;
    }

    return 0;
}

// ////////////////////////////////////////////////////////////////////////
typedef SimpleTreeReader<CbmTimeSlice> TreeReader_t;
typedef BoostSerializer<CbmTimeSlice> BoostSerializer_t;
typedef GenericSampler<TreeReader_t, BoostSerializer_t> TimeSliceSampler;

int main(int argc, char** argv)
{
    namespace fs = boost::filesystem;
    try
    {
        FairMQProgOptions config;
        InitConfig(config, argc, argv);

        int eventRate = config.GetValue<int>("event-rate");
        std::string filename = config.GetValue<std::string>("input.file.name");
        std::string treename = config.GetValue<std::string>("input.file.tree");
        std::string branchname = config.GetValue<std::string>("input.file.branch");

        if (!fs::exists(filename))
        {
            MQLOG(ERROR)    << "Input file "
                            << filename
                            << " does not exist. The runStsSampler process will now exit";
            return 1;
        }

        TimeSliceSampler sampler;
        /// configure sampler specific from parsed values
        sampler.SetProperty(TimeSliceSampler::EventRate, eventRate);
        sampler.SetFileProperties(filename, treename, branchname);
        // simple state machine helper function
        runStateMachine(sampler, config);
    }
    catch (std::exception& e)
    {
        MQLOG(ERROR)<< "Unhandled Exception : " 
                    << e.what() 
                    << ", application will now exit";
        return 1;
    }
    return 0;
}
