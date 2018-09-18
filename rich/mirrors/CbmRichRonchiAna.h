
#ifndef RICH_MIRRORS_CBMRICHRONCHIANA_H_
#define RICH_MIRRORS_CBMRICHRONCHIANA_H_

#include "TObject.h"
#include <string>
#include <vector>

#include <cmath>

using namespace std;

class CbmRichRonchiAna : public TObject 
{
public:
    CbmRichRonchiAna();
    
	virtual ~CbmRichRonchiAna();

	void Run();

	void SetTiffFileNameV(const string& fileName) {fTiffFileNameV = fileName;}
	void SetTiffFileNameH(const string& fileName) {fTiffFileNameH = fileName;}

private:

	string fTiffFileNameV;
	string fTiffFileNameH;

	vector<vector<int> > ReadTiffFile(const string& fileName);

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


