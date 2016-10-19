#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;


// Setup
//#pragma link C++ class CbmStsAddress;
#pragma link C++ class CbmStsElement;
#pragma link C++ class CbmStsModule;
#pragma link C++ class CbmStsSensor;
#pragma link C++ class CbmStsSensorConditions;
#pragma link C++ class CbmStsSensorPoint;
#pragma link C++ class CbmStsSensorType;
#pragma link C++ class CbmStsSetup;
#pragma link C++ class CbmStsStation;

// Monte-Carlo
#pragma link C++ class CbmStsAcceptance;
#pragma link C++ class CbmStsMC;
#pragma link C++ class CbmStsSensorFactory;
#pragma link C++ class CbmStsTrackStatus;

// Digitization
#pragma link C++ class CbmStsDigitize;
#pragma link C++ class CbmStsPhysics;
#pragma link C++ class CbmStsSensorTypeDssd;
#pragma link C++ class CbmStsSensorTypeDssdOrtho;
#pragma link C++ class CbmStsSensorTypeDssdIdeal;
#pragma link C++ class CbmStsSensorTypeDssdReal;
#pragma link C++ class CbmStsSignal;

// Reconstruction
#pragma link C++ class CbmStsClusterAnalysis;
#pragma link C++ class CbmStsClusterFinder;
#pragma link C++ class CbmStsClusterFinderGap;
#pragma link C++ class CbmStsClusterFinderIdeal;
#pragma link C++ class CbmStsClusterFinderReal;
#pragma link C++ class CbmStsClusterFinderSimple;
#pragma link C++ class CbmStsFindClusters;
#pragma link C++ class CbmStsFindClustersEvents;
#pragma link C++ class CbmStsFindHits;

// Enumerators
#pragma link C++ enum EStsElementLevel;

// Others
#pragma link C++ class CbmStsContFact;
#pragma link C++ class CbmStsFindTracks+;
#pragma link C++ class CbmStsFindTracksQa+;
#pragma link C++ class CbmStsFitTracks+;
#pragma link C++ class CbmStsHitProducerIdeal+;
//#pragma link C++ class CbmStsIdealMatchHits+;
//#pragma link C++ class CbmStsMatchHits+;
#pragma link C++ class CbmStsMatchTracks+;
#pragma link C++ class CbmStsRadTool;
#pragma link C++ class CbmStsTrackFinderIdeal+;
#pragma link C++ class CbmStsTrackFitterIdeal+;
#pragma link C++ class CbmStsSimulationQa+;
//#pragma link C++ class CbmStsReconstructionQa+;
#pragma link C++ class CbmStsDigitizeQa+;
#pragma link C++ class CbmStsDigitizeQaReport+;
#pragma link C++ class CbmStsTimeBasedQa;
#pragma link C++ class CbmStsTimeBasedQaReport;



#endif /* __CINT__ */

