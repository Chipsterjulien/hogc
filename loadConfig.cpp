#include "loadConfig.hpp"
#include "hog.hpp"

Configuration::Configuration() {
  this->debug = true;
  this->exitIfNoFrame = false;
  // Autodetect default API
  this->camNumber = (0 + cv::CAP_ANY);
  this->fps = 0;
  this->sleepTime = 0;
  this->videoWidth = 320;
  this->videoHeight = 240;
  this->jpgQuality = 75;
  this->ratioResizeWidth = 1;
  this->ratioResizeHeight = 1;
  this->confFile = "/etc/hogc/hogc.ini";
  this->camStream = "";
  this->imageFile = "";
  this->recordPath = "/tmp";
  this->logFile = "/var/log/hogc/errors.log";
  this->detection = "default";
  hitThreshold = 0;
  winStride = cv::Size(0, 0);
  padding = cv::Size(0, 0);
  scale = 1.05;
  finalThreshold = 2.0;
  useMeanshiftGrouping = false;
}

void Configuration::Clear() {
  data.clear();
}

void Configuration::GetConfig() {
  int timeInt = 0;
  int wX = 0, wY = 0, pX = 0, pY = 0;

  this->Get("debug", this->debug);
  this->Get("exitIfNoFrame", this->exitIfNoFrame);
  this->Get("camNumber", this->camNumber);
  this->Get("fps", this->fps);
  this->Get("videoWidth", this->videoWidth);
  this->Get("videoHeight", this->videoHeight);
  this->Get("jpgQuality", this->jpgQuality);
  this->Get("ratioResizeWidth", this->ratioResizeWidth);
  this->Get("ratioResizeHeight", this->ratioResizeHeight);
  this->Get("sleepTime", timeInt);
  this->sleepTime = (useconds_t)(timeInt);
  this->Get("camStream", this->camStream);
  this->Get("imageFile", this->imageFile);
  this->Get("recordPath", this->recordPath);
  this->Get("logFile", this->logFile);

  this->Get("detection", this->detection);
  this->Get("hitThreshold", this->hitThreshold);

  this->Get("winStrideX", wX);
  this->Get("winStrideY", wY);
  this->winStride = cv::Size(wX, wY);

  this->Get("paddingX", pX);
  this->Get("paddingY", pY);
  this->padding = cv::Size(pX, pY);

  this->Get("scale", this->scale);
  this->Get("finalThreshold", this->finalThreshold);
  this->Get("useMeanshiftGrouping", this->useMeanshiftGrouping);
}

void Configuration::GetParams(int argc, char **argv) {
  int option = 0;

  while((option = getopt(argc, argv, "hi:")) != EOF) {
    switch(option) {
      case 'h':
        help();
        exit(EXIT_SUCCESS);
        break;

      case 'i':
        this->confFile = optarg;
        break;

      default:
        help();
        exit(EXIT_FAILURE);
        break;
    }
  }
}

void Configuration::ShowAllValues() {
  std::cout << "Debug mode: " << this->debug << std::endl;
  std::cout << "Exit if no frame (daemon mode): " << this->exitIfNoFrame << std::endl;
  std::cout << "Cam number: " << this->camNumber << std::endl;
  std::cout << "Cam stream: " << this->camStream << std::endl;
  std::cout << "Image file: " << this->imageFile << std::endl;
  std::cout << "Jpg quality: " << this->jpgQuality << std::endl;
  std::cout << "Video width (camera): " << this->videoWidth << std::endl;
  std::cout << "Video height (camera): " << this->videoHeight << std::endl;
  std::cout << "Sleep time: " << this->sleepTime << std::endl;
  std::cout << "Frame rate: " << this->fps << std::endl;
  std::cout << "Output record file: " << this->recordPath << std::endl;
  std::cout << "Ratio width: " << this->ratioResizeWidth << std::endl;
  std::cout << "Ratio height: " << this->ratioResizeHeight << std::endl;
  std::cout << "Config file path: " << this->confFile << std::endl;
  std::cout << "Log file path: " << this->logFile << std::endl;
  std::cout << "Detect human: " << this->detection << std::endl;
  std::cout << "HitThreshold: " << this->hitThreshold << std::endl;
  std::cout << "WinStride: " << this->winStride << std::endl;
  std::cout << "Padding: " << this->padding << std::endl;
  std::cout << "Scale: " << this->scale << std::endl;
  std::cout << "FinalThreshold: " << this->finalThreshold << std::endl;
  std::cout << "UseMeanshiftGrouping: " << this->useMeanshiftGrouping << std::endl;
}

