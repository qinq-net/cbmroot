/** @file CbmTofHitFinderQa.h
 ** @author Pierre-Alain Loizeau <loizeau@physi.uni-heidelberg.de>
 ** @date 27/08/2015
 **/

/** @class CbmTofHitFinderQa
 ** @brief QA class for the TOF event based Hit finder tasks (clusterizers, ...)
 ** @author Pierre-Alain Loizeau <loizeau@physi.uni-heidelberg.de>
 ** @version 1.0
 **/
#ifndef _CBMTOFHITFINDERQA_H_
#define _CBMTOFHITFINDERQA_H_

#include "FairTask.h"
#include "CbmMCEventHeader.h"

class CbmTofGeoHandler;
class CbmTofCell;
// Geometry
class CbmTofGeoHandler;
class CbmTofDetectorId;
class CbmTofCell;

class CbmTofDigiPar;
class CbmTofDigiBdfPar;

class TClonesArray;
class TH1;
class TH2;
class TString;

class CbmTofHitFinderQa : public FairTask {

   public:
       CbmTofHitFinderQa();
       CbmTofHitFinderQa(const char* name, Int_t verbose = 1);
       virtual ~CbmTofHitFinderQa();

       virtual InitStatus Init();
       virtual void Exec(Option_t* option);
       virtual void Finish();

       /**
        ** @brief Inherited from FairTask.
        **/
       virtual void SetParContainers();

       Bool_t   RegisterInputs();
       
       Bool_t   SetHistoFileNameCartCoordNorm( TString sFilenameIn );
       Bool_t   SetHistoFileNameAngCoordNorm( TString sFilenameIn );
       Bool_t   SetHistoFileNameSphCoordNorm( TString sFilenameIn );
       Bool_t   SetHistoFileName( TString sFilenameIn );
       
       void   SetNormHistGenerationMode( Bool_t bModeIn = kTRUE){ fbNormHistGenMode = bModeIn;}
       
       Bool_t   SetWallPosZ( Double_t dWallPosCm = 1000);

   private:
      Bool_t   CreateHistos();
      Bool_t   FillHistos();
      Bool_t   NormalizeMapHistos();
      Bool_t   NormalizeNormHistos();
      Bool_t   WriteHistos();
      Bool_t   DeleteHistos();

      /**
      ** @brief Load the geometry: for now just resizing the Digis temporary vectors
      **/
      Bool_t   LoadGeometry();

      CbmTofHitFinderQa(const CbmTofHitFinderQa&);
      CbmTofHitFinderQa operator=(const CbmTofHitFinderQa&);

      Int_t         fEvents;            // Number of processed events

      // Geometry infos
      CbmMCEventHeader *fMCEventHeader;
      CbmTofGeoHandler * fGeoHandler;
      CbmTofDetectorId * fTofId;
      CbmTofCell       * fChannelInfo;
      Int_t iNbSmTot;
      std::vector< Int_t >                               fvTypeSmOffs; // Offset in SM index for first SM of each SM type
      Int_t iNbRpcTot;
      std::vector< std::vector< Int_t > >                fvSmRpcOffs;  // Offset in RPC index for first RPC of each SM
      Int_t fiNbChTot;
      std::vector< std::vector< std::vector< Int_t > > > fvRpcChOffs;  // Offset in channel index for first channel of each RPC
    
      // Parameters
      CbmTofDigiPar    * fDigiPar;
      CbmTofDigiBdfPar * fDigiBdfPar;

      TClonesArray          * fTofPointsColl; // TOF MC points
      TClonesArray          * fMcTracksColl;  // MC tracks
      TClonesArray          * fTofDigisColl;  // TOF Digis
      TClonesArray          * fTofDigiMatchPointsColl; // Indices of MC original points for each digi (CbmMatch)
      TClonesArray          * fTofHitsColl; // TOF hits
      TClonesArray          * fTofDigiMatchColl;  // Indices of TOF Digis for each hit (CbmMatch)
      TClonesArray          * fTofHitMatchColl; // // Indices of MC original points for each hit with TOT weight (CbmMatch)
      Bool_t                  fbHitProducerSource;

