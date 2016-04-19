/** @file CbmStsTimeBasedQaReport.h
 ** @author Grigory Kozlov <g.kozlov@gsi.de>
 ** @date 19.04.2016
 **/

#ifndef CBMSTSTIMEBASEDQAREPORT_H_
#define CBMSTSTIMEBASEDQAREPORT_H_

#include "CbmSimulationReport.h"
#include <string>

using std::string;

class CbmStsTimeBasedQaReport : public CbmSimulationReport
{
public:
  CbmStsTimeBasedQaReport();

  CbmStsTimeBasedQaReport(Bool_t useDaq);

  virtual ~CbmStsTimeBasedQaReport();
private:
  virtual void Create();

  virtual void Draw();

  void DrawNofObjectsHistograms(const string& type);

  void DrawResidualAndPullHistograms(const string& type);

  void DrawPointsInHitHistograms(const string& type);

  void DrawDigiPerObjectHistograms(const string& type);

  void DrawHistograms(const string& type);

  void ScaleHistograms(const string& type);

  Bool_t fDaq;

  ClassDef(CbmStsTimeBasedQaReport, 1)
};

#endif
