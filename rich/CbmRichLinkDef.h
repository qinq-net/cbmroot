// $Id: RichLinkDef.h,v 1.16 2006/09/13 14:56:13 hoehne Exp $

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class CbmRich+;
#pragma link C++ class CbmGeoRich+;
#pragma link C++ class CbmRichContFact;
#pragma link C++ class CbmGeoRichPar;
#pragma link C++ class CbmRichTrainAnnSelect;
#pragma link C++ class CbmRichTrainAnnElectrons;
#pragma link C++ class CbmRichEventDisplay+;

//detector
#pragma link C++ class CbmRichHitProducer+;
#pragma link C++ class CbmRichDigitizer+;
#pragma link C++ class CbmRichPmt+;

//reconstruction
#pragma link C++ class CbmRichReconstruction+;

//qa
#pragma link C++ class CbmRichTestSim+;
#pragma link C++ class CbmRichTestHits+;
#pragma link C++ class CbmRichGeoTest+;
#pragma link C++ class CbmRichUrqmdTest+;
#pragma link C++ class CbmRichGeoOpt+;
#pragma link C++ class CbmRichRingFitterQa+;
#pragma link C++ class CbmRichRecoQa+;
#pragma link C++ class CbmRichRecoTbQa+;

//prototype
#pragma link C++ class CbmRichProt+;
#pragma link C++ class CbmRichProtHitProducer+;
#pragma link C++ class CbmRichProtPrepareExtrapolation+;
#pragma link C++ class CbmRichProtProjectionProducer+;
#pragma link C++ class CbmRichPrototypeQa+;
#pragma link C++ class CbmRichSmallPrototypeQa+;
#pragma link C++ class CbmRichSmallPrototypeStudyReport+;

//mcbm
#pragma link C++ class CbmRichMCbmQa+;

//prototype analysis (beamtime and lab)
#pragma link C++ class CbmRichTrbUnpack+;
#pragma link C++ class CbmRichTrbUnpack2+;
#pragma link C++ class CbmTrbEdgeMatcher+;
#pragma link C++ class CbmTrbCalibrator+;
#pragma link C++ class CbmRichTrbEventBuilder+;
#pragma link C++ class CbmRichProtoAnalysis+;
#pragma link C++ class CbmRichEventDebug+;
#pragma link C++ class CbmRichRingHitsAnalyser+;

#pragma link C++ class CbmRichTrbRecoQa+;
#pragma link C++ class CbmRichTrbPulserQa+;
#pragma link C++ class CbmRichHitInfo+;
#pragma link C++ class CbmRichTrbRecoQaStudyReport+;

//mirrors
//#pragma link C++ class CbmRichMirror+;
//#pragma link C++ class CbmRichMirrorSortingAlignment+;
//#pragma link C++ class CbmRichMirrorSortingCorrection+;
//#pragma link C++ class CbmRichMirrorMisalignmentCorrectionUtils+;
//#pragma link C++ class CbmRichRonchiAna+;


#endif

