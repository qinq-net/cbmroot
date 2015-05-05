
#ifndef CBMANAJPSIREPORT_H
#define CBMANAJPSIREPORT_H

#include "CbmSimulationReport.h"
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


	ClassDef(CbmAnaJpsiReport, 1);
};

#endif /* CBMANAJPSIREPORT_H */

