#ifndef CBM_RICH_SMALL_PROTOTYPE_STUDY_REPORT
#define CBM_RICH_SMALL_PROTOTYPE_STUDY_REPORT

#include "CbmStudyReport.h"

/**
 * \class CbmRichSmallPrototypeStudyReport
 * \brief Creates study report for RICH small prototype analysis.
 *
 */
class CbmRichSmallPrototypeStudyReport : public CbmStudyReport
{
public:
    /**
     * \brief Constructor.
     */
    CbmRichSmallPrototypeStudyReport();
    
    /**
     * \brief Destructor.
     */
    virtual ~CbmRichSmallPrototypeStudyReport();
    
protected:
    /**
     * \brief Inherited from CbmLitStudyReport.
     */
    void Create();
    
    /**
     * \brief Inherited from CbmLitStudyReport.
     */
    void Draw();
    
    /**
     * \brief
     */
    void DrawNofHistAll();

    /**
     * \brief
     */
    void DrawNofHist(
    		const std::string& histName);
    
    ClassDef(CbmRichSmallPrototypeStudyReport, 1)
    
};

#endif
