/**
nh, adapt from 
 * \file CbmTrdTrackFinderIdeal.h
**/

#ifndef CBMTOFTRACKFINDERNN_H
#define CBMTOFTRACKFINDERNN_H

#include "CbmTofTrackFinder.h"
#include "CbmTofTrackFitter.h"
#include "CbmTofHit.h"
#include "CbmTofTracklet.h"
#include "LKFMinuit.h"

#include <vector>
#include <map>

class TClonesArray;

class CbmTofTrackFinderNN : public CbmTofTrackFinder 
{
public:
   /**
    * \brief Constructor.
    */
   CbmTofTrackFinderNN();

   /**
    * \brief Destructor
    */
   virtual ~CbmTofTrackFinderNN();

   /**
    * \brief Inherited from CbmTofTrackFinder.
    */
   void Init();

   Int_t DoFind(	TClonesArray* fTofHits,
			TClonesArray* fTofTracks);

   void TrklSeed(       Int_t         iHit	 );
   Int_t HitUsed(       Int_t         iHit	 ); 
 
/*			  
   void RemoveMultipleAssignedHits(        
			 TClonesArray* fTofHits,
			 Int_t         iDet
			 );
*/

   void UpdateTrackList( Int_t         iTrk );
   void UpdateTrackList( CbmTofTracklet*  pTrk );

   inline void SetFitter    (CbmTofTrackFitter* Fitter )   { fFitter     = Fitter;}
   inline void SetMaxTofTimeDifference ( Double_t val ){ fMaxTofTimeDifference = val;}
   inline void SetTxLIM  ( Double_t val ) { fTxLIM = val; }
   inline void SetTyLIM  ( Double_t val ) { fTyLIM = val; }
   inline void SetTyMean ( Double_t val ) { fTyMean = val; }
   inline void SetSIGLIM ( Double_t val ) { fSIGLIM = val; }
   inline void SetChiMaxAccept ( Double_t val ) { fChiMaxAccept = val; }

   inline Double_t GetSIGLIM () { return fSIGLIM; }
   inline Double_t GetChiMaxAccept () { return fChiMaxAccept; }

   void Line3Dfit(CbmTofTracklet*  pTrk);
   Bool_t  Active(CbmTofTracklet*  pTrk);

   void PrintStatus(char* cComm);

   //Copy constructor
   CbmTofTrackFinderNN(const CbmTofTrackFinderNN &finder);
   //assignment operator
   CbmTofTrackFinderNN& operator=(const CbmTofTrackFinderNN &fSource);

 private:

   TClonesArray* fHits;
   TClonesArray* fOutTracks;
   Int_t fiNtrks; // Number of Tracks
   CbmTofTrackFitter* fFitter;      // Pointer to TrackFitter concrete class
   CbmTofFindTracks*  fFindTracks;  // Pointer to Task 
   CbmTofDigiPar         * fDigiPar;
   Double_t fMaxTofTimeDifference;
   Double_t fTxLIM;
   Double_t fTyLIM;
   Double_t fTyMean;
   Double_t fSIGLIM;
   Double_t fChiMaxAccept;
   Double_t fPosYMaxScal;
   LKFMinuit fMinuit;

   //intermediate storage variables
   std::vector<CbmTofTracklet *>  fTracks;        // Tracklets to which hit is assigned
   //std::vector<std::map <CbmTofTracklet *, Int_t> > fvTrkMap;  // Tracklets to which hit is assigned
   std::vector< std::vector<CbmTofTracklet *> > fvTrkVec;        // Tracklets to which hit is assigned

   ClassDef(CbmTofTrackFinderNN,1);

};
 
#endif
