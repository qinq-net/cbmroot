/**
 * \file CbmLitFitQaReport.h
 * \brief Create report for fit QA.
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2011
 */
#ifndef CBMLITFITQAREPORT_H_
#define CBMLITFITQAREPORT_H_

#include "CbmSimulationReport.h"
#include "TSystem.h"

/**
 * \class CbmLitFitQaReport
 * \brief Create report for fit QA.
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2012
 */
class CbmLitFitQaReport : public CbmSimulationReport
{
public:
   /**
    * \brief Constructor.
    */
   CbmLitFitQaReport();

   /**
    * \brief Destructor.
    */
   virtual ~CbmLitFitQaReport();

protected:
   /**
    * \brief Inherited from CbmSimulationReport.
    */
   virtual void Create();

   /**
    * \brief Inherited from CbmSimulationReport.
    */
   virtual void Draw();

   std::string PrintResAndPullRow(
           const std::string& rowName,
           const std::string& histName,
           const std::string& propertyName);

	void DrawResidualAndPullHistograms(
	      const std::string& detName);

	void DrawTrackParams(
	      const std::string& detName);

	/**
	 * \fn DrawHistSigmaRMS
	 * \brief Draw sigma and RMS on histogram.
	 * \param[in] sigma Sigma value.
	 * \param[in] rms RMS value.
	 */
	void DrawHistSigmaRMS(
	   Double_t sigma,
	   Double_t rms);

	void DrawTrackParamsAtVertex();
        
        void DrawTrackMomentumAtVertex();

	ClassDef(CbmLitFitQaReport, 1);
};

#endif /* CBMLITFITQAREPORT_H_ */
