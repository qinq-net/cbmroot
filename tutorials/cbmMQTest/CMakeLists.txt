

Set(INCLUDE_DIRECTORIES
    ${CBMROOT_SOURCE_DIR}/fairmq
    ${CBMROOT_SOURCE_DIR}/MQ
    ${CBMROOT_SOURCE_DIR}/fairmq/zeromq
    ${CBMROOT_SOURCE_DIR}/base
    ${CBMROOT_SOURCE_DIR}/run
    ${CBMROOT_SOURCE_DIR}/tutorials/cbmMQTest
    ${CBMROOT_SOURCE_DIR}/tutorials/cbmMQTest/devices
    ${CBMROOT_SOURCE_DIR}/tutorials/cbmMQTest/devices/policies
    ${CBMROOT_SOURCE_DIR}/data
    ${CBMROOT_SOURCE_DIR}/data/beamtime
    ${CBMROOT_SOURCE_DIR}/data/beamtime/fhodo
    ${CBMROOT_SOURCE_DIR}/data/ecal
    ${CBMROOT_SOURCE_DIR}/data/global
    ${CBMROOT_SOURCE_DIR}/data/much
    ${CBMROOT_SOURCE_DIR}/data/mvd
    ${CBMROOT_SOURCE_DIR}/data/psd
    ${CBMROOT_SOURCE_DIR}/data/rich
    ${CBMROOT_SOURCE_DIR}/data/sts
    ${CBMROOT_SOURCE_DIR}/data/tof
    ${CBMROOT_SOURCE_DIR}/data/trd

    ${CBMROOT_SOURCE_DIR}/ecal
    ${CBMROOT_SOURCE_DIR}/trd
    ${CBMROOT_SOURCE_DIR}/psd
    ${CBMROOT_SOURCE_DIR}/rich
    ${CBMROOT_SOURCE_DIR}/tof
    ${CBMROOT_SOURCE_DIR}/tof/TofMC
    ${CBMROOT_SOURCE_DIR}/sts
    ${CBMROOT_SOURCE_DIR}/sts/digitize
    ${CBMROOT_SOURCE_DIR}/sts/reco
)


Include_Directories(${INCLUDE_DIRECTORIES})

