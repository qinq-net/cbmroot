/*
 * CbmAnaJpsiSuperEvent.h
 *
 *  Created on: Jun 25, 2015
 *      Author: slebedev
 */

#ifndef ANALYSIS_JPSIANA_CBMANAJPSISUPEREVENT_H_
#define ANALYSIS_JPSIANA_CBMANAJPSISUPEREVENT_H_
#include <string>
#include "CbmAnaJpsiCandidate.h"
#include "CbmAnaJpsiHist.h"
#include "CbmAnaJpsiCuts.h"
#include "CbmAnaJpsiKinematicParams.h"
#include "CbmHistManager.h"
#include "TH1D.h"
#include "CbmSimulationReport.h"
#include "TSystem.h"

using namespace std;

class CbmAnaJpsiSuperEvent  : public TObject
{

public:
	CbmAnaJpsiSuperEvent();

	virtual ~CbmAnaJpsiSuperEvent();

	/*
	 * \brief Add file for super event.
	 */
	void AddFile(const string& fileName) { fFileNames.push_back(fileName); }

	/*
	 * \brief Set output file.
	 */
	void SetOutputFile(const string& fileName) {fOutputFile = fileName;}

	/*
	 * \brief Set to true if you want to do super event after ptcut.
	 * This will speed up calculation time.
	 */
	void SetRunAfterPtCut(Bool_t b){fRunAfterPtCut = b;}

	void Run();

private:
	vector<string> fFileNames;
	vector<CbmAnaJpsiCandidate> fMinusCandidates;
	vector<CbmAnaJpsiCandidate> fPlusCandidates;

	string fOutputFile;

	CbmHistManager* fHM;

	CbmAnaJpsiCuts fCuts;

	Bool_t fRunAfterPtCut;

	void InitHist();

	void ReadCandidates();

	void DoSuperEvent();

	void Draw();


	ClassDef(CbmAnaJpsiSuperEvent, 1);
};

#endif /* ANALYSIS_JPSIANA_CBMANAJPSISUPEREVENT_H_ */
