#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    // 1. Load the image
    cv::Mat img = cv::imread("img1.jpg");
    if (img.empty()) {
        std::cerr << "Error: Could not open or find 'img1.jpg'!" << std::endl;
        return -1;
    }

    // 2. Convert from BGR to HSV color space
    cv::Mat hsv;
    cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);

    // 3. Define an aggressive, wide Red range to capture the bright sphere perfectly
    cv::Mat mask1, mask2, final_mask;
    // Lower red spectrum (Hue 0-15, completely widening Saturation and Value bounds)
    cv::inRange(hsv, cv::Scalar(0, 60, 50), cv::Scalar(15, 255, 255), mask1);
    // Upper red spectrum (Hue 165-180, completely widening Saturation and Value bounds)
    cv::inRange(hsv, cv::Scalar(165, 60, 50), cv::Scalar(180, 255, 255), mask2);
    final_mask = mask1 | mask2;

    // Clean up minor noise using morphological closing and opening
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
    cv::morphologyEx(final_mask, final_mask, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(final_mask, final_mask, cv::MORPH_OPEN, kernel);

    // 4. Trace outlines (contours)
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(final_mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (!contours.empty()) {
        // Find the absolute largest red contour by area to target the main sphere
        auto largest_contour = *std::max_element(contours.begin(), contours.end(), 
            [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
                return cv::contourArea(a) < cv::contourArea(b);
            }
        );

        // Make sure it's a huge object (greater than 50,000 pixels) to avoid noise boxes
        if (cv::contourArea(largest_contour) > 50000) {
            // 5. Generate the bounding rectangle around the red sphere
            cv::Rect bounding_box = cv::boundingRect(largest_contour);

            // 6. Log coordinates to your terminal
            std::cout << "========================================" << std::endl;
            std::cout << "SUCCESS: Red Sphere Isolated!" << std::endl;
            std::cout << "Top-Left Coordinate: (" << bounding_box.x << ", " << bounding_box.y << ")" << std::endl;
            std::cout << "Dimensions: " << bounding_box.width << " x " << bounding_box.height << " pixels" << std::endl;
            std::cout << "Center Pin: (" << bounding_box.x + bounding_box.width / 2 
                      << ", " << bounding_box.y + bounding_box.height / 2 << ")" << std::endl;
            std::cout << "========================================" << std::endl;

            // 7. Draw a thick green bounding box perfectly framing the red sphere
            cv::rectangle(img, bounding_box, cv::Scalar(0, 255, 0), 15);

            // Place text identifier right over the box
            cv::putText(img, "Red Sphere", cv::Point(bounding_box.x, bounding_box.y - 40),
                        cv::FONT_HERSHEY_SIMPLEX, 3.5, cv::Scalar(0, 255, 0), 8);
        } else {
            std::cout << "Detected red elements are too small to be the primary object." << std::endl;
        }
    } else {
        std::cout << "No red elements were detected." << std::endl;
    }

    // 8. Display with Auto-Resizing Window Configuration
    cv::namedWindow("Detected Object (Result)", cv::WINDOW_NORMAL);
    cv::resizeWindow("Detected Object (Result)", 600, 850); 
    
    cv::imshow("Detected Object (Result)", img);
    cv::imwrite("detected_output.jpg", img); 

    cv::waitKey(0);
    return 0;
}

        
  
