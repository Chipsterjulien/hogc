#include "hog.hpp"

int main(int argc, char **argv) {
  Configuration config;

  config.GetParams(argc, argv);
  if (!config.Load()) {
    exit(EXIT_FAILURE);
  }

  Logging log(config);

  if (config.getDebug()) {
    std::cerr << "Configuration loaded" << std::endl;
    log.Write("Configuration loaded");
    config.ShowAllValues();
  }

  searchHuman(config, log);

  if (config.getDebug()) {
    std::cerr << "End of program in debug mode" << std::endl;
    log.Write("End of program in debug mode");
  }

  log.Close();

  return EXIT_SUCCESS;
}

void help() {
}

void drawRectangle(cv::Mat& frame, std::vector< cv::Rect >& found, std::vector< cv::Rect >& found_filtered) {
  size_t i, j;
  for (i = 0; i < found.size(); i++) {
    cv::Rect r = found[i];
    for (j = 0; j < found.size(); j++)
      if (j != i && (r & found[j]) == r)
        break;
    if (j == found.size())
      found_filtered.push_back(r);
  }

  for (i = 0; i < found_filtered.size(); i++) {
    cv::Rect r = found_filtered[i];
    r.x += cvRound(r.width * 0.1);
    r.width = cvRound(r.width * 0.8);
    r.y += cvRound(r.height * 0.07);
    r.height = cvRound(r.height * 0.8);
    rectangle(frame, r.tl(), r.br(), cv::Scalar(0, 255, 0), 3);
  }
}

void initCapture(Configuration& config, cv::VideoCapture& cap) {
  // if imageFile is not define
  if (config.getImageFile() == "") {
    if (config.getCamStream() != "") {
      cap.open(config.getCamStream());
    } else {
      cap.open(config.getCamNumber());
    }

    if (!cap.isOpened())
      exit(EXIT_FAILURE);

    if (config.getVideoWidth() != 0 && config.getVideoHeight() != 0) {
      cap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));
      cap.set(CV_CAP_PROP_FRAME_WIDTH, config.getVideoWidth());
      cap.set(CV_CAP_PROP_FRAME_HEIGHT, config.getVideoHeight());
    }
    if (config.getFPS() != 0 && config.getCamStream() != "") {
      // Set the capture FPS
      cap.set(CV_CAP_PROP_FPS, config.getFPS());
    }
  }
}

void infiniteLoop(Configuration& config, Logging& log, cv::VideoCapture& cap, cv::Mat& frame, cv::HOGDescriptor& hog) {
  int sleepTime = config.getSleepTime() * 1000;

  std::vector < cv::Rect > found, found_filtered;
  std::vector<int> compression_params;
  compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
  compression_params.push_back(config.getJPGQuality());
  std::chrono::high_resolution_clock::time_point t1, t2;

  for(;;) {
    if (config.getImageFile() != "") {
      // read img from file
      frame = cv::imread(config.getImageFile());
    } else {
      // read img from camera or stream
      cap >> frame;
    }
    // Check if frame is empty
    if (frame.empty()) {
      if (config.getExitIfNoFrame()) {
        std::cerr << "No image to process. Exiting … !" << std::endl;
        log.Write("No image to process. Exiting … !");
        break;
      } else {
        if (config.getDebug()) {
          std::cerr << "I'm sleeping 1s and I will check a new frame" << std::endl;
          log.Write("I'm sleeping 1s and I will check a new frame");
        }

        usleep(1000000); // Sleep 1s or 1 000 000 microseconds
        continue;
      }
    }

    t1 = std::chrono::high_resolution_clock::now();

    // Resize frame with ratio
    if (config.getRatioResizeWidth() != 1 || config.getRatioResizeHeight() != 1) {
      resize(frame, frame, cv::Size(), config.getRatioResizeWidth(), config.getRatioResizeHeight());
    }

    detectMultiScaleFunc(config, log, hog, frame, found);

    // Draw rectangle around human
    drawRectangle(frame, found, found_filtered);
    recordPicture(config, frame, compression_params, found);
    // free memory before sleep
    frame.release();
    // clear vectors
    found.clear();
    found_filtered.clear();

    t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();

    if (config.getDebug()) {
      std::stringstream durationInMs;
      durationInMs << duration / 1000.0 << "ms" << std::endl;

      std::cerr << durationInMs.str();
      log.Write(durationInMs.str());
    }

    if (sleepTime > 0) {
      if (config.getDeltaSleepTime()) {
        auto diff = sleepTime - duration;

        if (diff < 0) {
          std::stringstream overtime;
          overtime << "Diff is negative: " << diff << std::endl;

          std::cerr << overtime.str();
          log.Write(overtime.str());

          usleep(sleepTime - diff);
        } else {
          usleep(diff);
        }
      } else {
        usleep(sleepTime);
      }
    }
  }

  cv::destroyAllWindows();
}

void detectMultiScaleFunc(Configuration& config, Logging& log, cv::HOGDescriptor& hog, cv::Mat& frame, std::vector< cv::Rect >& found) {
  std::string detection = config.getDetection();

  if (detection == "default") {
    hog.detectMultiScale(frame, found);
  } else if (detection == "custom") {
    hog.detectMultiScale(frame, found, config.getHitThreshold(), config.getWinStride(), config.getPadding(), config.getScale(), config.getFinalThreshold(), config.getUseMeanshiftGrouping());
  } else {
    std::stringstream errorStr;

    errorStr << "\"" << config.getDetection() << "\": unknown detection mode in \"" <<config.getConfFile() << "\" file !" << std::endl;

    std::cerr << errorStr.str() << std::endl;
    log.Write(errorStr.str());
  }
}

void recordPicture(Configuration& config, cv::Mat& frame, std::vector<int>& compression_params, std::vector< cv::Rect >& found) {
  if (found.size() != 0) {
    std::stringstream filename;
    if (config.getRecordPath().at(config.getRecordPath().length()-1) != '/') {
      filename << config.getRecordPath() << "/" << getFilename() << ".jpg";
    } else {
      filename << config.getRecordPath() << getFilename() << ".jpg";
    }

    imwrite(filename.str(), frame, compression_params);
  }

  if (config.getDebug() && found.size() != 0) {
    std::cerr << "Found" << std::endl;
  }
}

void searchHuman(Configuration& config, Logging& log) {
  cv::VideoCapture cap;
  cv::Mat frame;
  cv::HOGDescriptor hog;
  hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

  initCapture(config, cap);
  infiniteLoop(config, log, cap, frame, hog);
}

std::string getFilename() {
  time_t current_time;
  struct tm * time_info;
  char timeString[20];
  std::string returnTimeStr;

  time(&current_time);
  time_info = localtime(&current_time);

  strftime(timeString, 20, "%Y-%m-%d_%H-%M-%S", time_info);

  returnTimeStr = timeString;

  return returnTimeStr;
}
