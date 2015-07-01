// -------------------------------------------------------------------------
// -----              CbmMvdClusterAna  header file                    -----
// -----              Created 27/04/15  by S. Amar-Youcef              -----
// -------------------------------------------------------------------------


/**  CbmMvdClusterAna.h
 *@author S.Amar-Youcef <s.amar@gsi.de>
 *
 *  Cluster Analysis
 *
 **/

#ifndef CBMMVDCLUSTERANA_H
#define CBMMVDCLUSTERANA_H

#include "FairTask.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"
#include "TNtuple.h"
#include <iostream>
#include "TVector3.h"
#include "TClonesArray.h"
// #include "base/CbmLitPtrTypes.h"
#include "CbmTrackMatchNew.h"
// #include "CbmStsTrack.h"
#include "TGeoManager.h"
#include "TROOT.h"
#include "tools/CbmMvdGeoHandler.h"


class TClonesArray;
class CbmStsTrack;
class CbmVertex;
class FairTrackParam;
class CbmLitTrackParam;
class CbmStsTrack;


class CbmMvdClusterAna : public FairTask
{
public:
	CbmMvdClusterAna();
	CbmMvdClusterAna(const char* name, Int_t iMode = 0, Int_t iVerbose = 0);
	
	~CbmMvdClusterAna();
	
	InitStatus	Init();
	void			Exec(Option_t* opt); 
	void 			Finish();
	
private:
	TClonesArray* fMcPoints;
	TClonesArray* fMvdDigis;
	TClonesArray* fMvdClusters;
	TClonesArray* fMvdHits;
	
	TClonesArray* fMvdDigisMatch;	
	TClonesArray* fMvdClustersMatch;
	TClonesArray* fMvdHitsMatch;
	
	TClonesArray* fListMCTracks;
	TClonesArray* fStsTrackArray;
	TClonesArray* fStsTrackMatches;
	
	TH1F *fMvdHisto1[50];
	TH2F *fMvdHisto2[50];
	TProfile *fProf[50]; 
// 	
	TGeoManager* gGeoManager;
	int fNrMcPointsAll;
	int fNrHitsAll;
	
	Int_t	fMcperDigi[6];
	Int_t	fMcperHit[11];
	
	Double_t fPixelpitch[2];
  
 ClassDef(CbmMvdClusterAna,1);
};


#endif