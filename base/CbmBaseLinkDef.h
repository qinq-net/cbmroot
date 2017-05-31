#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class CbmAddress;
#pragma link C++ class CbmModuleList;

//#pragma link C++ class CbmDigiManager+;
#pragma link C++ class CbmPropagator+;
#pragma link C++ class CbmTrdTrackFitter+;
#pragma link C++ class CbmTrdTrackFinder+;

#pragma link C++ class CbmTofTrackFitter+;
#pragma link C++ class CbmTofTrackFinder+;

#pragma link C++ class CbmStsTrackFitter+;
#pragma link C++ class CbmStsTrackFinder+;

#pragma link C++ class CbmMuchTrackFinder+;

#pragma link C++ class CbmRichTrackExtrapolation+;
#pragma link C++ class CbmRichRingFinder+;
#pragma link C++ class CbmRichRingFitter+;
#pragma link C++ class CbmRichRingTrackAssign+;
#pragma link C++ class CbmRichRingSelect+;

#pragma link C++ class CbmPrimaryVertexFinder+;
#pragma link C++ class CbmGlobalTrackFitter+;
#pragma link C++ class CbmRichMerger+;
#pragma link C++ class CbmTofMerger+;
#pragma link C++ class CbmTrackMerger+;

#pragma link C++ class CbmMCDataArray;
#pragma link C++ class CbmMCDataManager;
#pragma link C++ class CbmRadDamage;

#pragma link C++ class CbmModule+;

#pragma link C++ function SetDefaultDrawStyle();
#pragma link C++ function DrawH1(TH1*, HistScale, HistScale, const string&, Int_t, Int_t, Int_t, Int_t, Int_t);
#pragma link C++ function DrawH2(TH2*, HistScale, HistScale, HistScale, const string&);
#pragma link C++ function DrawH1(const vector<TH1*>&, const vector<string>&, HistScale, HistScale, Bool_t, Double_t, Double_t, Double_t, Double_t, const string&);
#pragma link C++ function DrawGraph(TGraph*, HistScale, HistScale, const string&, Int_t, Int_t, Int_t, Int_t, Int_t);
#pragma link C++ function DrawGraph(const vector<TGraph*>&, const vector<string>&, HistScale, HistScale, Bool_t, Double_t, Double_t, Double_t, Double_t);
#pragma link C++ function DrawGraph2D(TGraph2D*, HistScale, HistScale, HistScale, const string&);

#pragma link C++ class CbmHistManager;
#pragma link C++ class CbmReport+;
#pragma link C++ class CbmSimulationReport+;
#pragma link C++ class CbmStudyReport+;
#pragma link C++ class CbmReportElement+;
#pragma link C++ class CbmTextReportElement+;
#pragma link C++ class CbmLatexReportElement+;
#pragma link C++ class CbmHtmlReportElement+;

#pragma link C++ class CbmMatchRecoToMC+;

#pragma link C++ enum ECbmModuleId;
#pragma link C++ enum ECbmDataType;

#endif

