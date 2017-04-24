#ifndef _L1HitPoint_h_
#define _L1HitPoint_h_

		/// contain strips positions and coordinates of hit
#if 1
struct L1HitPoint{
  L1HitPoint(): x(0.f), y(0.f), z(0.f), u(0.f), v(0.f), time(0.f) , n(-1),
  //timeEv(0.f), 
  timeEr(2.9f){};
  L1HitPoint(fscal x_, fscal y_, fscal z_, fscal v_, fscal u_, float time_, unsigned short int n_ = 0, 
             //float timeEv1_=0,
             float timeEr_=2.9f):
    x(x_), y(y_), z(z_), u(u_), v(v_), time(time_) , n(n_),
    //timeEv(timeEv1_),
    timeEr(timeEr_){};
    
  //  L1HitPoint(fscal x_, fscal y_, fscal z_, fscal v_, fscal u_, fscal time_, unsigned short int n_ = 0):
  //  x(x_), y(y_), z(z_), u(u_), v(v_), time(time_){};

  fscal Xs() const { return X()/Z(); }
  fscal Ys() const { return Y()/Z(); } // value to sort hits by

  fscal X() const { return x; }
  fscal Y() const { return y; }
  fscal Z() const { return z; }
  fscal U() const { return u; }
  fscal V() const { return v; }
 // unsigned short int N() const { return n; }
 // int GetSortIndex() const { return SortIndex; }
  void SetX(fscal X1) { x = X1; }
  void SetY(fscal Y1) { y = Y1; }
  void SetZ(fscal Z1) { z = Z1; }
  void SetU(fscal U1) { u = U1; }
  void SetV(fscal V1) { v = V1; }
    
  void Set(fscal &x1, fscal &y1, const float &z1, const fscal &v1, const fscal &u1, const float &time1, const short int &n1, 
        //   float timeEv1, 
           float timeEr1) {
      x = x1;
      y = y1;
      z = z1;
      u = u1;
      v = v1;
      time = time1;
      n = n1;
     // timeEv =timeEv1;
      timeEr =timeEr1;
  }
  void SetN(unsigned short int N1) { n = N1; }
//  void SetSortIndex(int SortIndex1) { SortIndex = SortIndex1; }
  
 private:
  
  float x, y;
  float z; // TODO: may be we should use iz
  float u, v; // x\u, v - front and back strips; x, y, z - coordinates of hits
        
 
//  int SortIndex;
  public:
    float time, 
    //timeEv, 
    timeEr;
    short int n; // number of event
};
#else

static const float R = 60;
static const float shortPackingConstant = 2 * R / 65535.f;
static const float MZ = 110;
static const float shortPackingConstantZ = MZ / 65535.f;

		/// contain strips positions and coordinates of hit
struct L1HitPoint{
  L1HitPoint(){};
  L1HitPoint(fscal x_, fscal y_, fscal z_, fscal v_, fscal u_, unsigned short int n_ = 0): x(f2s(x_)), y(f2s(y_)), z(f2sZ(z_)), u(f2s(u_)), v(f2s(v_)), n(n_){};

  fscal Xs() const { return X()/Z(); }
  fscal Ys() const { return Y()/Z(); } // value to sort hits by

  fscal X() const { return s2f(x); }
  fscal Y() const { return s2f(y); }
  fscal Z() const { return s2fZ(z); }
  fscal U() const { return s2f(u); }
  fscal V() const { return s2f(v); }

  unsigned short int N() const { return n; }
 private:
  //unsigned short int
  unsigned short int f2s(float f) const {
    return (f + R)/shortPackingConstant;
  }
  float s2f(unsigned short int f) const {
    return (float(f)+0.5)*shortPackingConstant - R;
  }

  unsigned short int f2sZ(float f) const {
    return (f)/shortPackingConstantZ;
  }
  float s2fZ(unsigned short int f) const {
    return (float(f)+0.5)*shortPackingConstantZ;
  }
  
  unsigned short int x, y;
  unsigned short int z; // TODO: may be we should use iz
  unsigned short int u, v; // x\u, v - front and back strips; x, y, z - coordinates of hits
        
  unsigned short int n; // number of event
  
 
};

#endif

#endif


