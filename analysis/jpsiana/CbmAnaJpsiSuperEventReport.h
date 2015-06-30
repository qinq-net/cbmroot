
#ifndef CBMANAJPSISUPEREVENTREPORT_H
#define CBMANAJPSISUPEREVENTREPORT_H

#include "CbmSimulationReport.h"
#include "CbmAnaJpsiHist.h"
#include "TSystem.h"

class CbmAnaJpsiSuperEventReport : public CbmSimulationReport
{
public:
   /**
    * \brief Constructor.
    */
	CbmAnaJpsiSuperEventReport();

   /**
    * \brief Destructor.
    */
   virtual ~CbmAnaJpsiSuperEventReport();

   /*
    * \brief Create report.
    */
   void Create(
   		const string& fileEventByEvent,
   		const string& fileSuperEvent,
   		const string& outputDir);

protected:
   /**
    * \brief Inherited from CbmSimulationReport.
    */
   virtual void Create();

   /**
    * \brief Inherited from CbmSimulationReport.
    */
   virtual void Draw();

   /*
    * \brief Draw comparison between superevent and event-by-event
    */
   void DrawComparison();


   CbmHistManager* fHMSuperEvent; // histogram manager for super event histograms
   CbmHistManager* fHMEventByEvent; //histogram manager for event-by-event histograms



	ClassDef(CbmAnaJpsiSuperEventReport, 1);
};

#endif

