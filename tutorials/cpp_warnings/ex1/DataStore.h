class DataStore
{

 public:

  DataStore();

  int  GetValue() { return fValue; }
  int  GetPointerValue() { return *fPointerValue; }

  void SetValue(int _val) { fValue = _val; }

  void SetPointerValue(int _val);
  void SetPointerValueCorrect(int _val);


 private:

  int fValue, fSecondValue;
  int* fPointerValue;

};

