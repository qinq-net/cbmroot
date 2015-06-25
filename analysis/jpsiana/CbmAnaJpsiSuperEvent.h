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
#include "TClonesArray.h"

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
	TClonesArray* fCandidates;
	vector<string> fFileNames;
	vector<CbmAnaJpsiCandidate> fMinusCandidates;
	vector<CbmAnaJpsiCandidate> fPlusCandidates;

	void ReadCandidates();

	void DoSuperEvent();

	ClassDef(CbmAnaJpsiSuperEvent, 1);
};

#endif /* ANALYSIS_JPSIANA_CBMANAJPSISUPEREVENT_H_ */
