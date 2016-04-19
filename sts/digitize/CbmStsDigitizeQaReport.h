#ifndef CBMSTSDIGITIZEQAREPORT_H_
#define CBMSTSDIGITIZEQAREPORT_H_

#include "CbmSimulationReport.h"
#include <string>

class CbmStsSetup;
class CbmStsDigitize;

using std::string;

class CbmStsDigitizeQaReport : public CbmSimulationReport
{
    public:
	CbmStsDigitizeQaReport(CbmStsSetup * setup, CbmStsDigitize * digitizer);
	virtual ~CbmStsDigitizeQaReport();


    private:
	CbmStsSetup * fSetup;
	CbmStsDigitize * fDigitizer;
	virtual void Create();
	virtual void Draw();
	void DrawNofObjectsHistograms();
	void DrawLogHistograms();
	void Draw2dHistograms();
	void ScaleHistograms();

	CbmStsDigitizeQaReport(const CbmStsDigitizeQaReport&);
	CbmStsDigitizeQaReport& operator=(const CbmStsDigitizeQaReport&);

	ClassDef(CbmStsDigitizeQaReport, 1)
};

#endif
