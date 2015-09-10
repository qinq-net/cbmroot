// Functions needed for CTest runtime dependency
// The only way to introduce such a runtime dependency is via a file
// which is created at the end of the macro if everything worked as
// expected
void Generate_CTest_Dependency_File(TString filename)
{
  TString touchCommand = "touch " + filename;
  gSystem->Exec(touchCommand);
}

TString Remove_CTest_Dependency_File(TString outDir, TString macroName, const char* setup = "")
{
  TString _setup(setup);
  TString testDir = outDir;
  TString testFile = "";

  if (_setup.EqualTo("")) {
    testFile = macroName + "_ok";
  } else {
    testFile = macroName + "_" + _setup + "_ok";
  }
  TString depFile =  outDir + "/" + testFile;
  
  if (gSystem->FindFile(testDir, testFile)) {
    TString rmCommand = "rm " + testFile;
    gSystem->Exec(rmCommand);
  }
  depFile.ReplaceAll("//", "/");
  return depFile;
}
