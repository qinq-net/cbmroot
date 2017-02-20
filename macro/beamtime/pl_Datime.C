void pl_Datime(){
  pTime=new TDatime();
  cout << " add time stamp to canvas " <<pTime->GetDate() << ", "<<pTime->GetTime() <<endl;
  TPaveText *pt = new TPaveText(.01,.97,.1,.99);
  pt->AddText(Form("%d,%d",pTime->GetDate(),pTime->GetTime()));
  pt->Draw();
}
