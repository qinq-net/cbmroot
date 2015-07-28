/** @file CbmTofTestBeamClusterizer.h
 ** @author nh adopted from
 ** @author Pierre-Alain Loizeau <loizeau@physi.uni-heidelberg.de>
 ** @date 23.08.2013
 **/
/** @class CbmTofSimpClusterizer
 ** @brief Simple Cluster building and hit producing for CBM ToF using Digis as input
 ** @author Pierre-Alain Loizeau <loizeau@physi.uni-heidelberg.de>
 ** @version 1.0
 **/
#ifndef CBMTOFTESTBEAMCLUSTERIZER_H
#define CBMTOFTESTBEAMCLUSTERIZER_H  1

// TOF Classes and includes
// Input/Output
//class CbmTofPoint;
class CbmTofDigi;
class CbmTofDigiExp;
class CbmMatch;
   // Geometry
class CbmTofGeoHandler;
class CbmTofDetectorId;
class CbmTofDigiPar;
class CbmTofDigiBdfPar;
class CbmTofCell;
class CbmTofFindTracks;

class TMbsMappingTofPar;
class TTofCalibData;

// FAIR classes and includes
#include "FairTask.h"

// ROOT Classes and includes
class TClonesArray;
class TH1;
class TH2;
class TString;
#include "TTimeStamp.h"

// C++ Classes and includes
#include <vector>

class CbmTofTestBeamClusterizer : public FairTask
{
   public:

      /**
       ** @brief Constructor.
       **/
      CbmTofTestBeamClusterizer();

      /**
       ** @brief Constructor.
       **/
      CbmTofTestBeamClusterizer(const char *name, Int_t verbose = 1, Bool_t writeDataInOut = kTRUE);
      /**
       ** @brief Destructor.
       **/
      virtual ~CbmTofTestBeamClusterizer();

      /**
       ** @brief Inherited from FairTask.
       **/
      virtual InitStatus Init();

      /**
       ** @brief Inherited from FairTask.
       **/
      virtual void SetParContainers();

      /**
       ** @brief Inherited from FairTask.
       **/
      virtual void Exec(Option_t * option);

      /**
       ** @brief Inherited from FairTask.
       **/
      virtual void Finish();
      virtual void Finish(Double_t calMode);

      inline void SetCalMode    (Int_t iMode)           { fCalMode     = iMode;}
      inline void SetCalSel     (Int_t iSel)            { fCalSel      = iSel;}
      inline void SetCalSmType  (Int_t iCalSmType)      { fCalSmType   = iCalSmType;}
      inline void SetCaldXdYMax (Double_t dCaldXdYMax)  { fdCaldXdYMax = dCaldXdYMax;}
      inline void SetCalCluMulMax (Int_t ival)          { fiCluMulMax  = ival;}
      inline void SetTRefId     (Int_t Id)              { fTRefMode    = Id;}
      inline void SetDutId      (Int_t Id)              { fDutId       = Id;}
      inline void SetSelId      (Int_t Id)              { fSelId       = Id;}
      inline void SetBeamRefType   (Int_t Id)           { fiBeamRefType      = Id;}
      inline void SetBeamRefSm     (Int_t Id)           { fiBeamRefSm        = Id;}
      inline void SetBeamRefDet    (Int_t Id)           { fiBeamRefDet       = Id;}
      inline void SetBeamAddRefMul (Int_t ival)         { fiBeamAddRefMul  = ival;}
      inline void SetTRefDifMax (Double_t val)          { fTRefDifMax  = val;}
      inline void SetdTRefMax   (Double_t val)          { fdTRefMax    = val;}
      inline void PosYMaxScal   (Double_t val)          { fPosYMaxScal = val;}
      inline void SetTotMax     (Double_t val)          { fTotMax      = val;}
      inline void SetTotMin     (Double_t val)          { fTotMin      = val;}
      inline void SetTotMean    (Double_t val)          { fTotMean     = val;}
      inline void SetTotPreRange(Double_t val)          { fTotPreRange = val;}
      inline void SetMaxTimeDist(Double_t val)          { fMaxTimeDist = val;}
      inline void SetSel2Id     (Int_t ival)            { fSel2Id = ival;}

      inline void SetOutHstFileName(TString OutHstFileName) { fOutHstFileName = OutHstFileName; }
      inline void SetCalParFileName(TString CalParFileName) { fCalParFileName = CalParFileName; }

   protected:

   private:
      /**
       ** @brief Copy constructor.
       **/
      CbmTofTestBeamClusterizer(const CbmTofTestBeamClusterizer&);
      /**
       ** @brief Copy operator.
       **/
      CbmTofTestBeamClusterizer& operator=(const CbmTofTestBeamClusterizer&);

