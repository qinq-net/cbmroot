#ifndef STSCOSYBL_H
#define STSCOSYBL_H

#include "FairTask.h"
#include "TH1F.h"
#include "TH2F.h"

#include "CbmFiberHodoAddress.h"
#include "CbmFiberHodoDigi.h"

using std::vector;
class TClonesArray;

class StsCosyBL : public FairTask
{
  public:

    /** Default constructor **/
    StsCosyBL();

    /** Constructor with parameters (Optional)
    //  StsCosyBL(Int_t verbose);
    **/

    /** Destructor **/
    ~StsCosyBL();


    /** Initiliazation of task at the beginning of a run **/
    virtual InitStatus Init();

    /** ReInitiliazation of task when the runID changes **/
    virtual InitStatus ReInit();


    /** Executed for each event. **/
    virtual void Exec(Option_t* opt);

    /** Load the parameter container from the runtime database **/
    virtual void SetParContainers();

    /** Recursive FinishEvent of subtasks **/
    virtual void FinishEvent();

    /** Finish task called at the end of the run **/
    virtual void Finish();
    //    virtual Int_t AddFile( const char* name ); 
    void BLInit();
    void BLClean();
    void BaseLine(TClonesArray* fBaselineDigis,     vector< vector < vector < double> > > base_line_array);
    void HodoBaseLine(TClonesArray* fBaselineDigis, vector< vector < vector < double> > > base_line);

    void SetTriggeredMode(Bool_t mode) { fTriggeredMode = mode; }
    void SetTriggeredStation(Int_t station) { fTriggeredStation = station ; }

    virtual void Reset();    

    Int_t GetEntries () { return fChain->GetEntries();}

 private:
    TClonesArray*     auxDigis;       /** Input array of CbmFiberHodoDigi **/
    TClonesArray*     fDigis;       /** Input array of CbmFiberHodoDigi **/
    TClonesArray*     fBLDigis;
    TClonesArray*     hBLDigis;
    TClonesArray*     hDigis;
    TClonesArray*     cDigis;
    TClonesArray*     chDigis;

    TChain *fChain;                   //!
    TFile *outFile;                   //!
    static Double_t GetBlPos( TH1F * hist );
    static const Int_t kBaselineMinAdc = 0;
    static const Int_t kBaselineMaxAdc = 4096;
    static const Int_t kBaselineNBins = 4096;


    TH2F *baseline_ch[3][2];       //!
    TH2F *raw_ch[3][2];            //!
    TH2F *raw_ch_woBL[3][2];       //!
    TH2F *calibr_ch[3][2];         //!
    TH1F *calibr_ch1D[3][2];       //!
    
    TH2F *hodo_baseline_ch[2][2];  //!
    TH2F *hodo_calib_ch[2][2];     //!

    Bool_t calib;
    Bool_t hodo_calib;

    Bool_t fTriggeredMode; ///< Flag if data is taken in triggered mode
    Int_t  fTriggeredStation;
    Int_t  fNofEvent;

    vector< vector < vector < double> > > base_line_array;
    vector< vector < vector < double> > > hodo_BL_array;
    /** Input array from previous already existing data level **/
    //  TClonesArray* <InputDataLevel>;

    /** Output array to  new data level**/
    //  TClonesArray* <OutputDataLevel>;

    StsCosyBL(const StsCosyBL&);
    StsCosyBL operator=(const StsCosyBL&);

    ClassDef(StsCosyBL,1);
};

#endif
