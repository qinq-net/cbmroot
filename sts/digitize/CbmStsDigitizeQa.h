#ifndef CBMSTSDIGITIZEQA_H_
#define CBMSTSDIGITIZEQA_H_

#include "FairTask.h"
#include "CbmStsDigi.h"
#include "CbmStsDigitize.h"
//#include "CbmTimeSlice.h"
#include "CbmHistManager.h"
#include "CbmMCDataArray.h"
#include <fstream>
#include <vector>
class TClonesArray;

using std::string;
using std::vector;

class CbmStsDigitizeQa : public FairTask
{
    public:
	CbmStsDigitizeQa(CbmStsDigitize * digitizer);

	virtual ~CbmStsDigitizeQa();

	virtual InitStatus Init();

	virtual void Exec(Option_t* opt);

	virtual void Finish();

	void SetOutputDir(const string& outputDir) { fOutputDir = outputDir; }

	void CreateHistograms();

	void CreateNofObjectsHistograms();

	void CreateDigiHistograms();

	void ProcessDigisAndPoints(const TClonesArray* digis, const TClonesArray * points);
void ProcessAngles();

    private:
	void ReadDataBranches();

	CbmHistManager* fHM;
	CbmHistManager* fHMStation;
	string fOutputDir;
	CbmStsDigitize * fDigitizer;
	CbmStsSetup * fSetup;
	Int_t fNofStation;

	TClonesArray* fStsDigis;
	TClonesArray* fStsPoints;
	vector<CbmStsDigi> fStsDigiData;

	Int_t fMaxScale;
	ofstream fOutFile;
	vector < vector <vector <vector < vector <Int_t>>>>> fnOfDigisChip;
	

	CbmStsDigitizeQa(const CbmStsDigitizeQa&);
	CbmStsDigitizeQa& operator=(const CbmStsDigitizeQa&);

	ClassDef(CbmStsDigitizeQa, 1);
};

#endif
