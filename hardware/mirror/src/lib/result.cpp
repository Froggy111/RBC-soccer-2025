#include "lib/result.hpp"

namespace result {
  using namespace std;

  void panic(string err_msg, string function_name) {
    cerr << "Error in " + function_name + ":\n" + err_msg << endl;
    exit(1);
  }
}