Set(SYSTEM_INCLUDE_DIRECTORIES
    ${BASE_INCLUDE_DIRECTORIES}
    ${ROOT_INCLUDE_DIR}
    ${ZMQ_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
)

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

configure_file( ${CBMROOT_SOURCE_DIR}/tutorials/cbmMQTest/options/CbmMQTestConfig.INI.in ${CMAKE_BINARY_DIR}/bin/config/CbmMQTestConfig.INI)
configure_file( ${CBMROOT_SOURCE_DIR}/tutorials/cbmMQTest/run/scripts/startMQChain.sh.in ${CMAKE_BINARY_DIR}/bin/startMQChain.sh)

file(MAKE_DIRECTORY ${CBMROOT_SOURCE_DIR}/tutorials/cbmMQTest/data_io)

set(LINK_DIRECTORIES
    ${ROOT_LIBRARY_DIR}
    ${Boost_LIBRARY_DIRS}
    ${FAIRROOT_LIBRARY_DIR}
    ${CBMLIBDIR}
) 



link_directories(${LINK_DIRECTORIES})




Set(Exe_Names
    runStsSampler
    runStsClusterFinder
)

set(Exe_Source
    run/runStsSampler.cxx
    run/runStsClusterFinder.cxx
)


############################################################
# Check if we run on Ubuntu version>=11 and if it is the case
# add --no-as-needed flag to the linker.
# it is needed on Ubuntu xx>=11 to load Ttree in executables
# see : https://root.cern.ch/phpBB3/viewtopic.php?f=3&t=14064
# Alternative is to instantiate a TApplication or load manually with gSystem->Load("libTree.so") in main file

if(NOT CMAKE_SYSTEM_NAME MATCHES Darwin)
    # use the LSB stuff if possible 
    EXECUTE_PROCESS(
       COMMAND cat /etc/lsb-release
       COMMAND grep DISTRIB_ID
       COMMAND awk -F= "{ print $2 }"
       COMMAND tr "\n" " "
       COMMAND sed "s/ //"
       OUTPUT_VARIABLE LSB_ID
       RESULT_VARIABLE LSB_ID_RESULT
    )
    EXECUTE_PROCESS(
       COMMAND cat /etc/lsb-release
       COMMAND grep DISTRIB_RELEASE
       COMMAND awk -F= "{ print $2 }"
       COMMAND tr "\n" " "
       COMMAND sed "s/ //"
       OUTPUT_VARIABLE LSB_VER
       RESULT_VARIABLE LSB_VER_RESULT
    )
    
    #message("LSB output: ${LSB_ID_RESULT}:${LSB_ID} ${LSB_VER_RESULT}:${LSB_VER}")
    if(NOT ${LSB_ID} STREQUAL "")
        # found some, use it :D
        set(INSTALLER_PLATFORM "${LSB_ID}-${LSB_VER}" CACHE PATH "Installer chosen platform")
        if(${LSB_ID} STREQUAL "Ubuntu" AND ${LSB_VER} VERSION_GREATER "10.0")
            #message(STATUS "Running on ${INSTALLER_PLATFORM} which is more recent than Ubuntu 11.0")
            #message(STATUS "--no-as-needed flag is added to the linker of GenericMQTuto executables")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl, --no-as-needed")
        endif()
    else(NOT ${LSB_ID} STREQUAL "")
       set(INSTALLER_PLATFORM "linux-generic" CACHE PATH "Installer chosen platform")
    endif(NOT ${LSB_ID} STREQUAL "")
endif(NOT CMAKE_SYSTEM_NAME MATCHES Darwin)
############################################################


List(LENGTH Exe_Names _length)
Math(EXPR _length ${_length}-1)

ForEach(_file RANGE 0 ${_length})
    List(GET Exe_Names ${_file} _name)
    List(GET Exe_Source ${_file} _src)
    Set(EXE_NAME ${_name})
    Set(SRCS ${_src})
    Set(DEPENDENCIES 
        Base 
        BaseMQ 
        Net
        Tree
        RIO 
        Cint 
        Core
        FairMQ
        ${FAIRMQ_DEPENDENCIES}
        ParBase 
        FairTools 
        CbmBase 
        CbmRun
        CbmData 
        boost_serialization
        CbmEcal
        CbmRich 
        CbmTrd 
        CbmTof 
        CbmPsd 
        #CbmGenerators 
        #Field 
        CbmSts
        )

    GENERATE_EXECUTABLE()
EndForEach(_file RANGE 0 ${_length})




Set(SRCS
    run/runFileSink.cxx
)

set(EXE_NAME
    runCbmMQFileSink 
)


Set(DEPENDENCIES 
    Base 
    BaseMQ 
    Net
    Tree
    RIO 
    Cint 
    Core
    FairMQ
    ${FAIRMQ_DEPENDENCIES}
    ParBase 
    FairTools 
    CbmData 
    boost_serialization
    )

    GENERATE_EXECUTABLE()


Set(Exe_Source
    run/runSplitter.cxx
    run/runMerger.cxx
)

set(Exe_Names
    runStsSplitter
    runStsMerger
)

List(LENGTH Exe_Names _length)
Math(EXPR _length ${_length}-1)

ForEach(_file RANGE 0 ${_length})
    List(GET Exe_Names ${_file} _name)
    List(GET Exe_Source ${_file} _src)
    Set(EXE_NAME ${_name})
    Set(SRCS ${_src})

Set(DEPENDENCIES 
    FairMQ
    ${FAIRMQ_DEPENDENCIES}
    )

    GENERATE_EXECUTABLE()
EndForEach(_file RANGE 0 ${_length})
