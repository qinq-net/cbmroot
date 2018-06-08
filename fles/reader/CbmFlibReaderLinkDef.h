#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class CbmFlibFileSource;
//#pragma link C++ class CbmNxFlibFileSource;
#pragma link C++ class CbmFlibFileSourceNew;

//#pragma link C++ class CbmTSUnpack;
#pragma link C++ class CbmTSUnpackSpadic;
#pragma link C++ class CbmTSUnpackSpadic20;
#pragma link C++ class CbmTSUnpackSpadic11OnlineMonitor;
#pragma link C++ class CbmTSUnpackSpadic20OnlineMonitor;
#pragma link C++ class CbmTSUnpackSpadic20DesyOnlineMonitor;
#pragma link C++ class CbmTSUnpackSpadicLegacy;
#pragma link C++ class CbmTSUnpackNxyter;
#pragma link C++ class CbmTSUnpackStsxyter;
#pragma link C++ class CbmTSUnpackFiberHodo;
#pragma link C++ class CbmTSUnpackDummy;
#pragma link C++ class CbmTSUnpackTrb;
#pragma link C++ class get4v1x::Message;
#pragma link C++ class get4v1x::FullMessage;
#pragma link C++ class CbmTSUnpackGet4v1x;
#pragma link C++ class CbmTSUnpackSpadicOnlineFex;

#pragma link C++ class CbmGet4FastMonitor;
#pragma link C++ class CbmGet4EpochBuffer;
#pragma link C++ class CbmGet4EventBuilder;
#pragma link C++ class CbmGet4EventBuffer;

#ifdef BUILD_TOF_BEAMTIME
 #pragma link C++ class CbmGet4EventDumper;
 // #pragma link C++ class CbmGet4EventMonitor;
#endif // NOT BUILD_BEAMTIME_LIB

#pragma link C++ class CbmFiberHodoMapping;
#pragma link C++ class CbmSpadicTriggerComp;
#pragma link C++ class CbmTrdTestBeamTools;
#pragma link C++ class CbmTrdTestBeamTools2017DESY;
#pragma link C++ class CbmTrdTestBeamTools2017GIF;
#pragma link C++ class CbmTrdTestBeamToolsBuch2016;
#pragma link C++ class CbmTrdLabTools;
#pragma link C++ class CbmTrdQABase;
#pragma link C++ class CbmTrdQACosmics;
#pragma link C++ class CbmTrdQAHit;
#pragma link C++ class CbmTrdQAEfficiency;
//#pragma link C++ class CbmTrdQABaseline;
#pragma link C++ class CbmTrdSimpleDigitizer;
#pragma link C++ class CbmTrdAdvDigitizer;
#pragma link C++ class CbmTrdDigiAnalysis;
#pragma link C++ class CbmTrdSimpleClusterizer;
#pragma link C++ class CbmTrdSimpleClusterAnalysis;
#pragma link C++ class CbmTrdDESYAlignment;
#pragma link C++ class CbmTrdRawBeamProfile;
#pragma link C++ class CbmTrdRawBuchData;
#pragma link C++ class CbmTrdRawPulseMonitor;
#pragma link C++ class CbmTrdOnlineDisplay;
#pragma link C++ class CbmTrdEveOnlineDisplay;
#pragma link C++ class CbmTrdClusterAnalysis;
//#pragma link C++ class CbmTrdDaqBuffer;
#pragma link C++ class CbmTrdTestBeamAnalysis;
#pragma link C++ class CbmTrdTimeCorrel;
#pragma link C++ class CbmTrdTestBeamAnalysis2015SPS;

#endif