      // Functions common for all clusters approximations
      /**
       ** @brief Recover pointer on input TClonesArray: TofPoints, TofDigis...
       **/
      Bool_t   RegisterInputs();
      /**
       ** @brief Create and register output TClonesArray of Tof Hits.
       **/
      Bool_t   RegisterOutputs();
      /**
       ** @brief Initialize other parameters not included in parameter classes.
       **/
      Bool_t   InitParameters();
      /**
       ** @brief Initialize other parameters not included in parameter classes.
       **/
      Bool_t   InitCalibParameter();
      /**
       ** @brief Load the geometry: for now just resizing the Digis temporary vectors
       **/
      Bool_t   LoadGeometry();
      /**
       ** @brief Delete the geometry related arrays: for now just clearing the Digis temporary vectors
       **/
      Bool_t   DeleteGeometry();

      // Histogramming functions
      Bool_t   CreateHistos();
      Bool_t   FillHistos();
      Bool_t   WriteHistos();
      Bool_t   DeleteHistos();

      /**
       ** @brief Build clusters out of ToF Digis and store the resulting info in a TofHit.
       **/
      Bool_t   BuildClusters();
      Bool_t   MergeClusters();

      // ToF geometry variables
      CbmTofGeoHandler      * fGeoHandler;
      CbmTofDetectorId      * fTofId;
      CbmTofDigiPar         * fDigiPar;
      CbmTofCell            * fChannelInfo;
      CbmTofDigiBdfPar      * fDigiBdfPar;

      TMbsMappingTofPar     *fMbsMappingPar;

      // Input variables 
      TClonesArray          * fTofPointsColl; // TOF MC points
      TClonesArray          * fMcTracksColl;  // MC tracks
      TClonesArray          * fTofDigisColl;  // TOF Digis

      // Output variables
      Bool_t                  fbWriteHitsInOut;
      TClonesArray          * fTofHitsColl;       // TOF hits
      TClonesArray          * fTofDigiMatchColl;  // TOF Digis
      Int_t  fiNbHits;                            // Index of the CbmTofHit TClonesArray

      // Generic
      Int_t fVerbose;

      // Intermediate storage variables
      std::vector< std::vector< std::vector< std::vector< CbmTofDigi* > > > >
               fStorDigi; //[nbType][nbSm*nbRpc][nbCh][nDigis]
      std::vector< std::vector< std::vector< std::vector< CbmTofDigiExp* > > > >
               fStorDigiExp; //[nbType][nbSm*nbRpc][nbCh][nDigis]
      std::vector< std::vector< std::vector< std::vector< Int_t > > > >
               fStorDigiInd; //[nbType][nbSm*nbRpc][nbCh][nDigis]
      /*
      std::vector< std::vector< std::vector< std::vector< std::vector< CbmTofDigi* > > > > >
               fStorDigi; //[nbType][nbSm][nbRpc][nbCh][nDigis]
      std::vector< std::vector< std::vector< std::vector< std::vector< CbmTofDigiExp* > > > > >
               fStorDigiExp; //[nbType][nbSm][nbRpc][nbCh][nDigis]
      */
      std::vector< std::vector< std::vector< Int_t > > > fviClusterMul; //[nbType][nbSm][nbRpc]
      std::vector< std::vector< std::vector< Int_t > > > fviClusterSize; //[nbType][nbRpc][nClusters]
      std::vector< std::vector< std::vector< Int_t > > > fviTrkMul; //[nbType][nbRpc][nClusters]
      std::vector< std::vector< std::vector< Double_t > > > fvdX; //[nbType][nbRpc][nClusters]
      std::vector< std::vector< std::vector< Double_t > > > fvdY; //[nbType][nbRpc][nClusters]
      std::vector< std::vector< std::vector< Double_t > > > fvdDifX; //[nbType][nbRpc][nClusters]
      std::vector< std::vector< std::vector< Double_t > > > fvdDifY; //[nbType][nbRpc][nClusters]
      std::vector< std::vector< std::vector< Double_t > > > fvdDifCh; //[nbType][nbRpc][nClusters]

      // Histograms
      TH1* fhClustBuildTime;
      TH1* fhHitsPerTracks;
      TH1* fhPtsPerHit;
      TH1* fhTimeResSingHits;
      TH2* fhTimeResSingHitsB;
      TH2* fhTimePtVsHits;
      TH1* fhClusterSize;
      TH2* fhClusterSizeType;
      TH1* fhTrackMul;
      TH2* fhClusterSizeMulti;
      TH2* fhTrk1MulPos;
      TH2* fhHiTrkMulPos;
      TH2* fhAllTrkMulPos;
      TH2* fhMultiTrkProbPos;
      TH1* fhDigSpacDifClust;
      TH1* fhDigTimeDifClust;
      TH2* fhDigDistClust;
      TH2* fhClustSizeDifX;
      TH2* fhClustSizeDifY;
      TH2* fhChDifDifX;
      TH2* fhChDifDifY;

