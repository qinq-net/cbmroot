# The name of our project is "CBMROOT".  CMakeLists files in this project can
# refer to the root source directory of the project as ${CBMROOT_SOURCE_DIR} or
# as ${CMAKE_SOURCE_DIR} and to the root binary directory of the project as 
# ${CBMROOT_BINARY_DIR} or ${CMAKE_BINARY_DIR}.
# This difference is important for the base classes which are in CBMROOT 
# and PANDAROOT.

# Check if cmake has the required version
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.3 FATAL_ERROR)

# Set name of our project to "CBMROOT". Has to be done
# after check of cmake version                        
project(CBMROOT)

FIND_PATH(FAIRBASE NAMES FairRun.h  PATHS
  ${CMAKE_SOURCE_DIR}/base/steer
  NO_DEFAULT_PATH
)

If (FAIRBASE)
  Message(STATUS "Found FAIRBASE")
  SET(FAIRBASE ${FAIRBASE})
Else (FAIRBASE)
  Message(STATUS "NOT Found FAIRBASE")
  IF(NOT DEFINED ENV{FAIRROOTPATH})
     MESSAGE(FATAL_ERROR "You did not define the environment variable FAIRROOTPATH which is needed to find FairRoot. Please set this variable and execute cmake again.")
  ENDIF(NOT DEFINED ENV{FAIRROOTPATH})
  SET(FAIRROOTPATH $ENV{FAIRROOTPATH})

