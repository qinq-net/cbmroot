// ------------------------------------------------------------------
// -----                     CbmTofTests                        -----
// -----              Created 29/08/2013 by P.-A. Loizeau       -----
// ------------------------------------------------------------------
#ifndef _CBMTOFANATESTBEAM_H_
#define _CBMTOFANATESTBEAM_H_ 1

class CbmTofGeoHandler;
class CbmTofCell;
// Geometry
class CbmTofGeoHandler;
class CbmTofDetectorId;
class CbmTofCell;

class CbmTofDigiPar;
class CbmTofDigiBdfPar;
class TMbsMappingTofPar;

#include "FairTask.h"

class TClonesArray;
class TH1;
class TH2;
class TString;
#include "TTimeStamp.h"

class CbmTofAnaTestbeam : public FairTask {
   friend class CbmTofFindTracks;
   public:
       CbmTofAnaTestbeam();
       CbmTofAnaTestbeam(const char* name, Int_t verbose = 1);
       virtual ~CbmTofAnaTestbeam();

       virtual InitStatus Init();
       virtual void Exec(Option_t* option);
       virtual void Finish();

       /**
        ** @brief Inherited from FairTask.
        **/
       virtual void SetParContainers();

       Bool_t   RegisterInputs();

      inline Double_t GetDXMean    () const           { return fdDXMean;}
      inline Double_t GetDYMean    () const           { return fdDYMean;}
      inline Double_t GetDTMean    () const           { return fdDTMean;}
      inline Double_t GetDXWidth   () const           { return fdDXWidth;}
      inline Double_t GetDYWidth   () const           { return fdDYWidth;}
      inline Double_t GetDTWidth   () const           { return fdDTWidth;}
      inline Double_t GetMul0Max   () const           { return fdMul0Max;}
      inline Double_t GetMul4Max   () const           { return fdMul4Max;}
      inline Double_t GetCh4Sel    () const           { return fdCh4Sel;}
      inline Double_t GetDCh4Sel   () const           { return fdDCh4Sel;}
      inline Double_t GetPosY4Sel  () const           { return fdPosY4Sel;}
      inline Double_t GetPosY4SelOff  () const        { return fdPosY4SelOff;}
      inline Double_t GetChS2Sel    () const          { return fdChS2Sel;}
      inline Double_t GetDChS2Sel   () const          { return fdDChS2Sel;}
      inline Double_t GetPosYS2Sel  () const          { return fdPosYS2Sel;}
      inline Double_t GetPosYS2SelOff () const        { return fdPosYS2SelOff;}
      inline Double_t GetSel2TOff  () const           { return fdSel2TOff;}
      inline Double_t GetMulDMax   () const           { return fdMulDMax;}
      inline Double_t GetDTDia     () const           { return fdDTDia;}
      inline Double_t GetDTD4MAX   () const           { return fdDTD4MAX;}
      inline Double_t GetHitDistMin() const           { return fdHitDistMin;}
      inline Double_t GetTOffD4    () const           { return fdTOffD4;}
      inline Double_t GetTShift    () const           { return fdTShift;}

      inline Int_t GetCorMode   () const              { return fiCorMode;}
      inline Int_t GetDut       () const              { return fiDut;}
      inline Int_t GetMrpcRef   () const              { return fiMrpcRef;}
      inline Int_t GetMrpcSel2  () const              { return fiMrpcSel2;}
      inline Int_t GetMrpcSel3  () const              { return fiMrpcSel3;}
      inline Int_t GetPlaSelect () const              { return fiPlaSelect;}
      inline Int_t GetBeamRefSmType () const          { return fiBeamRefSmType;}
      inline Int_t GetBeamRefSmId    () const         { return fiBeamRefSmId;}

