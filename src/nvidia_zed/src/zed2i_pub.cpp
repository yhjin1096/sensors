///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2018, STEREOLABS.
//
// All rights reserved.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

/*****************************************************************************************
 ** This sample demonstrates how to capture stereo images and calibration parameters    **
 ** from the ZED camera with OpenCV without using the ZED SDK.                          **
 *****************************************************************************************/

#include "nvidia_zed/zed2i.hpp"

#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/Image.h>
#include <image_transport/image_transport.h>

int main(int argc, char** argv)
{
    ros::init(argc, argv, "zed2i_pub");
    ros::NodeHandle nh;
    image_transport::ImageTransport it(nh);
    image_transport::Publisher pub = it.advertise("camera/image_raw", 1);

    cv::Size2i image_size = cv::Size2i(1280, 720);
    std::string calibration_file = "/home/cona/yhj/sensors/src/nvidia_zed/SN33204635.conf";

    cv::Mat map_left_x, map_left_y;
    cv::Mat map_right_x, map_right_y;
    cv::Mat cameraMatrix_left, cameraMatrix_right;
    initCalibration(calibration_file, image_size, map_left_x, map_left_y, map_right_x, map_right_y, cameraMatrix_left, cameraMatrix_right);

    std::cout << " Projection Matrix L: \n" << cameraMatrix_left << std::endl << std::endl;
    std::cout << " Projection Matrix R: \n" << cameraMatrix_right << std::endl << std::endl;

    char key = 'r';

    cv::VideoCapture cap(4);
    if (!cap.isOpened())
        return -1;
    cap.grab();
    // Set the video resolution (2*Width * Height)
    cap.set(cv::CAP_PROP_FRAME_WIDTH, image_size.width * 2);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, image_size.height);
    cap.grab();

    cv::Mat frame, left_raw, left_rect, right_raw, right_rect;

    sensor_msgs::ImagePtr msg;

    while (ros::ok() && key != 'q') {
        // Get a new frame from camera
        cap >> frame;
        // Extract left and right images from side-by-side
        left_raw = frame(cv::Rect(0, 0, frame.cols / 2, frame.rows));
        right_raw = frame(cv::Rect(frame.cols / 2, 0, frame.cols / 2, frame.rows));
        // Display images
        // cv::imshow("left RAW", left_raw);

        cv::remap(left_raw, left_rect, map_left_x, map_left_y, cv::INTER_LINEAR);
        // cv::remap(right_raw, right_rect, map_right_x, map_right_y, cv::INTER_LINEAR);

        // cv::imshow("right RECT", right_rect);
        cv::imshow("left RECT", left_rect);

        std_msgs::Header tmp_header;
        tmp_header.stamp = ros::Time::now();
        tmp_header.frame_id = "zed2i";
        msg = cv_bridge::CvImage(tmp_header, "bgr8", left_raw).toImageMsg();

        pub.publish(msg);

        ros::spinOnce();

        key = cv::waitKey(1);
    }
    return 0;
}
