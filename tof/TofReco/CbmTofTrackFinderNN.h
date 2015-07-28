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

   void TrklSeed(       Int_t         iHit
			 );
   Int_t HitUsed(       Int_t         iHit
			 );  
/*			  
   void RemoveMultipleAssignedHits(        
			 TClonesArray* fTofHits,
			 Int_t         iDet
			 );
*/
   void UpdateTrackList( Int_t         iTrk );

   inline void SetFitter    (CbmTofTrackFitter* Fitter )   { fFitter     = Fitter;}
   inline void SetMaxTofTimeDifference ( Double_t val ){ fMaxTofTimeDifference = val;}
   inline void SetTxLIM  ( Double_t val ) { fTxLIM = val; }
   inline void SetTyLIM  ( Double_t val ) { fTyLIM = val; }
   inline void SetTyMean ( Double_t val ) { fTyMean = val; }
   inline void SetSIGLIM ( Double_t val ) { fSIGLIM = val; }
   inline void SetSIGT   ( Double_t val ) { fSIGT = val; }
   inline void SetSIGX   ( Double_t val ) { fSIGX = val; }
   inline void SetSIGY   ( Double_t val ) { fSIGY = val; }

   inline Double_t GetSIGLIM () { return fSIGLIM; }
   inline Double_t GetSIGT () { return fSIGT; }
   inline Double_t GetSIGX () { return fSIGX; }
   inline Double_t GetSIGY () { return fSIGY; }

   void Line3Dfit(CbmTofTracklet*  pTrk);

   //Copy constructor
   CbmTofTrackFinderNN(const CbmTofTrackFinderNN &finder);
   //assignment operator
   CbmTofTrackFinderNN& operator=(const CbmTofTrackFinderNN &fSource);

 private:

   TClonesArray* fHits;
   TClonesArray* fTracks;
   Int_t fiNtrks; // Number of Tracks
   CbmTofTrackFitter* fFitter;      // Pointer to TrackFitter concrete class
   CbmTofFindTracks*  fFindTracks;  // Pointer to Task 
   CbmTofDigiPar         * fDigiPar;
   Double_t fMaxTofTimeDifference;
   Double_t fTxLIM;
   Double_t fTyLIM;
   Double_t fTyMean;
   Double_t fSIGLIM;
   Double_t fSIGT;
   Double_t fSIGX;
   Double_t fSIGY;
   Double_t fPosYMaxScal;
   LKFMinuit fMinuit;

   std::vector<std::map <CbmTofTracklet *, Int_t> > fvTrkMap;  // Tracklets to which hit is assigned

   ClassDef(CbmTofTrackFinderNN,1);

};
 
#endif
