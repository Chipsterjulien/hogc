#include "log.hpp"
#include "hog.hpp"

Logging::Logging(Configuration& config) {
  this->inFile.open(config.getLogFile(), std::ios::out | std::ios::ate);

  if(inFile.bad()) {
    std::stringstream errorStr;

    errorStr << "Unable to open log file: " << config.getLogFile() << std::endl;
    std::cerr << errorStr.str();
    std::cout << errorStr.str();

    exit(EXIT_FAILURE);
  }
}

void Logging::Close() {
  inFile.close();
}

void Logging::Write(std::string str) {
  const int MAXLEN = 11;
  char s[MAXLEN];
  time_t t = time(0);
  strftime(s, MAXLEN, "%02d/%02m/%Y", localtime(&t));

  inFile << "[ " << s << " ]: " << str << std::endl;
  inFile.flush();
}