      // Histograms
         // Flag for Normalization histograms generation
      Bool_t  fbNormHistGenMode;
         // Input file names and path for Mapping Normalization histos
      TString fsHistoInNormCartFilename;   
      TString fsHistoInNormAngFilename;    
      TString fsHistoInNormSphFilename;  
         // Output file name and path
      TString fsHistoOutFilename;   
         // Position of the TOF wall on Z axis for centering histos with Z
      Double_t fdWallPosZ;
         // Geometric Mapping
      TH2 * fhTrackMapXY;  // Only when creating normalization histos
      TH2 * fhTrackMapXZ;  // Only when creating normalization histos
      TH2 * fhTrackMapYZ;  // Only when creating normalization histos
      TH2 * fhTrackMapAng; // Only when creating normalization histos
      TH2 * fhTrackMapSph; // Only when creating normalization histos
      TH2 * fhPointMapXY;
      TH2 * fhPointMapXZ;
      TH2 * fhPointMapYZ;
      TH2 * fhPointMapAng;
      TH2 * fhPointMapSph;
      TH2 * fhDigiMapXY;
      TH2 * fhDigiMapXZ;
      TH2 * fhDigiMapYZ;
      TH2 * fhDigiMapAng;
      TH2 * fhDigiMapSph;
      TH2 * fhHitMapXY;
      TH2 * fhHitMapXZ;
      TH2 * fhHitMapYZ;
      TH2 * fhHitMapAng;
      TH2 * fhHitMapSph;
         // L/R digis missmatch
      TH2 * fhLeftRightDigiMatch;
         // Nb different MC Points and Tracks in Hit
      TH1 * fhNbPointsInHit;
      TH1 * fhNbTracksInHit;
         // Mapping of position for hits coming from a single MC Point
      TH2 * fhHitMapSingPntXY;
      TH2 * fhHitMapSingPntXZ;
      TH2 * fhHitMapSingPntYZ;
      TH2 * fhHitMapSingPntAng;
      TH2 * fhHitMapSingPntSph;
         // Mapping of position for hits coming from multiple MC Points
      TH2 * fhHitMapMultPntXY;
      TH2 * fhHitMapMultPntXZ;
      TH2 * fhHitMapMultPntYZ;
      TH2 * fhHitMapMultPntAng;
      TH2 * fhHitMapMultPntSph;
         // Mapping of position for hits coming from a single MC Track
      TH2 * fhHitMapSingTrkXY;
      TH2 * fhHitMapSingTrkXZ;
      TH2 * fhHitMapSingTrkYZ;
      TH2 * fhHitMapSingTrkAng;
      TH2 * fhHitMapSingTrkSph;
         // Mapping of position for hits coming from multiple MC Tracks
      TH2 * fhHitMapMultTrkXY;
      TH2 * fhHitMapMultTrkXZ;
      TH2 * fhHitMapMultTrkYZ;
      TH2 * fhHitMapMultTrkAng;
      TH2 * fhHitMapMultTrkSph;
         // Hit Quality for Hits coming from a single MC Point
      TH1 * fhSinglePointHitDeltaX;
      TH1 * fhSinglePointHitDeltaY;
      TH1 * fhSinglePointHitDeltaZ;
      TH1 * fhSinglePointHitDeltaR;
      TH1 * fhSinglePointHitDeltaT;
      TH1 * fhSinglePointHitPullX;
      TH1 * fhSinglePointHitPullY;
      TH1 * fhSinglePointHitPullZ;
      TH1 * fhSinglePointHitPullR;
         // Hit Quality for Hits coming from multiple MC Points
            // To Point closest to Hit
      TH2 * fhMultiPntHitClosestDeltaX;
      TH2 * fhMultiPntHitClosestDeltaY;
      TH2 * fhMultiPntHitClosestDeltaZ;
      TH2 * fhMultiPntHitClosestDeltaR;
      TH2 * fhMultiPntHitClosestDeltaT;
      TH2 * fhMultiPntHitClosestPullX;
      TH2 * fhMultiPntHitClosestPullY;
      TH2 * fhMultiPntHitClosestPullZ;
      TH2 * fhMultiPntHitClosestPullR;
            // To Point furthest from Hit
      TH2 * fhMultiPntHitFurthestDeltaX;
      TH2 * fhMultiPntHitFurthestDeltaY;
      TH2 * fhMultiPntHitFurthestDeltaZ;
      TH2 * fhMultiPntHitFurthestDeltaR;
      TH2 * fhMultiPntHitFurthestDeltaT;
      TH2 * fhMultiPntHitFurthestPullX;
      TH2 * fhMultiPntHitFurthestPullY;
      TH2 * fhMultiPntHitFurthestPullZ;
      TH2 * fhMultiPntHitFurthestPullR;
            // To mean Point position
      TH2 * fhMultiPntHitMeanDeltaX;
      TH2 * fhMultiPntHitMeanDeltaY;
      TH2 * fhMultiPntHitMeanDeltaZ;
      TH2 * fhMultiPntHitMeanDeltaR;
      TH2 * fhMultiPntHitMeanDeltaT;
      TH2 * fhMultiPntHitMeanPullX;
      TH2 * fhMultiPntHitMeanPullY;
      TH2 * fhMultiPntHitMeanPullZ;
      TH2 * fhMultiPntHitMeanPullR;
            // To best Point position
      TH2 * fhMultiPntHitBestDeltaX;
      TH2 * fhMultiPntHitBestDeltaY;
      TH2 * fhMultiPntHitBestDeltaZ;
      TH2 * fhMultiPntHitBestDeltaR;
      TH2 * fhMultiPntHitBestDeltaT;
      TH2 * fhMultiPntHitBestPullX;
      TH2 * fhMultiPntHitBestPullY;
      TH2 * fhMultiPntHitBestPullZ;
      TH2 * fhMultiPntHitBestPullR;
         // Hit Quality for Hits coming from a single MC Track
      TH1 * fhSingleTrackHitDeltaX;
      TH1 * fhSingleTrackHitDeltaY;
      TH1 * fhSingleTrackHitDeltaZ;
      TH1 * fhSingleTrackHitDeltaR;
      TH1 * fhSingleTrackHitDeltaT;
      TH1 * fhSingleTrackHitPullX;
      TH1 * fhSingleTrackHitPullY;
      TH1 * fhSingleTrackHitPullZ;
      TH1 * fhSingleTrackHitPullR;
         // Hit Quality for Hits coming from a single MC Track but multi Pnt
      TH1 * fhSingTrkMultiPntHitDeltaX;
      TH1 * fhSingTrkMultiPntHitDeltaY;
      TH1 * fhSingTrkMultiPntHitDeltaZ;
      TH1 * fhSingTrkMultiPntHitDeltaR;
      TH1 * fhSingTrkMultiPntHitDeltaT;
      TH1 * fhSingTrkMultiPntHitPullX;
      TH1 * fhSingTrkMultiPntHitPullY;
      TH1 * fhSingTrkMultiPntHitPullZ;
      TH1 * fhSingTrkMultiPntHitPullR;
         // Hit Quality for Hits coming from multiple MC Tracks
            // To Track closest to Hit
      TH2 * fhMultiTrkHitClosestDeltaX;
      TH2 * fhMultiTrkHitClosestDeltaY;
      TH2 * fhMultiTrkHitClosestDeltaZ;
      TH2 * fhMultiTrkHitClosestDeltaR;
      TH2 * fhMultiTrkHitClosestDeltaT;
      TH2 * fhMultiTrkHitClosestPullX;
      TH2 * fhMultiTrkHitClosestPullY;
      TH2 * fhMultiTrkHitClosestPullZ;
      TH2 * fhMultiTrkHitClosestPullR;
            // To Track furthest from Hit
      TH2 * fhMultiTrkHitFurthestDeltaX;
      TH2 * fhMultiTrkHitFurthestDeltaY;
      TH2 * fhMultiTrkHitFurthestDeltaZ;
      TH2 * fhMultiTrkHitFurthestDeltaR;
      TH2 * fhMultiTrkHitFurthestDeltaT;
      TH2 * fhMultiTrkHitFurthestPullX;
      TH2 * fhMultiTrkHitFurthestPullY;
      TH2 * fhMultiTrkHitFurthestPullZ;
      TH2 * fhMultiTrkHitFurthestPullR;
            // To mean Track position
      TH2 * fhMultiTrkHitMeanDeltaX;
      TH2 * fhMultiTrkHitMeanDeltaY;
      TH2 * fhMultiTrkHitMeanDeltaZ;
      TH2 * fhMultiTrkHitMeanDeltaR;
      TH2 * fhMultiTrkHitMeanDeltaT;
      TH2 * fhMultiTrkHitMeanPullX;
      TH2 * fhMultiTrkHitMeanPullY;
      TH2 * fhMultiTrkHitMeanPullZ;
      TH2 * fhMultiTrkHitMeanPullR;
            // To best Track position
      TH2 * fhMultiTrkHitBestDeltaX;
      TH2 * fhMultiTrkHitBestDeltaY;
      TH2 * fhMultiTrkHitBestDeltaZ;
      TH2 * fhMultiTrkHitBestDeltaR;
      TH2 * fhMultiTrkHitBestDeltaT;
      TH2 * fhMultiTrkHitBestPullX;
      TH2 * fhMultiTrkHitBestPullY;
      TH2 * fhMultiTrkHitBestPullZ;
      TH2 * fhMultiTrkHitBestPullR;
      
