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
      Int_t iNbChTot;
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
         // Mapping
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
         // Hit Quality for Hits coming from a single MC Point
      TH1 * fhSinglePointHitDeltaX;
      TH1 * fhSinglePointHitDeltaY;
      TH1 * fhSinglePointHitDeltaZ;
      TH1 * fhSinglePointHitDeltaR;
      TH1 * fhSinglePointHitDeltaT;
         // Hit Quality for Hits coming from a multiple MC Points
            // To Point closest to Hit
      TH1 * fhMultiPntHitClosestDeltaX;
      TH1 * fhMultiPntHitClosestDeltaY;
      TH1 * fhMultiPntHitClosestDeltaZ;
      TH1 * fhMultiPntHitClosestDeltaR;
      TH1 * fhMultiPntHitClosestDeltaT;
            // To Point furthest from Hit
      TH1 * fhMultiPntHitFurthestDeltaX;
      TH1 * fhMultiPntHitFurthestDeltaY;
      TH1 * fhMultiPntHitFurthestDeltaZ;
      TH1 * fhMultiPntHitFurthestDeltaR;
      TH1 * fhMultiPntHitFurthestDeltaT;
            // To mean Point position
      TH1 * fhMultiPntHitMeanDeltaX;
      TH1 * fhMultiPntHitMeanDeltaY;
      TH1 * fhMultiPntHitMeanDeltaZ;
      TH1 * fhMultiPntHitMeanDeltaR;
      TH1 * fhMultiPntHitMeanDeltaT;
         // Hit Quality for Hits coming from a single MC Track
      TH1 * fhSingleTrackHitDeltaX;
      TH1 * fhSingleTrackHitDeltaY;
      TH1 * fhSingleTrackHitDeltaZ;
      TH1 * fhSingleTrackHitDeltaR;
      TH1 * fhSingleTrackHitDeltaT;
         // Hit Quality for Hits coming from a multiple MC Points
            // To Track closest to Hit
      TH1 * fhMultiTrkHitClosestDeltaX;
      TH1 * fhMultiTrkHitClosestDeltaY;
      TH1 * fhMultiTrkHitClosestDeltaZ;
      TH1 * fhMultiTrkHitClosestDeltaR;
      TH1 * fhMultiTrkHitClosestDeltaT;
            // To Track furthest from Hit
      TH1 * fhMultiTrkHitFurthestDeltaX;
      TH1 * fhMultiTrkHitFurthestDeltaY;
      TH1 * fhMultiTrkHitFurthestDeltaZ;
      TH1 * fhMultiTrkHitFurthestDeltaR;
      TH1 * fhMultiTrkHitFurthestDeltaT;
            // To mean Track position
      TH1 * fhMultiTrkHitMeanDeltaX;
      TH1 * fhMultiTrkHitMeanDeltaY;
      TH1 * fhMultiTrkHitMeanDeltaZ;
      TH1 * fhMultiTrkHitMeanDeltaR;
      TH1 * fhMultiTrkHitMeanDeltaT;

      ClassDef(CbmTofHitFinderQa, 1);
};

#endif