EndIf (FAIRBASE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ 
# is checked
set(CMAKE_MODULE_PATH "${FAIRROOTPATH}/share/fairbase/cmake/modules_old" ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH "${FAIRROOTPATH}/share/fairbase/cmake/modules"  ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

# Check if the compiler support specific C++11 features
# Up to now this is only a check since the code does not use
# any of the features of the new standard

IF(FAIRROOTPATH)
  Set(CheckSrcDir "${FAIRROOTPATH}/share/fairbase/cmake/checks")
ELSE(FAIRROOTPATH)
  Set(CheckSrcDir "${CMAKE_SOURCE_DIR}/cmake/checks")
ENDIF(FAIRROOTPATH)

if(FAIRROOTPATH)
  find_package(FairRoot)
  SET(MY_ROOT_INCLUDE_PATH ${FAIRROOT_INCLUDE_DIR})
  If(CMAKE_INSTALL_PREFIX)
     Message("***** HERE *****")
     SET(MY_ROOT_INCLUDE_PATH ${MY_ROOT_INCLUDE_PATH} "${CMAKE_INSTALL_PREFIX}/include")
  EndIf(CMAKE_INSTALL_PREFIX)
endif(FAIRROOTPATH)

include(CheckCXX11Features)


# Load some basic macros which are needed later on
include(CbmMacros)
include(FairMacros)
include(WriteConfigFile)
include(CTest)
include(CheckCompiler)
Include(ExternalProject)

# CbmRoot uses from 22.07.15 C++11, so we need an compiler which supports C++11
# Check if the used compiler support C++11. If not stop with an error message
# Check also if FairSoft and FairRoot have been compiled with C++11 support 
If(NOT _HAS_CXX11_FLAG)
  Message(FATAL_ERROR "The used C++ compiler (${CMAKE_CXX_COMPILER}) does not support C++11. CbmRoot can only be compiled with compilers supporting C++11. Please install such an compiler.")
EndIf()

Execute_process(COMMAND $ENV{SIMPATH}/bin/fairsoft-config --cxxflags OUTPUT_VARIABLE _res_fairsoft_config OUTPUT_STRIP_TRAILING_WHITESPACE)
String(FIND ${_res_fairsoft_config} "-std=c++11" POS_C++11)
If(${POS_C++11} EQUAL -1) 
  Message(FATAL_ERROR "FairSoft wasn't compiled with c++11 support. Please recompile FairSoft with a compiler which supports c++11.")
EndIf()

Execute_process(COMMAND $ENV{SIMPATH}/bin/fairsoft-config --root-version OUTPUT_VARIABLE _res_root_version OUTPUT_STRIP_TRAILING_WHITESPACE)
String(FIND ${_res_root_version} "Usage" POS_USAGE)
If(${POS_USAGE} EQUAL 1) 
  Message(FATAL_ERROR "FairSoft version to old. At least nov15 compiled with ROOT6 is needed.")
EndIf()
If(NOT ${_res_root_version} EQUAL 6)
  Message(FATAL_ERROR "FairSoft was not compiled with ROOT6. At least FairSoft nov15 compiled with ROOT6 is needed.")
EndIf()

# Extract the FairRoot version from fairroot-config
# The version info is of the form Major.Minor.Patch e.g. 15.11.1 and
# is stored in the variable FairRoot_VERSION
FairRootVersion()

# Since 23.06.2016 CbmRoot need at least FairRoot v-15.11 f
# Check if the FairRoot version has at least this version 
If(FairRoot_VERSION VERSION_LESS 15.11.0)
  Message(FATAL_ERROR "\n CbmRoot needs at least FairRoot version v-15.11 (15.11.0).\n You use FairRoot ${FairRoot_VERSION}. Please upgrade your FairRoot version.") 
EndIf()

Execute_process(COMMAND $ENV{FAIRROOTPATH}/bin/fairroot-config --fairsoft_path OUTPUT_VARIABLE _simpath OUTPUT_STRIP_TRAILING_WHITESPACE)
Remove_Trailing_Slash($ENV{SIMPATH})
Set(_simpath ${_ret_val})
Remove_Trailing_Slash(${_simpath})
Set(_fairroot_config ${_ret_val})
String(COMPARE EQUAL ${_simpath} ${_fairroot_config}  _same_fairsoft)
If(NOT _same_fairsoft)
    Message(STATUS "FairSoft version used for FairRoot compilation: ${_fairroot_config}")
    Message(STATUS "FairSoft version now (defined by SIMPATH): ${_simpath}")
    Message(FATAL_ERROR "Both versions must be the same.")
EndIf()


#Check the compiler and set the compile and link flags
Check_Compiler()
#Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Weffc++ -Wno-unused-variable -Wno-unused-parameter -Wno-sign-compare -Wno-ignored-qualifiers -Wno-overloaded-virtual")
Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Weffc++ ")
#Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-padded -Wno-global-constructors")
#Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast")

Set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
Set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
Set(INCLUDE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/include")

Set(VMCWORKDIR ${CBMROOT_SOURCE_DIR})

Option(USE_PATH_INFO "Information from PATH and LD_LIBRARY_PATH are used." OFF)
If(USE_PATH_INFO)
  Set(PATH $ENV{PATH})
  If (APPLE)
    Set(LD_LIBRARY_PATH $ENV{DYLD_LIBRARY_PATH})
  Else (APPLE)
    Set(LD_LIBRARY_PATH $ENV{LD_LIBRARY_PATH})
  EndIf (APPLE)
Else(USE_PATH_INFO)
  STRING(REGEX MATCHALL "[^:]+" PATH $ENV{PATH})
EndIf(USE_PATH_INFO)

# Check if the user wants to build the project in the source
# directory
CHECK_OUT_OF_SOURCE_BUILD()

# Check if we are on an UNIX system. If not stop with an error
# message
IF(NOT UNIX)
  MESSAGE(FATAL_ERROR "You're not on an UNIX system. The project was up to now only tested on UNIX systems, so we break here. If you want to go on please edit the CMakeLists.txt in the source directory.")
ENDIF(NOT UNIX)  

#Check if necessary environment variables are set
#If not stop execution
if(NOT CBMROOT_MINIMAL)
  IF(NOT DEFINED ENV{SIMPATH})
    MESSAGE(FATAL_ERROR "You did not define the environment variable SIMPATH which is nedded to find the external packages. Please set this variable and execute cmake again.")
  ENDIF(NOT DEFINED ENV{SIMPATH})
  SET(SIMPATH $ENV{SIMPATH})
else(NOT CBMROOT_MINIMAL)
  if(NOT DEFINED ENV{ROOTSYS})
    MESSAGE(FATAL_ERROR "You did not define the environment variable ROOTSYS which is nedded for the installation of the minimal version of CbmRoot. Please set this variable and execute cmake again.")
  endif(NOT DEFINED ENV{ROOTSYS})
endif(NOT CBMROOT_MINIMAL)


# Check if the external packages are installed into a separate install
# directory
CHECK_EXTERNAL_PACKAGE_INSTALL_DIR()

# searches for needed packages
# REQUIRED means that cmake will stop if this packages are not found
# For example the framework can run without GEANT4, but ROOT is
# mandatory
if(CBMROOT_MINIMAL)
  MESSAGE("******* Install minimal CbmRoot Version ********")
  find_package(ROOT 5.32.00 REQUIRED)
else(CBMROOT_MINIMAL)
  find_package(ROOT 5.32.00 REQUIRED)
  find_package(PLUTO)
  find_package(GENERATORS REQUIRED)
  find_package(GEANT3 REQUIRED)
  find_package(GEANT4)
  find_package(GEANT4DATA)
  find_package(GEANT4VMC)
  find_package(CLHEP)
  find_package(TBB)
  find_package(SSE)
  find_package(IWYU)
  find_package(ZeroMQ)
# switch of the checking for the RuleChecker, since there is a problem
# with JAVA on some systems which resulting in endless hanging of CMake 
#  find_package(RuleChecker)


  Set(Boost_NO_SYSTEM_PATHS TRUE)
  Set(Boost_NO_BOOST_CMAKE TRUE)
#  set(Boost_DEBUG TRUE)
  If(${ROOT_LIBRARY_DIR} MATCHES /lib/root)
    set(BOOST_ROOT ${SIMPATH})
    set(GSL_DIR ${SIMPATH})
  Else(${ROOT_LIBRARY_DIR} MATCHES /lib/root)
    set(BOOST_ROOT ${SIMPATH}/basics/boost)
    set(GSL_DIR ${SIMPATH}/basics/gsl)
  EndIf(${ROOT_LIBRARY_DIR} MATCHES /lib/root)
  Message("-- Looking for Boost ...")
  # If an older version of boost is found both of the variables below are
  # cached and in a second cmake run, a good boost version is found even 
  # if the version is to old. 
  # To overcome this problem both variables are cleared before checking
  # for boost.
  Unset(Boost_INCLUDE_DIR CACHE)
  Unset(Boost_LIBRARY_DIRS CACHE)
  find_package(Boost 1.48 COMPONENTS program_options thread system timer serialization regex)
  If (Boost_FOUND)
    Set(Boost_Avail 1)
  Else (Boost_FOUND)
    Set(Boost_Avail 0)
  EndIf (Boost_FOUND)
  Message("-- Looking for GSL ...")
  find_package(GSL)

endif(CBMROOT_MINIMAL)

# set a variable which should be used in all CMakeLists.txt
# Defines all basic include directories from fairbase
SetBasicVariables()

Set(BASE_INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES} 
${SYSTEM_INCLUDE_DIRECTORIES} 
)

# Set the library version in the main CMakeLists.txt
SET(CBMROOT_MAJOR_VERSION 0)
SET(CBMROOT_MINOR_VERSION 0)
SET(CBMROOT_PATCH_VERSION 0)
SET(CBMROOT_VERSION "${CBMROOT_MAJOR_VERSION}.${CBMROOT_MINOR_VERSION}.${CBMROOT_PATCH_VERSION}")
If(NOT ROOT_FOUND_VERSION OR ROOT_FOUND_VERSION LESS 59999)
  SET(FAIRROOT_LIBRARY_PROPERTIES ${FAIRROOT_LIBRARY_PROPERTIES}
      VERSION "${CBMROOT_VERSION}"
      SOVERSION "${CBMROOT_MAJOR_VERSION}"
      SUFFIX ".so"
  )
Else()
  SET(FAIRROOT_LIBRARY_PROPERTIES ${FAIRROOT_LIBRARY_PROPERTIES}
      VERSION "${CBMROOT_VERSION}"
      SOVERSION "${CBMROOT_MAJOR_VERSION}"
  )
EndIf()

SET(CBMLIBDIR ${CMAKE_BINARY_DIR}/lib)  
SET(LD_LIBRARY_PATH  ${CBMLIBDIR} ${Boost_LIBRARY_DIRS} ${LD_LIBRARY_PATH})


Generate_Version_Info()

# Recurse into the given subdirectories.  This does not actually
# cause another cmake executable to run.  The same process will walk through
# the project's entire directory structure.

If(CMAKE_SYSTEM_NAME MATCHES Darwin)
  Include(CheckIncludeFile)
  Check_Include_File("_types/_uint8_t.h" _UINT8_T_EXIST)
EndIf()

### Subdirectories to be compiled #####

### Extrenals included as svn externals
add_subdirectory (external)

### Base directories
add_subdirectory (base)
add_subdirectory (data)
add_subdirectory (field)  
add_subdirectory (generators)
add_subdirectory (run)

### Module directories
add_subdirectory (passive)
add_subdirectory (mvd) 
add_subdirectory (sts) 
add_subdirectory (rich) 
add_subdirectory (much)
add_subdirectory (trd) 
add_subdirectory (tof) 
add_subdirectory (ecal) 
add_subdirectory (psd)

### Reco directories
add_subdirectory (global) 
add_subdirectory (L1)
add_subdirectory (KF)
add_subdirectory (littrack) 
add_subdirectory (tracking)
add_subdirectory (eventbuilder)

### Analysis directories
add_subdirectory (analysis)
add_subdirectory (dilept)
add_subdirectory (hadron)

add_subdirectory (macro)
#####add_subdirectory (trdcatrack)
#add_subdirectory (tutorials/Tutorial1/src)
#add_subdirectory (tutorials/Tutorial2/src)
#add_subdirectory (vis)


# Check if the compilation flag -std=c++11 is set
# In this case the code is compiled with the new standard and it is possible
# to compile the fles_ipc library
If(NOT CMAKE_CXX_FLAGS)
  Message(STATUS "No C++11 support found. FLES_IPC library will not be build.")
Else()
  String(FIND ${CMAKE_CXX_FLAGS} "-std=c++11" POS_C++11)
  If(${POS_C++11} EQUAL -1)
    Message(STATUS "No C++11 support found. FLES_IPC library will not be build.")
  Else()
    Message(STATUS "C++11 support found. Build FLES_IPC library.")
    add_subdirectory(fles)
    Find_Library(BaseMQ_FOUND NAME BaseMQ PATHS ${FAIRROOTPATH}/lib)
    If(BaseMQ_FOUND)
      Message(STATUS "BaseMQ library found. Build MQTest tutorials.")
#      add_subdirectory(tutorials/MQTest)
    EndIf()
  EndIf()
EndIf()

add_subdirectory(tutorials)

Option(BUILD_UNITTESTS "Build all unit tests and add them as new tests" ON)
If(BUILD_UNITTESTS)

  Message("Build the unit tests.")

  Set(GTEST_ROOT ${SIMPATH})
  Find_Package(GTest)

  If(GTEST_FOUND)
    Message("Add tests library and tests.")
    Add_Subdirectory(test)
  EndIf(GTEST_FOUND)
EndIf(BUILD_UNITTESTS)

Execute_Process(COMMAND ${ROOT_CONFIG_EXECUTABLE} --has-opengl
                OUTPUT_VARIABLE EveIsBuild
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )

If(${EveIsBuild} STREQUAL yes)
  Message(STATUS "Build the eventdisplay.")

  add_subdirectory (eventdisplay)

Else(${EveIsBuild} STREQUAL yes)
  Message(STATUS "The eventdisplay is not build, there is no OpenGL support.")
EndIf(${EveIsBuild} STREQUAL yes)

##set(BUILD_BEAMTIME FALSE CACHE BOOL "Build everything needed (DABC, RocLib ...) to read testbeam data.")
#If(BUILD_BEAMTIME)
##  add_subdirectory (roclight)
#  add_subdirectory (roc)
#  add_subdirectory (beamtime)
#else(BUILD_BEAMTIME)
# tof beamtime software is needed for the new tof tracking and fles
# It requires EITHER roc OR roclight to be compiled
  add_subdirectory (roclight)
  add_subdirectory (beamtime/tof)
  add_subdirectory (beamtime)
#EndIf(BUILD_BEAMTIME)
  

Option(BUILD_DOXYGEN "Build Doxygen" OFF)
if(BUILD_DOXYGEN)
  MESSAGE(STATUS "*** Building the Doxygen documentaion ***")
  ADD_SUBDIRECTORY(doxygen)
endif(BUILD_DOXYGEN)

if(RULE_CHECKER_FOUND)

  ADD_CUSTOM_TARGET(RULES 
     COMMAND ${RULE_CHECKER_SCRIPT1} ${CMAKE_BINARY_DIR} viol > violations.html
     DEPENDS $ENV{ALL_RULES})

endif(RULE_CHECKER_FOUND)

if(IWYU_FOUND)

  ADD_CUSTOM_TARGET(checkHEADERS
     DEPENDS $ENV{ALL_HEADER_RULES}
  )

endif(IWYU_FOUND)

if(NOT CBMROOT_MINIMAL)
  WRITE_CONFIG_FILE(config.sh)
  WRITE_CONFIG_FILE(config.csh)

  SET(ROOT_INCLUDE_PATH ${ROOT_INCLUDE_PATH} "${CMAKE_INSTALL_PREFIX}/include")
  SET(VMCWORKDIR ${CMAKE_INSTALL_PREFIX}/share/cbmroot)
  WRITE_CONFIG_FILE(config.sh_install)
  WRITE_CONFIG_FILE(config.csh_install)

else(NOT CBMROOT_MINIMAL)
  SET(FULL_CONFIG_FILE "false")
  WRITE_CONFIG_FILE(config.sh)
endif(NOT CBMROOT_MINIMAL)


Install(FILES ${CMAKE_BINARY_DIR}/config.sh_install
        DESTINATION bin
        RENAME CbmRootConfig.sh
       )

Install(FILES ${CMAKE_BINARY_DIR}/check_system.sh
        DESTINATION bin
       )

Install(FILES ${CMAKE_BINARY_DIR}/config.csh_install
        DESTINATION bin
        RENAME CbmRootConfig.csh
       )


Install(DIRECTORY gconfig geometry input parameters
        DESTINATION share/cbmroot
        PATTERN ".svn" EXCLUDE)
