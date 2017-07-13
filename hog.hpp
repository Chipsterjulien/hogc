#ifndef hog_h_included
#define hog_h_included

#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <chrono>
#include "loadConfig.hpp"
#include "log.hpp"

void help();
void detectMultiScaleFunc(Configuration& config, Logging& log, cv::HOGDescriptor& hog, cv::Mat& frame, std::vector< cv::Rect >& found);
void drawRectangle(cv::Mat& frame, std::vector< cv::Rect >& found, std::vector< cv::Rect >& found_filtered);
void infiniteLoop(Configuration& config, Logging& log, cv::VideoCapture& cap, cv::Mat& frame, cv::HOGDescriptor& hog);
void initCapture(Configuration& config, cv::VideoCapture& cap);
void recordPicture(Configuration& config, cv::Mat& frame, std::vector<int>& compression_params, std::vector< cv::Rect >& found);
void searchHuman(Configuration& config, Logging& log);
std::string getFilename();
#endif
