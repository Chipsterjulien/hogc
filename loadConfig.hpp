#ifndef loadconfig_h_included
#define loadconfig_h_included

#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <chrono>

class Configuration {
  public:
    Configuration();

    void Clear();
    void GetConfig();
    void GetParams(int argc, char **argv);
    void ShowAllValues();

    bool Contains(const std::string& key) const;
    bool Load();

    // get value associated with given key
    bool Get(const std::string& key, std::string& value) const;
    bool Get(const std::string& key, int&    value) const;
    bool Get(const std::string& key, long&   value) const;
    bool Get(const std::string& key, double& value) const;
    bool Get(const std::string& key, bool&   value) const;

    bool getDebug() const;
    bool getExitIfNoFrame() const;

    int getCamNumber() const;
    int getFPS() const;
    int getJPGQuality() const;
    int getVideoWidth() const;
    int getVideoHeight() const;

    useconds_t getSleepTime() const;

    double getRatioResizeWidth() const;
    double getRatioResizeHeight() const;

    std::string getCamStream() const;
    std::string getImageFile() const;
    std::string getRecordPath() const;
    std::string getLogFile() const;

  private:
    bool debug;
    bool exitIfNoFrame;

    int camNumber;
    int fps;
    int jpgQuality;
    int videoWidth;
    int videoHeight;

    useconds_t sleepTime;

    double ratioResizeWidth;
    double ratioResizeHeight;

    std::string confFile;
    std::string camStream;
    std::string imageFile;
    std::string recordPath;
    std::string logFile;

    std::map<std::string, std::string> data;

    static std::string Trim(const std::string& str);
};

#endif