      inline void SetDXMean    (Double_t val)           { fdDXMean = val;}
      inline void SetDYMean    (Double_t val)           { fdDYMean = val;}
      inline void SetDTMean    (Double_t val)           { fdDTMean = val;}
      inline void SetDXWidth   (Double_t val)           { fdDXWidth = val;}
      inline void SetDYWidth   (Double_t val)           { fdDYWidth = val;}
      inline void SetDTWidth   (Double_t val)           { fdDTWidth = val;}
      inline void SetMul0Max   (Double_t val)           { fdMul0Max = val;}
      inline void SetMul4Max   (Double_t val)           { fdMul4Max = val;}
      inline void SetCh4Sel    (Double_t val)           { fdCh4Sel = val;}
      inline void SetDCh4Sel   (Double_t val)           { fdDCh4Sel = val;}
      inline void SetPosY4Sel  (Double_t val)           { fdPosY4Sel = val;}
      inline void SetPosY4SelOff (Double_t val)         { fdPosY4SelOff = val;}
      inline void SetChS2Sel    (Double_t val)          { fdChS2Sel = val;}
      inline void SetDChS2Sel   (Double_t val)          { fdDChS2Sel = val;}
      inline void SetPosYS2Sel  (Double_t val)          { fdPosYS2Sel = val;}
      inline void SetPosYS2SelOff (Double_t val)        { fdPosYS2SelOff = val;}
      inline void SetSel2TOff  (Double_t val)           { fdSel2TOff = val;}
      inline void SetMulDMax   (Double_t val)           { fdMulDMax = val;}
      inline void SetDTDia     (Double_t val)           { fdDTDia = val;}
      inline void SetDTD4MAX   (Double_t val)           { fdDTD4MAX = val;}
      inline void SetHitDistMin(Double_t val)           { fdHitDistMin = val;}
      inline void SetTOffD4    (Double_t val)           { fdTOffD4 = val;}
      inline void SetTShift    (Double_t val)           { fdTShift = val;}
      inline void SetChi2Lim   (Double_t val)           { fdChi2Lim = val;}

      inline void SetCorMode   (Int_t ival)             { fiCorMode = ival;}
      inline void SetDut       (Int_t ival)             { fiDut = ival;}
      inline void SetMrpcRef   (Int_t ival)             { fiMrpcRef  = ival;}
      inline void SetMrpcSel2  (Int_t ival)             { fiMrpcSel2 = ival;}
      inline void SetMrpcSel3  (Int_t ival)             { fiMrpcSel3 = ival;}
      inline void SetPlaSelect (Int_t ival)             { fiPlaSelect = ival;}
      inline void SetBeamRefSmType (Int_t ival)         { fiBeamRefSmType = ival;}
      inline void SetBeamRefSmId    (Int_t ival)        { fiBeamRefSmId   = ival;}

      inline void SetCalParFileName(TString CalParFileName) { fCalParFileName = CalParFileName; }

   private:
      Bool_t   LoadGeometry();
      Bool_t   CreateHistos();
      Bool_t   FillHistos();
      Bool_t   WriteHistos();
      Bool_t   DeleteHistos();

      /**
       ** @brief Initialize other parameters not included in parameter classes.
       **/
      Bool_t   InitParameters();
      /**
      ** @brief Load the geometry: for now just resizing the Digis temporary vectors
      **/
      Bool_t   LoadCalParameter();

      CbmTofAnaTestbeam(const CbmTofAnaTestbeam&);
      CbmTofAnaTestbeam operator=(const CbmTofAnaTestbeam&);

      Int_t         fEvents;            // Number of processed events

      // Geometry infos
      CbmTofGeoHandler * fGeoHandler;
      CbmTofDetectorId * fTofId;
      CbmTofCell       * fChannelInfo;
      CbmTofCell       * fChannelInfoRef;
      CbmTofCell       * fChannelInfoDut;
      CbmTofCell       * fChannelInfoSel2;

      TMbsMappingTofPar     *fMbsMappingPar;

      Int_t iNbSmTot;
      std::vector< Int_t >                               fvTypeSmOffs; // Offset in SM index for first SM of each SM type
      Int_t iNbRpcTot;
      std::vector< std::vector< Int_t > >                fvSmRpcOffs;  // Offset in RPC index for first RPC of each SM
      Int_t iNbChTot;
      std::vector< std::vector< std::vector< Int_t > > > fvRpcChOffs;  // Offset in channel index for first channel of each RPC
    
      // Parameters
      CbmTofDigiPar    * fDigiPar;
      CbmTofDigiBdfPar * fDigiBdfPar;

      TClonesArray          * fTofDigisColl;      // TOF Digis
      TClonesArray          * fTofHitsColl;       // TOF hits
      TClonesArray          * fTofDigiMatchColl;  // TOF DigiMatches
      TClonesArray          * fTofTrackColl;      // TOF Tracks

