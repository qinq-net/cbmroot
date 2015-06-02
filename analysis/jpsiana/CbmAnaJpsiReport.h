
#ifndef CBMANAJPSIREPORT_H
#define CBMANAJPSIREPORT_H

#include "CbmSimulationReport.h"
#include "CbmAnaJpsiHist.h"
#include "TSystem.h"

class CbmAnaJpsiReport : public CbmSimulationReport
{
public:
   /**
    * \brief Constructor.
    */
	CbmAnaJpsiReport();

   /**
    * \brief Destructor.
    */
   virtual ~CbmAnaJpsiReport();

protected:
   /**
    * \brief Inherited from CbmSimulationReport.
    */
   virtual void Create();

   /**
    * \brief Inherited from CbmSimulationReport.
    */
   virtual void Draw();

   void DrawAnalysisStepsH2(
         const string& hName,
		 bool DoDrawEfficiency);

   void DrawAnalysisStepsH1(
            const string& hName,
            bool doScale);

   void DrawSourceTypesH1(
         const string& hName,
         bool doScale = true);

   void DrawCutH1(
         const string& hName,
         double cutValue);

   void Draw2DCut(
         const string& hist);

   void DrawCutDistributions();

   void DrawMinvMismatches(
   		int step);

   void DrawMinvMismatchesAll();

   void DrawEfficiency(const string& h,
		   const string& hMC);

   void DrawPtYEfficiency(
			int step);

   void DrawPtYEfficiencyAll();

   void SetAnalysisStepLabels(
         TH1* h);

   void DrawBgSource2D(
         const string& histName,
         const vector<string>& yLabels,
         const string& zTitle);


	ClassDef(CbmAnaJpsiReport, 1);
};

#endif /* CBMANAJPSIREPORT_H */