bool Configuration::Load() {
  std::ifstream inFile((this->confFile).c_str());

  if (!inFile.good()) {
    std::cerr << "Cannot read configuration file \"" << this->confFile << "\"" << std::endl;

    return false;
  }

  while(inFile.good() && ! inFile.eof()) {
    std::string line = "";

    getline(inFile, line);
    // filter out comments
    if (!line.empty()) {
      int pos = line.find('#');

      if (pos != std::string::npos) {
          line = line.substr(0, pos);
      }
    }

    // split line into key and value
    if (!line.empty()) {
      int pos = line.find('=');

      if (pos != std::string::npos) {
        std::string key     = Trim(line.substr(0, pos));
        std::string value   = Trim(line.substr(pos + 1));

        if (!key.empty() && !value.empty()) {
            data[key] = value;
        }
      }
    }
  }

  this->GetConfig();

  return true;
}

bool Configuration::Contains(const std::string& key) const {
  return data.find(key) != data.end();
}

bool Configuration::Get(const std::string& key, std::string& value) const {
  std::map<std::string,std::string>::const_iterator iter = data.find(key);

  if (iter != data.end()) {
      value = iter->second;
      return true;
  } else {
      return false;
  }
}

bool Configuration::Get(const std::string& key, int& value) const {
  std::string str;

  if (Get(key, str)) {
      value = atoi(str.c_str());
      return true;
  } else {
      return false;
  }
}

bool Configuration::Get(const std::string& key, long& value) const {
  std::string str;

  if (Get(key, str)) {
      value = atol(str.c_str());
      return true;
  } else {
      return false;
  }
}

bool Configuration::Get(const std::string& key, double& value) const {
  std::string str;

  if (Get(key, str)) {
      value = atof(str.c_str());
      return true;
  } else {
      return false;
  }
}

bool Configuration::Get(const std::string& key, bool& value) const {
  std::string str;

  if (Get(key, str)) {
      value = (str == "true");
      return true;
  } else {
      return false;
  }
}

std::string Configuration::Trim(const std::string& str) {
  int first = str.find_first_not_of(" \t");

  if (first != std::string::npos) {
      int last = str.find_last_not_of(" \t");

      return str.substr(first, last - first + 1);
  } else {
      return "";
  }
}

bool Configuration::getDebug() const {
  return this->debug;
}

bool Configuration::getExitIfNoFrame() const {
  return this->exitIfNoFrame;
}

bool Configuration::getUseMeanshiftGrouping() const {
  return this->useMeanshiftGrouping;
}

int Configuration::getCamNumber() const {
  return this->camNumber;
}

int Configuration::getFPS() const {
  return this->fps;
}

int Configuration::getJPGQuality() const {
  return this->jpgQuality;
}

int Configuration::getVideoWidth() const {
  return this->videoWidth;
}

int Configuration::getVideoHeight() const {
  return this->videoHeight;
}

useconds_t Configuration::getSleepTime() const {
  return this->sleepTime;
}

double Configuration::getRatioResizeWidth() const {
  return this->ratioResizeWidth;
}

double Configuration::getRatioResizeHeight() const {
  return this->ratioResizeHeight;
}

double Configuration::getHitThreshold() const {
  return this->hitThreshold;
}

double Configuration::getScale() const {
  return this->scale;
}

double Configuration::getFinalThreshold() const {
  return this->finalThreshold;
}

std::string Configuration::getCamStream() const {
  return this->camStream;
}

std::string Configuration::getImageFile() const {
  return this->imageFile;
}

std::string Configuration::getLogFile() const {
  return this->logFile;
}

std::string Configuration::getRecordPath() const {
  return this->recordPath;
}

std::string Configuration::getDetection() const {
  return this->detection;
}

std::string Configuration::getConfFile() const {
  return this->confFile;
}

cv::Size Configuration::getWinStride() const {
  return this->winStride;
}

cv::Size Configuration::getPadding() const {
  return this->padding;
}
