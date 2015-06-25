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
#include "TH1D.h"

using namespace std;

class CbmAnaJpsiSuperEvent {

public:
	CbmAnaJpsiSuperEvent();

	virtual ~CbmAnaJpsiSuperEvent();

	/*
	 * \brief Add file for super event.
	 */
	void AddFile(const string& fileName) { fFileNames.push_back(fileName); }

	void Run();

private:
	vector<string> fFileNames;
	vector<CbmAnaJpsiCandidate> fMinusCandidates;
	vector<CbmAnaJpsiCandidate> fPlusCandidates;

	TH1D* fMinv;

	void InitHist();

	void ReadCandidates();

	void DoSuperEvent();

	ClassDef(CbmAnaJpsiSuperEvent, 1);
};

#endif /* ANALYSIS_JPSIANA_CBMANAJPSISUPEREVENT_H_ */
