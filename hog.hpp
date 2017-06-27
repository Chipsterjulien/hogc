#ifndef hog_h_included
#define hog_h_included

#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <map>
#include <fstream> // Use to open and read files
#include <unistd.h>
#include <sstream>
#include <chrono>

class Configuration {
public:
  void Clear();
  bool Load(const std::string& file);
  bool Contains(const std::string& key) const;

  // get value associated with given key
  bool Get(const std::string& key, std::string& value) const;
  bool Get(const std::string& key, int&    value) const;
  bool Get(const std::string& key, long&   value) const;
  bool Get(const std::string& key, double& value) const;
  bool Get(const std::string& key, bool&   value) const;

private:
  std::map<std::string, std::string> data;

  static std::string Trim(const std::string& str);
};

void getConfig(Configuration config, bool* debug, bool* exitNoFrame, int* camNumber, std::string* camStream, std::string* imageFile, double* ratioResizeWidth, double* ratioResizeHeight, int* videoWidth, int* videoHeight, int* fps, int* sleepTime, std::string* recordPath, int* jpgQuality);
std::string getFilename();
void drawRectangle(cv::Mat* frame, std::vector< cv::Rect >* found, std::vector< cv::Rect >* found_filtered);
void recordPicture(bool* debug, cv::Mat* frame, std::vector<int>* compression_params, std::string* recordPath, std::vector< cv::Rect >* found);
void searchHuman(bool* debug, bool* exitNoFrame, int* camNumber, std::string* camStream, std::string* imageFile, double* ratioResizeWidth, double* ratioResizeHeight, int* videoWidth, int* videoHeight, int* fps, int* sleepTime, std::string* recordPath, int* jpgQuality);

#endif
