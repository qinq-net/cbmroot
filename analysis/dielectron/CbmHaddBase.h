/**
 * @author Elena Lebedeva <e.lebedeva@gsi.de>
 * @since 2015
 * @version 1.0
 **/


#ifndef CBM_HADD_BASE_H
#define CBM_HADD_BASE_H

#include "TObject.h"

using namespace std;

class CbmHaddBase: public TObject {

public:
	/*
	 *\brief Hadd all files in a dir with a specified fileTemplate.
	 *\param dir Path to the directory with files
	 *\param fileTemplate [addString].auau.25gev.[00000][.root] [] - will be add to the file name automatically
	 *\param addString analysis or litqa
	 *\param nofFiles number Of files to add
	 */
	static void AddFilesInDir(
			const string& dir,
			const string& fileTemplate,
			const string& addString,
			Int_t nofFiles);

private:
	static TFile* CreateAndMergeTempTargetFile(
			const string& dir,
			const string& addString,
			Int_t targetFileNum,
			TList* fileList);

	/*
	 * \brief Close and delete files in the list.
	 */
	static void CloseFilesFromList(
		    TList* fileList);

	/*
	 * \brief Check that file is not NULL and file size is more then a size limit.
	 */
	static Bool_t CheckFileSize(
			TFile* file,
			Int_t fileSizeLimit = 50000);

	/*
	 * \brief Check that file is not NULL, file size is more than a limit, number of events is equal to nofEvents
	 */
	static Bool_t CheckFile(
			TFile* file,
			Int_t fileSizeLimit = 50000,
			Int_t nofEvents = 1000);

	/*
	 * \brief Merge root file into one. This function was taken from hadd.C macro.
	 */
	static void MergeRootfile(
			TDirectory *target,
			TList *sourcelist );

	ClassDef(CbmHaddBase,1);
};


#endif
