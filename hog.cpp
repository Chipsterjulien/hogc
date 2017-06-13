#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <map>
#include <fstream> // Use to open and read files
#include <unistd.h>
#include <sstream>

using namespace std;
using namespace cv;

class Configuration {
public:
  void Clear();
  bool Load(const string& file);
  bool Contains(const string& key) const;

  // get value associated with given key
  bool Get(const string& key, string& value) const;
  bool Get(const string& key, int&    value) const;
  bool Get(const string& key, long&   value) const;
  bool Get(const string& key, double& value) const;
  bool Get(const string& key, bool&   value) const;

private:
  map<string, string> data;

  static string Trim(const string& str);
};

void Configuration::Clear() {
  data.clear();
}

bool Configuration::Load(const string& file) {
  ifstream inFile(file.c_str());

  if (!inFile.good()) {
      cout << "Cannot read configuration file " << file << endl;
      return false;
  }

  while (inFile.good() && ! inFile.eof()) {
    string line;
    getline(inFile, line);

    // filter out comments
    if (!line.empty()) {
      int pos = line.find('#');

      if (pos != string::npos) {
          line = line.substr(0, pos);
      }
    }

    // split line into key and value
    if (!line.empty()) {
      int pos = line.find('=');

      if (pos != string::npos) {
        string key     = Trim(line.substr(0, pos));
        string value   = Trim(line.substr(pos + 1));

        if (!key.empty() && !value.empty()) {
            data[key] = value;
        }
      }
    }
  }

  return true;
}

bool Configuration::Contains(const string& key) const {
  return data.find(key) != data.end();
}

bool Configuration::Get(const string& key, string& value) const {
  map<string,string>::const_iterator iter = data.find(key);

  if (iter != data.end()) {
      value = iter->second;
      return true;
  } else {
      return false;
  }
}

bool Configuration::Get(const string& key, int& value) const {
  string str;

  if (Get(key, str)) {
      value = atoi(str.c_str());
      return true;
  } else {
      return false;
  }
}

bool Configuration::Get(const string& key, long& value) const {
  string str;

  if (Get(key, str)) {
      value = atol(str.c_str());
      return true;
  } else {
      return false;
  }
}

bool Configuration::Get(const string& key, double& value) const {
  string str;

  if (Get(key, str)) {
      value = atof(str.c_str());
      return true;
  } else {
      return false;
  }
}

bool Configuration::Get(const string& key, bool& value) const {
  string str;

  if (Get(key, str)) {
      value = (str == "true");
      return true;
  } else {
      return false;
  }
}

string Configuration::Trim(const string& str) {
  int first = str.find_first_not_of(" \t");

  if (first != string::npos) {
      int last = str.find_last_not_of(" \t");

      return str.substr(first, last - first + 1);
  } else {
      return "";
  }
}

void getConfig(Configuration config, bool* debug, int* camNumber, string* camStream, double* ratioResizeWidth, double* ratioResizeHeight, int* videoWidth, int* videoHeight, int* fps, int* sleepTime, string* recordPath, int* jpgQuality) {
  config.Get("camNumber", *camNumber);
  config.Get("camStream", *camStream);
  config.Get("videoWidth", *videoWidth);
  config.Get("videoHeight", *videoHeight);
  config.Get("fps", *fps);
  config.Get("sleepTime", *sleepTime);
  config.Get("ratioResizeWidth", *ratioResizeWidth);
  config.Get("ratioResizeHeight", *ratioResizeHeight);
  config.Get("recordPath", *recordPath);
  config.Get("jpgQuality", *jpgQuality);
  config.Get("debug", *debug);
}

string getFilename() {
  time_t current_time;
  struct tm * time_info;
  char timeString[20];
  string returnTimeStr;

  time(&current_time);
  time_info = localtime(&current_time);

  strftime(timeString, 20, "%Y-%m-%d_%H-%M-%S", time_info);

  returnTimeStr = timeString;

  return returnTimeStr;
}

void searchHuman(bool* debug, int* camNumber, string* camStream, double* ratioResizeWidth, double* ratioResizeHeight, int* videoWidth, int* videoHeight, int* fps, int* sleepTime, string* recordPath, int* jpgQuality) {
  VideoCapture cap;
  useconds_t Sleep = (*sleepTime) * 1000;

  if ((*camStream).size() != 0) {
    cap.open(*camStream);
  } else {
    cap.open(*camNumber);
  }

  if (!cap.isOpened()) {
    exit(EXIT_FAILURE);
  }

  Mat frame;
  HOGDescriptor hog;
  hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
  vector < Rect > found, found_filtered;

  if (*videoWidth != 0 && *videoHeight != 0) {
    cap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));
    cap.set(CV_CAP_PROP_FRAME_WIDTH, *videoWidth);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, *videoHeight);
  }

  if (*fps != 0 && (*camStream).size() == 0) {
    // Set the capture FPS
    cap.set(CV_CAP_PROP_FPS, *fps);
  }

  vector<int> compression_params;
  compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
  compression_params.push_back(75);

  for (;;) {
    // read img
    cap >> frame;

    // Test if frame is not empty
    if (frame.empty()) {
      cerr << "No image to process !" << endl;
      break;
    }

    // resize frame;
    resize(frame, frame, Size(), *ratioResizeWidth, *ratioResizeHeight);
    // Detect human
    hog.detectMultiScale(frame, found);
    // Draw rectangle around human
    size_t i, j;
    for (i = 0; i < found.size(); i++) {
      Rect r = found[i];
      for (j = 0; j < found.size(); j++)
        if (j != i && (r & found[j]) == r)
          break;
      if (j == found.size())
        found_filtered.push_back(r);
    }

    for (i = 0; i < found_filtered.size(); i++) {
      Rect r = found_filtered[i];
      r.x += cvRound(r.width * 0.1);
      r.width = cvRound(r.width * 0.8);
      r.y += cvRound(r.height * 0.07);
      r.height = cvRound(r.height * 0.8);
      rectangle(frame, r.tl(), r.br(), Scalar(0, 255, 0), 3);
    }

    if (found.size() != 0) {
      string filename;
      filename.append(*recordPath).append(getFilename()).append(".jpg");
      imwrite(filename, frame, compression_params);
    }

    if (debug && found.size() != 0) {
      cerr << "Found" << endl;
    }

    if (debug) {
      imshow("Output", frame);
      if (waitKey(10) == 27) {
        break;
      }
    }

    found.clear();
    found_filtered.clear();

    if (sleepTime != 0) {
      usleep(Sleep);
    }
  }

  destroyAllWindows();
}


int main(int argc, char **argv) {
  Configuration config;

  bool debug;
  int camNumber, videoWidth, videoHeight, fps, sleepTime, jpgQuality;
  double ratioResizeWidth, ratioResizeHeight;
  string camStream, recordPath;


  config.Load("cfg/hog.ini");

  getConfig(config, &debug, &camNumber, &camStream, &ratioResizeWidth, &ratioResizeHeight, &videoWidth, &videoHeight, &fps, &sleepTime, &recordPath, &jpgQuality);
  searchHuman(&debug, &camNumber, &camStream, &ratioResizeWidth, &ratioResizeHeight, &videoWidth, &videoHeight, &fps, &sleepTime, &recordPath, &jpgQuality);

  if (debug)
    cerr << "End of program in debug mode" << endl;

  return EXIT_SUCCESS;
}
