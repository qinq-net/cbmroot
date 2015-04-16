
#include "CbmAnaJpsiTask.h"

using namespace std;

CbmAnaJpsiTask::CbmAnaJpsiTask()
  : FairTask("CbmAnaJpsiTask")

{
}

CbmAnaJpsiTask::~CbmAnaJpsiTask()
{

}

InitStatus CbmAnaJpsiTask::Init()
{
   //cout << "CbmRichUrqmdTest::Init"<<endl;
   FairRootManager* ioman = FairRootManager::Instance();
   if (NULL == ioman) { Fatal("CbmAnaJpsiTask::Init","RootManager not instantised!"); }

   return kSUCCESS;
}

void CbmAnaJpsiTask::Exec(
      Option_t* option)
{
  /// fEventNum++;

 //  cout << "CbmRichUrqmdTest, event No. " <<  fEventNum << endl;


}


void CbmAnaJpsiTask::Finish()
{

}


ClassImp(CbmAnaJpsiTask)