         // Physics coord mapping, 1 per particle type
            // Phase space
               // Primary tracks
      std::vector<TH2 *> fvhPtmRapGenTrk;
      std::vector<TH2 *> fvhPtmRapStsPnt;
      std::vector<TH2 *> fvhPtmRapTofPnt;
      std::vector<TH2 *> fvhPtmRapTofHit;
      std::vector<TH2 *> fvhPtmRapTofHitSinglePnt;
      std::vector<TH2 *> fvhPtmRapTofHitSingleTrk;
               // Secondary tracks
      std::vector<TH2 *> fvhPtmRapSecGenTrk;
      std::vector<TH2 *> fvhPtmRapSecStsPnt;
      std::vector<TH2 *> fvhPtmRapSecTofPnt;
      std::vector<TH2 *> fvhPtmRapSecTofHit;
      std::vector<TH2 *> fvhPtmRapSecTofHitSinglePnt;
      std::vector<TH2 *> fvhPtmRapSecTofHitSingleTrk;
      
            // PLab
               // Primary tracks
      std::vector<TH1 *> fvhPlabGenTrk;
      std::vector<TH1 *> fvhPlabStsPnt;
      std::vector<TH1 *> fvhPlabTofPnt;
      std::vector<TH1 *> fvhPlabTofHit;
      std::vector<TH1 *> fvhPlabTofHitSinglePnt;
      std::vector<TH1 *> fvhPlabTofHitSingleTrk;
               // Secondary tracks
      std::vector<TH1 *> fvhPlabSecGenTrk;
      std::vector<TH1 *> fvhPlabSecStsPnt;
      std::vector<TH1 *> fvhPlabSecTofPnt;
      std::vector<TH1 *> fvhPlabSecTofHit;
      std::vector<TH1 *> fvhPlabSecTofHitSinglePnt;
      std::vector<TH1 *> fvhPlabSecTofHitSingleTrk;
      
