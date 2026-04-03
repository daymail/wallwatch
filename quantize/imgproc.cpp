#include "imgproc.h"
#include <opencv2/opencv.hpp>

namespace wallwatch{

std::vector<Argb> ExtractPixels(const std::string& path){
    cv::Mat frame;
    cv::VideoCapture cap(path);
    if(cap.isOpened()){
        double frames = cap.get(cv::CAP_PROP_FRAME_COUNT);
        int middleFrame = static_cast<int>(frames/2);
        cap.set(cv::CAP_PROP_POS_FRAMES, middleFrame);
        cap.read(frame);
    }else{
        frame = cv::imread(path);
    }

    if(frame.empty()){
        return {};
    }

    cv::Mat rgba;
    cv::cvtColor(frame, rgba, cv::COLOR_BGR2RGBA);

    std::vector<Argb> pixels;
    pixels.reserve(rgba.total());

    const uint8_t* data = rgba.ptr();
    for(size_t i = 0;i<rgba.total();i++){
        size_t idx = i*4;
        Argb argb = (data[idx +3]<< 24) | (data[idx] << 16) | (data[idx + 1] << 8) | data[idx + 2];
        pixels.push_back(argb);
    }
    return pixels;
}

std::vector<Argb> ImageToPixels(const std::string&  path){
    cv::Mat img = cv::imread(path);
    if(path.empty()){
        return {};
    }

    cv::Mat rgba;
    cv::cvtColor(img, rgba, cv::COLOR_BGR2RGBA);

    std::vector<Argb> pixels;
    pixels.reserve(rgba.total());

    const uint8_t* data = rgba.ptr();
    for(size_t i = 0;i<rgba.total();i++){
        size_t idx = i*4;
        uint8_t r = data[idx];
        uint8_t g = data[idx+1];
        uint8_t b = data[idx+2];
        uint8_t a = data[idx+3];

        Argb argb = (a << 24) | (r << 16) | (g << 8) | b;
        pixels.push_back(argb);
    }
    return pixels;
}

std::vector<Argb> ImageToPixels(const std::string& path, int maxDimension){
    cv::Mat img = cv::imread(path);
    if(img.empty()){
        return {};
    }
    if(img.rows > maxDimension || img.cols > maxDimension){
        double scale = static_cast<double>(maxDimension) / std::max(img.rows, img.cols);
        cv::Mat resized;
        cv::resize(img, resized, cv::Size(), scale, scale);
        img = resized;
    }

    cv::Mat rgba;
    cv::cvtColor(img, rgba, cv::COLOR_BGR2RGBA);
    std::vector<Argb> pixels;
    pixels.reserve(rgba.total());

    const uint8_t* data = rgba.ptr();
    for(size_t i = 0;i<rgba.total();i++){
        size_t idx = i*4;
        Argb argb = (data[idx +3]<< 24) | (data[idx] << 16) | (data[idx + 1] << 8) | data[idx + 2];
        pixels.push_back(argb);
    }
    return pixels;
}
}   //namespace wallwatch
