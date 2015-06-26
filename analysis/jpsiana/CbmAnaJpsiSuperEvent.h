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

class CbmAnaJpsiSuperEvent  : public CbmSimulationReport
{

public:
	CbmAnaJpsiSuperEvent();

	virtual ~CbmAnaJpsiSuperEvent();

	/*
	 * \brief Add file for super event.
	 */

	void AddFile(const string& fileName) { fFileNames.push_back(fileName); }

	void Run();
protected:
	virtual void Create();

	virtual void Draw();

//private:
	vector<string> fFileNames;
	vector<CbmAnaJpsiCandidate> fMinusCandidates;
	vector<CbmAnaJpsiCandidate> fPlusCandidates;

	TH1D* fMinv;
	TH1D* fh_SuperEvent_signal_minv_Reco;
	TH1D* fh_SuperEvent_signal_minv_Chi2Primary;
	TH1D* fh_SuperEvent_signal_minv_ElID;
	TH1D* fh_SuperEvent_signal_minv_Pt;

	CbmHistManager* fHM;
	CbmAnaJpsiCuts fCuts;



	void InitHist();

	void ReadCandidates();

	void DoSuperEvent();

	void CreateAnalysisStepsH1(
	      const string& name,
	      const string& axisX,
	      const string& axisY,
	      double nBins,
	      double min,
	      double max
	      );

	ClassDef(CbmAnaJpsiSuperEvent, 1);
};

#endif /* ANALYSIS_JPSIANA_CBMANAJPSISUPEREVENT_H_ */
