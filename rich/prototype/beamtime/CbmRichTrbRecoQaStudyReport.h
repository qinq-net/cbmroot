
#ifndef CBMRICHTRBRECOQASTUDYREPORT_H
#define CBMRICHTRBRECOQASTUDYREPORT_H

#include "CbmStudyReport.h"
#include <string>
using std::string;
class TH1;


class CbmRichTrbRecoQaStudyReport : public CbmStudyReport
{
public:
   /**
    * \brief Constructor.
    */
	CbmRichTrbRecoQaStudyReport();

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichTrbRecoQaStudyReport();

protected:
    /**
    * \brief Inherited from CbmLitStudyReport.
    */
   void Create();

	/**
	* \brief Inherited from CbmLitStudyReport.
	*/
	void Draw();


	void FitGausAndDrawH1(
			const string& histName,
			const string& canvasName);

   ClassDef(CbmRichTrbRecoQaStudyReport, 1)
};

#endif
