class DataContainer
{

 public:

  DataContainer();

  int  GetValue(int i); 
  void SetValue(int index, int val);
  void PrintAllValues();

 private:

  static const int maxIndex = 100;
  int* fValue;
};