      Double_t fdDXMean;
      Double_t fdDYMean;
      Double_t fdDTMean;
      Double_t fdDXWidth;
      Double_t fdDYWidth;
      Double_t fdDTWidth;

      // Histograms

      TH1 *fhDT2;
      TH2 *fhXX2;
      TH2 *fhYY2;
      TH1 *fhNMatch04;
      TH2 *fhXX04;
      TH2 *fhYY04;
      TH2 *fhTT04;

      TH1 *fhChi04;
      TH1 *fhChiSel24;
      TH1 *fhDXSel24;
      TH1 *fhDYSel24;
      TH1 *fhDTSel24;
      TH2 *fhDXDY04;
      TH2 *fhDXDT04;
      TH2 *fhDYDT04;
      TH1 *fhTofSel24;

      TH1 *fhNMatch04sel;
      TH1 *fhChi04best;
      TH1 *fhDigiMul0best;
      TH1 *fhDigiMul4best;
      TH2 *fhDXDY04best;
      TH2 *fhDXDT04best;
      TH2 *fhDYDT04best;
      TH2 *fhChiDT04best;
      TH2 *fhDT24DT04best;
      TH2 *fhDTD4DT04best;
      TH2 *fhX0DT04best;
      TH2 *fhY0DT04best;

      TH1 *fhNMatchD4sel;
      TH1 *fhChi04D4best;
      TH1 *fhTofD4best;
      TH1 *fhVelD4best;
      TH1 *fhDigiMul0D4best;
      TH1 *fhDigiMul4D4best;
      TH2 *fhCluMul04D4best;
      TH2 *fhDXDY04D4best;
      TH2 *fhDXDT04D4best;
      TH2 *fhDYDT04D4best;
      TH2 *fhDistDT04D4best;
      TH2 *fhTexpDT04D4best;
      TH2 *fhCluSize0DT04D4best;
      TH2 *fhCluSize4DT04D4best;
      TH2 *fhTot0DT04D4best;
      TH2 *fhTot4DT04D4best;
      TH2 *fhChiDT04D4best;
      TH2 *fhDT24DT04D4best;
      TH2 *fhDTD4DT04D4best;
      TH2 *fhX0DT04D4best;
      TH2 *fhY0DT04D4best;
      TH2 *fhDTMul4D4best;
      TH2 *fhDTX4D4best;
      TH2 *fhDTY4D4best;
      TH2 *fhDXX4D4best;
      TH2 *fhDXY4D4best;
      TH2 *fhDYX4D4best;
      TH2 *fhDYY4D4best;
      TH2 *fhDTMul0D4best;
      TH2 *fhDTX0D4best;
      TH2 *fhDTY0D4best;
      TH2 *fhDXX0D4best;
      TH2 *fhDXY0D4best;
      TH2 *fhDYX0D4best;
      TH2 *fhDYY0D4best;

      TH1 *fhChi04D4sbest;
      TH1 *fhTofD4sbest;
      TH1 *fhVelD4sbest;
      TH1 *fhDigiMul0D4sbest;
      TH1 *fhDigiMul4D4sbest;
      TH2 *fhCluMul04D4sbest;
      TH2 *fhDXDY04D4sbest;
      TH2 *fhDXDT04D4sbest;
      TH2 *fhDYDT04D4sbest;
      TH2 *fhDistDT04D4sbest;
      TH2 *fhTexpDT04D4sbest;
      TH2 *fhCluSize0DT04D4sbest;
      TH2 *fhCluSize4DT04D4sbest;
      TH2 *fhTot0DT04D4sbest;
      TH2 *fhTot4DT04D4sbest;
      TH2 *fhChiDT04D4sbest;
      TH2 *fhDT24DT04D4sbest;
      TH2 *fhDTD4DT04D4sbest;
      TH2 *fhX0DT04D4sbest;
      TH2 *fhY0DT04D4sbest;
      TH2 *fhDTMul4D4sbest;
      TH2 *fhDTX4D4sbest;
      TH2 *fhDTY4D4sbest;
      TH2 *fhDXX4D4sbest;
      TH2 *fhDXY4D4sbest;
      TH2 *fhDYX4D4sbest;
      TH2 *fhDYY4D4sbest;
      TH2 *fhDTMul0D4sbest;
      TH2 *fhDTX0D4sbest;
      TH2 *fhDTY0D4sbest;
      TH2 *fhDXX0D4sbest;
      TH2 *fhDXY0D4sbest;
      TH2 *fhDYX0D4sbest;
      TH2 *fhDYY0D4sbest;

