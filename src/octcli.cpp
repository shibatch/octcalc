#include <iostream>

#include "octcore.hpp"

int main(int argc, char **argv) {
  octcore::OctCore core;

  for(int i=1;i<argc;i++) {
    pair<string, tlfloat_octuple> p = core.execute(argv[i]);

    cout << p.first << endl;
    cout << tlfloat::to_string(p.second, 75) << endl;
  }

  return 0;
}
