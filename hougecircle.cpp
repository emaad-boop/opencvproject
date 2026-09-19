#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // 1. Load in color (need color for final drawing) + grayscale copy
    cv::Mat color = cv::imread("circles.jpg", cv::IMREAD_COLOR);
    if (color.empty()) {
        std::cout << "Could not open or find the image!" << std::endl;
        return -1;
    }
    cv::Mat gray;
    cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);

    // 2. Blur slightly to reduce noise
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);

    // 3. Threshold: coins are bright, background is black
    cv::Mat thresh;
    cv::threshold(blurred, thresh, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // 4. Clean up small noise / close small gaps
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(thresh, thresh, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(thresh, thresh, cv::MORPH_CLOSE, kernel);

    // 5. Find contours (each coin = one blob)
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 6. Filter by area and draw the fitted circle for each valid contour
    cv::Mat display_img = color.clone();
    int count = 0;
    for (const auto& c : contours) {
        double area = cv::contourArea(c);
        if (area < 1000) continue;  // discard tiny noise blobs; tune threshold as needed

        cv::Point2f center;
        float radius;
        cv::minEnclosingCircle(c, center, radius);

        // Optional: filter out non-circular blobs (e.g. two merged coins)
        double circularity = area / (CV_PI * radius * radius);
        if (circularity < 0.6) continue; // reject weird/merged shapes

        cv::circle(display_img, center, (int)radius, cv::Scalar(0, 255, 0), 3, cv::LINE_AA);
        cv::circle(display_img, center, 3, cv::Scalar(0, 0, 255), -1, cv::LINE_AA);
        count++;
    }

    std::cout << "Coins detected: " << count << std::endl;

    cv::imshow("Detected Circles", display_img);
    cv::waitKey(0);
    return 0;
}
