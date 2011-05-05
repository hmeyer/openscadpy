#ifndef ACCURACY_H
#define ACCURACY_H
  struct Accuracy {
    double fn;
    double fs;
    double fa;
    Accuracy():fn(0.0),fs(1.0),fa(12.0) {}
    Accuracy(double fn, double fs, double fa):fn(fn),fs(fs),fa(fa) {}
  };

int get_fragments_from_r(double r, const Accuracy &acc);

#endif
