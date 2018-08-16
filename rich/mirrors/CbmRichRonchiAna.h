
#ifndef RICH_MIRRORS_CBMRICHRONCHIANA_H_
#define RICH_MIRRORS_CBMRICHRONCHIANA_H_

#include "TObject.h"

using namespace std;

class CbmRichRonchiAna : public TObject {
public:
    CbmRichRonchiAna();
    
	virtual ~CbmRichRonchiAna();

	void Run();

private:


	/**
	* \brief Copy constructor.
	*/
	CbmRichRonchiAna(const CbmRichRonchiAna&);

	/**
	* \brief Assignment operator.
	*/
	CbmRichRonchiAna& operator=(const CbmRichRonchiAna&);
};

#endif /* RICH_MIRRORS_CBMRICHRONCHIANA_H_ */