         // MC Tracks losses
            // Primary tracks
      std::vector<TH2 *> fvhPtmRapGenTrkTofPnt;
      std::vector<TH2 *> fvhPtmRapGenTrkTofHit;
      std::vector<TH1 *> fvhPlabGenTrkTofPnt;
      std::vector<TH1 *> fvhPlabGenTrkTofhit;
            // Secondary tracks
      std::vector<TH2 *> fvhPtmRapSecGenTrkTofPnt;
      std::vector<TH2 *> fvhPtmRapSecGenTrkTofHit;
      std::vector<TH1 *> fvhPlabSecGenTrkTofPnt;
      std::vector<TH1 *> fvhPlabSecGenTrkTofhit;

         // Integrated TofHit Efficiency
      std::vector<ULong64_t> fvulIdxTracksWithPnt;
      std::vector<ULong64_t> fvulIdxTracksWithHit;
      TH1* fhIntegratedHitPntEff;
      std::vector<ULong64_t> fvulIdxPrimTracksWithPnt;
      std::vector<ULong64_t> fvulIdxPrimTracksWithHit;
      TH1* fhIntegratedHitPntEffPrim;
      std::vector<ULong64_t> fvulIdxSecTracksWithPnt;
      std::vector<ULong64_t> fvulIdxSecTracksWithHit;
      TH1* fhIntegratedHitPntEffSec;
      
      ClassDef(CbmTofHitFinderQa, 1);
};

#endif