      std::vector< TH2* > fhRpcDigiCor;     //[nbDet]
      std::vector< TH1* > fhRpcCluMul;      //[nbDet]
      std::vector< TH2* > fhRpcCluPosition; //[nbDet]
      std::vector< TH2* > fhRpcCluDelPos;   //[nbDet]
      std::vector< TH2* > fhRpcCluDelMatPos;   //[nbDet]
      std::vector< TH2* > fhRpcCluTOff;        //[nbDet] 
      std::vector< TH2* > fhRpcCluDelTOff;     //[nbDet] 
      std::vector< TH2* > fhRpcCluDelMatTOff;  //[nbDet] 
      std::vector< TH2* > fhRpcCluTrms;     //[nbDet] 
      std::vector< TH2* > fhRpcCluTot;      // [nbDet]
      std::vector< TH2* > fhRpcCluSize;     // [nbDet]
      std::vector< TH2* > fhRpcCluAvWalk;   // [nbDet]
      std::vector< TH2* > fhRpcCluAvLnWalk; // [nbDet]
      std::vector< std::vector< std::vector<TH2 *> > >fhRpcCluWalk; // [nbDet][nbCh][nSide]

      std::vector< TH2* > fhSmCluPosition; //[nbSmTypes]
      std::vector< TH2* > fhSmCluTOff; 

      std::vector< std::vector< TH1* > > fhTRpcCluMul;      //[nbDet][nbSel]
      std::vector< std::vector< TH2* > > fhTRpcCluPosition; //[nbDet][nbSel]
      std::vector< std::vector< TH2* > > fhTRpcCluTOff;     //[nbDet] [nbSel]
      std::vector< std::vector< TH2* > > fhTRpcCluTot;      // [nbDet][nbSel]
      std::vector< std::vector< TH2* > > fhTRpcCluSize;     // [nbDet][nbSel]
      std::vector< std::vector< TH2* > > fhTRpcCluAvWalk;   // [nbDet][nbSel]
      std::vector< std::vector< TH2* > > fhTRpcCluDelTof;   // [nbDet][nbSel]
      std::vector< std::vector< TH2* > > fhTRpcCludXdY;     // [nbDet][nbSel]
      std::vector< std::vector< std::vector< std::vector<TH2 *> > > >fhTRpcCluWalk; // [nbDet][nbSel][nbCh][nSide]

      std::vector< std::vector< TH2* > > fhTSmCluPosition; //[nbSmTypes][nbSel]
      std::vector< std::vector< TH2* > > fhTSmCluTOff;     //[nbSmTypes][nbSel]
      std::vector< std::vector< TH2* > > fhTSmCluTRun;     //[nbSmTypes][nbSel]

      std::vector< TH1* > fhSeldT;  //[nbSel] 

      std::vector< std::vector< std::vector< std::vector< Double_t > > > > fvCPDelTof;   //[nSMT][nRpc][nbClDelTofBinX][nbSel]
      std::vector< std::vector< std::vector< std::vector< Double_t > > > > fvCPTOff;     //[nSMT][nRpc][nCh][nbSide]
      std::vector< std::vector< std::vector< std::vector< Double_t > > > > fvCPTotGain;  //[nSMT][nRpc][nCh][nbSide]
      std::vector< std::vector< std::vector< std::vector< Double_t > > > > fvCPTotOff;  //[nSMT][nRpc][nCh][nbSide]
      std::vector< std::vector< std::vector< std::vector< std::vector< Double_t > > > > > fvCPWalk; //[nSMT][nRpc][nCh][nbSide][nbWalkBins]

      // Digis quality
      Int_t fiNbSameSide;
      TH1* fhNbSameSide;
      TH1* fhNbDigiPerChan;

      // Control
      TTimeStamp fStart;
      TTimeStamp fStop;

      // Calib
      Double_t dTRef;
      Double_t fdTRefMax;
      Int_t    fCalMode;
      Int_t    fCalSel;
      Int_t    fCalSmType;
      Double_t fdCaldXdYMax;
      Int_t    fiCluMulMax;
      Int_t    fTRefMode;
      Int_t    fTRefHits;
      Int_t    fDutId;
      Int_t    fSelId;
      Int_t    fiBeamRefType;
      Int_t    fiBeamRefSm;
      Int_t    fiBeamRefDet;
      Int_t    fiBeamAddRefMul;
      Int_t    fSel2Id;

      Double_t fPosYMaxScal;
      Double_t fTRefDifMax;
      Double_t fTotMax;
      Double_t fTotMin;
      Double_t fTotOff;
      Double_t fTotMean;
      Double_t fTotPreRange;
      Double_t fMaxTimeDist;

      TString       fCalParFileName;      // name of the file name with Calibration Parameters
      TString       fOutHstFileName;      // name of the histogram output file name with Calibration Parameters
      TFile*        fCalParFile;          // pointer to Calibration Parameter file 

      // Constants or setting parameters
      Int_t    fiNevtBuild;
      Int_t    fiMsgCnt;

      Double_t fdTOTMax;
      Double_t fdTOTMin;
      Double_t fdTTotMean;

      Double_t fdMaxTimeDist; // Isn't this just a local variable? Why make it global and preset?!?
      Double_t fdMaxSpaceDist; // Isn't this just a local variable? Why make it global and preset?!?

      Double_t fdEvent;

   ClassDef(CbmTofTestBeamClusterizer, 1);
};

#endif // CBMTOFTESTBEAMCLUSTERIZER_H
