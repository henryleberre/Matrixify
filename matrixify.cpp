#include <array>
#include <thread>
#include <string>
#include <vector>
#include <memory>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <filesystem>

#include "SimplexNoise/src/SimplexNoise.h"
#include <opencv2/opencv.hpp>

int main(int argc, char** argv) {
    std::srand((unsigned int)std::time(NULL));

    std::vector<std::string> filenames;
    for (const auto & entry : std::filesystem::directory_iterator("temp/input_frames/"))
        filenames.push_back(entry.path());
    
    std::sort(filenames.begin(), filenames.end());

    if (filenames.size() == 0) {
        std::cout << "No Input Frames Found... Aborting.\n";
        return EXIT_FAILURE;
    }

    const cv::Mat testFrame = cv::imread(filenames[0]);

    const std::uint32_t WIDTH        = testFrame.cols;
    const std::uint32_t HEIGHT       = testFrame.rows;
    const std::uint32_t FPS          = 60;
    const std::uint32_t FRAME_COUNT  = filenames.size();
    const float         FRAME_DELTA  = 1.f / FPS;
    const float         MATRIX_SCALE = 1.f * (std::max(WIDTH, HEIGHT) / 1920);
    const std::uint32_t CHAR_SIZE_X  = 15 * MATRIX_SCALE;
    const std::uint32_t CHAR_SIZE_Y  = 15 * MATRIX_SCALE;
    const std::uint32_t GRID_CHAR_COUNT_X = WIDTH  / CHAR_SIZE_X + 1; // CHAR_SIZE_X doesn't always divide WIDTH
    const std::uint32_t GRID_CHAR_COUNT_Y = HEIGHT / CHAR_SIZE_Y;

    std::vector<cv::Mat> input_character_images;
    for (const auto & entry : std::filesystem::directory_iterator("temp/input_characters/")) {
        cv::Mat m = cv::imread(entry.path(), cv::IMREAD_UNCHANGED);
        cv::resize(m, m, cv::Size(CHAR_SIZE_X, CHAR_SIZE_Y));
        input_character_images.push_back(m);
    }

    struct Streamer {
        float gx, gy;
        float gdy;
        std::vector<std::size_t> textIDs;
    }; // struct Streamer

    std::vector<Streamer> streamers(GRID_CHAR_COUNT_X);
    for (uint32_t sIdx = 0; sIdx < streamers.size(); ++sIdx) {
        Streamer& s = streamers[sIdx];

        // don't ask any questions about these constants. FBI open up.

        s.textIDs.resize(GRID_CHAR_COUNT_Y*0.75 + std::rand()%(int)(GRID_CHAR_COUNT_Y*0.25));
        for (std::size_t& c : s.textIDs)
            c = std::rand()%input_character_images.size();

        s.gx  = sIdx % GRID_CHAR_COUNT_X;
        s.gy  = -(float)s.textIDs.size();
        s.gdy = (0.4 * (std::rand() / (float)RAND_MAX) + 0.3f) * 100 * FRAME_DELTA;
    }

    cv::Mat blackOverlay        = cv::Mat(HEIGHT, WIDTH, CV_8UC1, cv::Scalar(0));
    cv::Mat previousTextOverlay = cv::Mat(HEIGHT, WIDTH, CV_8UC1, cv::Scalar(0));

    for (std::uint32_t frameIdx = 0; frameIdx < FRAME_COUNT; ++frameIdx) {
        std::cout << "\rFrame " << (frameIdx+1) << std::flush;

        cv::Mat originalFrame          = cv::imread(filenames[frameIdx]);
        cv::Mat originalFrameGrayscale = cv::imread(filenames[frameIdx], cv::IMREAD_GRAYSCALE);
        cv::Mat outputFrame            = originalFrame.clone();
 
        cv::Mat textOverlay   = cv::Mat(HEIGHT, WIDTH, CV_8UC1, cv::Scalar(0));

        for (Streamer& s : streamers) {
            for (std::uint32_t cIdx = 0; cIdx < s.textIDs.size(); ++cIdx) {
                int cImgX = static_cast<int>(s.gx * CHAR_SIZE_X);
                int cImgY = static_cast<int>(s.gy + cIdx) * CHAR_SIZE_Y;

                if (cImgY + (int)CHAR_SIZE_Y < 0)
                    continue;

                if (cImgY <= CHAR_SIZE_Y)
                    cv::rectangle(blackOverlay, cv::Rect(cImgX, 0, CHAR_SIZE_X, CHAR_SIZE_Y), cv::Scalar(255), -1);

                if (cImgY > 0)
                    cImgY = cImgY % HEIGHT;

                const std::size_t ids = s.textIDs[((int)s.gy + cIdx) % s.textIDs.size()];

                cv::rectangle(blackOverlay, cv::Rect(cImgX, cImgY, CHAR_SIZE_X, CHAR_SIZE_Y), cv::Scalar(255), -1);

                for (int cy = 0; cy < std::clamp(cImgY + (int)CHAR_SIZE_Y, 0, (int)HEIGHT-1) - std::abs(cImgY); ++cy) {
                    for (int cx = 0; cx < std::clamp(cImgX + (int)CHAR_SIZE_X, 0, (int)WIDTH-1) - std::abs(cImgX); ++cx) {
                        textOverlay.at<uchar>(cv::Point(cImgX + cx, cImgY + cy)) = input_character_images[ids].at<uchar>(cv::Point(cx, cy));
                    }
                }
            }
      
            s.gy += s.gdy;

            if (s.gy >= (float)GRID_CHAR_COUNT_Y)
                s.gy -= static_cast<float>(GRID_CHAR_COUNT_Y);
        }

        textOverlay *= 0.1f;
        textOverlay += 0.9f * previousTextOverlay;
        previousTextOverlay = textOverlay.clone();

        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; x++) {
                if (blackOverlay.at<uchar>(cv::Point(x, y)) > 0) {
                    outputFrame.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b(0, 0, 0);
                }

                if (textOverlay.at<uchar>(cv::Point(x, y)) > 0) {
                    const float fTextIntensity = textOverlay.at<uchar>(cv::Point(x, y))/255.f;

                    outputFrame.at<cv::Vec3b>(cv::Point(x, y)) = fTextIntensity * cv::Vec3b(40, 255, 40);
                }

                const float fMatrixIntensity = originalFrameGrayscale.at<uchar>(cv::Point(x, y))/255.f;

                outputFrame.at<cv::Vec3b>(cv::Point(x, y)) *= fMatrixIntensity;
            }
        }

        cv::imwrite("temp/output_frames/frame_"+std::to_string(frameIdx+1)+".png", outputFrame);
    }

    const std::string command = "ffmpeg -y -framerate "+std::to_string(FPS)+" -i \"temp/output_frames/frame_%d.png\" -vcodec libx264 \"output/output.mkv\" 2> /dev/null";
    std::system(command.c_str());
}