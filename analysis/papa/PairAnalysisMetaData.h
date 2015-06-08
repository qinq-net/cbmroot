#ifndef PAIRANALYSISMETADATA_H
#define PAIRANALYSISMETADATA_H
///////////////////////////////////////////////////////////////////////////////////////////
//                                                                                       //
// Meta data container:                                                                  //
// information that can be used for plotting, qa, trending, ....                         //
//                                                                                       //
//   Julian Book   <Julian.Book@cern.ch>                                                 //
//                                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////

#include <Rtypes.h>

#include <TNamed.h>
// #include <TCollection.h>
#include <TList.h>
/* #include <TParamater<Int_t>.h> */
/* #include <TParamater<Double_t>.h> */

#include <TList.h> //new

class TString;
class TList;
class TParmameter;
// class TVectorT<double>;

class PairAnalysisMetaData : public TNamed {
public:

  PairAnalysisMetaData();
  PairAnalysisMetaData(const char* name, const char* title);
  virtual ~PairAnalysisMetaData();

  void Init();

  // functions to fill
  void FillMeta(const char *name, Double_t val);
  void FillMeta(const char *name, Int_t    val);
  void FillMeta(const char *name, const char *val);

  // functions to get and draw histograms
  //  void ReadFromFile(const char* file="metadata.root", const char *task="", const char *config="");
  //  void SetList(TList * const list)          { fList=list; }
  //  TList *GetList()                    const { return fList; }

  void SetMetaData(TList &list, Bool_t setOwner=kTRUE);
  void ResetMetaData()                 { fMetaList.Clear(); }
  const TList* GetMetaData() const { return &fMetaList; }

  TObject* FindObject(const char *name) { return fMetaList.FindObject(name); }


  //  virtual void Print(const Option_t* option = "") const;
  void DrawSame(const Option_t *opt="leg can");
  //  virtual void Draw(const Option_t* option = "");

private:

  TList fMetaList;             //-> list of parameters
  //  TList    *fList;                  //! List of list of histograms

  PairAnalysisMetaData(const PairAnalysisMetaData &hist);
  PairAnalysisMetaData& operator = (const PairAnalysisMetaData &hist);

  ClassDef(PairAnalysisMetaData,1)
};
#endif
