#ifndef log_h_included
#define log_h_included

#include <fstream>
#include "loadConfig.hpp"

class Logging {
  public:
    Logging(Configuration& config);
    void Close();
    void Write(std::string str);

  private:
    std::string logFile;
    std::ofstream inFile;
};

#endif
