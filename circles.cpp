#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>

// Define structure to hold custom circle details
struct CircleDetails {
    cv::Point2f center;
    float radius;
    double circularity;
};

// Comparison function to sort circles from largest to smallest by radius
bool compareRadius(const CircleDetails& a, const CircleDetails& b) {
    return a.radius > b.radius;
}

int main() {
    // 1. Load the input image
    cv::Mat img = cv::imread("biggercircles.jpg");
    if (img.empty()) {
        std::cout << "Could not open or find the image!" << std::endl;
        return -1;
    }

    // 2. Preprocessing: Convert to grayscale and blur
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(9, 9), 2, 2);

    // 3. Thresholding to create a binary mask
    cv::Mat thresh;
    cv::threshold(gray, thresh, 0, 255, cv::THRESH_BINARY_INV + cv::THRESH_OTSU);

    // 4. Find contours
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(thresh, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<CircleDetails> allCircles;

    // 5. Loop through contours to evaluate size and circularity
    for (size_t i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);
        double perimeter = cv::arcLength(contours[i], true);

        if (perimeter == 0) continue;

        // Approximate structural bounding circle
        cv::Point2f center;
        float radius;
        cv::minEnclosingCircle(contours[i], center, radius);

        // Filter out extreme noise or tiny fragments upfront
        if (area < 50) continue;

        double circularity = (4 * CV_PI * area) / (perimeter * perimeter);

        // If it looks circular enough, save its data
        if (circularity > 0.7 && circularity < 1.3) {
            CircleDetails circle;
            circle.center = center;
            circle.radius = radius;
            circle.circularity = circularity;
            allCircles.push_back(circle);
        }
    }

    // 6. Sort detected shapes from largest to smallest
    std::sort(allCircles.begin(), allCircles.end(), compareRadius);

    std::cout << "Detected " << allCircles.size() << " circular object(s)." << std::endl;

    int bigCircleCount = 0;

    // 7. Iterate, filter, draw, and print data for large circles ONLY
    for (size_t i = 0; i < allCircles.size(); i++) {
        
        // 🛑 FILTER LOGIC: Change '50.0' to a higher value (like 80.0 or 100.0) if small circles still show up
        if (allCircles[i].radius > 50.0) {
            bigCircleCount++;

            cv::Point2f center = allCircles[i].center;
            float radius = allCircles[i].radius;
            int diameter = (int)(radius * 2);

            // Draw bounding overlay on image
            cv::circle(img, center, (int)radius, cv::Scalar(0, 255, 0), 2);
            cv::circle(img, center, 2, cv::Scalar(0, 0, 255), -1);

            // Generate layout labels next to target items
            std::string label = "C" + std::to_string(bigCircleCount) + ": D=" + std::to_string(diameter);
            cv::putText(img, label, cv::Point(center.x - 40, center.y - radius - 10), 
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1.5);

            // Print structural coordinate data to terminal windows
            std::cout << "Circle " << bigCircleCount << " -> "
                      << "Center: (" << (int)center.x << ", " << (int)center.y << ") | "
                      << "Radius: " << radius << " | "
                      << "Diameter: " << diameter << " | "
                      << "Circularity: " << allCircles[i].circularity << std::endl;
        }
    }

    std::cout << "Filtered out small items. Displaying " << bigCircleCount << " big circle(s)." << std::endl;

    // 8. Render output layout graphics
    cv::imshow("Detected Big Circles Only", img);
    cv::imwrite("detected_output.jpg", img);
    cv::waitKey(0);

    return 0;
}

