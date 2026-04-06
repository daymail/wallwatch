#include <opencv2/opencv.hpp>
#include <cstring>
#include "wsmeans.h"
#include "imgproc.h"
#include "celebi.h"
#include "wu.h"

namespace wallwatch{

std::vector<Argb> ExtractPixels(const std::string& path, int maxDimension){
    cv::Mat frame;
    bool isVideo = path.find(".mp4") != std::string::npos || path.find(".mkv") != std::string::npos || path.find(".gif") != std::string::npos;

    if(isVideo){
        cv::VideoCapture cap(path);
        if(cap.isOpened()){
            cap.set(cv::CAP_PROP_POS_AVI_RATIO, 0.5);
            if(cap.grab()){
                cap.retrieve(frame);
            }
            cap.release();}
        }else{
            frame = cv::imread(path, cv::IMREAD_REDUCED_COLOR_8);
    }

    if(frame.empty()) return {};

    if(frame.rows > maxDimension || frame.cols > maxDimension){
        double scale = static_cast<double>(maxDimension)/std::max(frame.rows, frame.cols);
        cv::resize(frame, frame, cv::Size(), scale, scale, cv::INTER_NEAREST);
    }

    cv::Mat bgra;
    cv::cvtColor(frame, bgra, cv::COLOR_BGR2BGRA);

    if(!bgra.isContinuous()){
        std::vector<Argb> pixels;
        pixels.reserve(bgra.total());
        for(int r = 0;r<bgra.rows;++r){
            const Argb* rowPtr = bgra.ptr<Argb>(r);
            pixels.insert(pixels.end(), rowPtr, rowPtr + bgra.cols);
        }
        return pixels;
    }

    std::vector<Argb> pixels(bgra.total());
    std::memcpy(pixels.data(), bgra.data, bgra.total() * sizeof(uint32_t));
    return pixels;
}

}   //namespace wallwatch