      TH1 *fhNMatch24;
      TH1 *fhNMatch24sel;
      TH1 *fhDT24sel;
      TH1 *fhChi24;
      TH2 *fhXY24;
      TH2 *fhDXDY24;
      TH2 *fhDXDT24;
      TH2 *fhDYDT24;

      TH2 *fhXY0D4best;
      TH2 *fhXY4D4best;
      TH2 *fhXX04D4best;
      TH2 *fhYY04D4best;
      TH2 *fhXYSel2D4best;
      TH2 *fhXY0D4sel;
      TH2 *fhXY4D4sel;
      TH2 *fhXYSel2D4sel;

      TH1 *fhDTD4sel;
      TH1 *fhTofD4sel;
      TH1 *fhBRefMul;
      TH1 *fhDTD4;

      TH2 *fhXX02[2];
      TH2 *fhYY02[2];

      std::vector< TH2* > fhXYPos;     //[nbDet]

      TH2 *fhDT04DX0_1, *fhDT04DY0_1, *fhDT04DT0_1;
      TH2 *fhDT04DX4_1, *fhDT04DY4_1, *fhDT04DT4_1;
      TH2 *fhDT04DX0_2, *fhDT04DY0_2, *fhDT04DT0_2;
      TH2 *fhDT04DX4_2, *fhDT04DY4_2, *fhDT04DT4_2;

      // Test class performance
 
      // Rates and data rates

      // Performances check
 
      // Control
      TTimeStamp fStart;
      TTimeStamp fStop;

      TString       fCalParFileName;      // name of the file name with Calibration Parameters
      TFile*        fCalParFile;          // pointer to Calibration Parameter file 
      TH1      *fhDTD4DT04D4Off;          // 'calibration' histo
      TH1          *fhDTX4D4Off;          // 'calibration' histo
      TH1          *fhDTY4D4Off;          // 'calibration' histo
      TH1        *fhDTTexpD4Off;          // 'calibration' histo
      Double_t        fdMulDMax;          // max multiplicity in Diamond counter
      Double_t        fdDTDia;            // max time difference between diamonds
      Double_t        fdDTD4MAX;          // max time difference between reference & diamond
      Double_t        fdMul0Max;          // max multiplicity in Dut
      Double_t        fdMul4Max;          // max multiplicity in HD/Buc Ref counter
      Double_t        fdCh4Sel;           // Center of selected strip numbers 
      Double_t        fdDCh4Sel;          // Width  of selected strip numbers 
      Double_t        fdPosY4Sel;         // Fraction of Y-Size 
      Double_t        fdPosY4SelOff;      // Offset of selection interval in cm 
      Double_t        fdChS2Sel;           // Center of selected strip numbers 
      Double_t        fdDChS2Sel;          // Width  of selected strip numbers 
      Double_t        fdPosYS2Sel;         // Fraction of Y-Size 
      Double_t        fdPosYS2SelOff;      // Offset of selection interval in cm 
      Double_t        fdSel2TOff;          // Offset for matching Sel2 time

      Double_t        fdHitDistMin;       // minimal spatial distance of correlated hits 
      Double_t        fdTOffD4;           // Offset to calibrate TD4 to time of flight 
      Double_t        fdTShift;           // general time shift of all hits  
      Double_t        fdChi2Lim;          // Chi2 Limit to accept selector coincidence

      Int_t        fiCorMode; // Correction histogram generation mode
      Int_t            fiDut; // Device under test
      Int_t        fiMrpcRef; // Reference Mrpc
      Int_t       fiMrpcSel2; // Coincident Mrpc 2
      Int_t       fiMrpcSel3; // Coincident Mrpc 3
      Int_t      fiPlaSelect; // Select plastics: 0 - P2, 2 - Buc2013
      Int_t  fiBeamRefSmType; // Beam reference counter 
      Int_t    fiBeamRefSmId; // Beam reference counter 
      Int_t         fiDutNch; // Number of celss in Device under test

      ClassDef(CbmTofAnaTestbeam, 1);
};

#endif

