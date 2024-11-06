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

#include <sl/Camera.hpp>

#include "nvidia_zed/zed2i.hpp"

#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/Image.h>
#include <image_transport/image_transport.h>

// zed.setCameraSetting(sl::VIDEO_SETTINGS_LIST, )
// not use, just looking for option
enum class VIDEO_SETTINGS_LIST {
        BRIGHTNESS, /**< Brightness control \n Affected value should be between 0 and 8. \note Not available for ZED X/X Mini cameras.*/
        CONTRAST, /**< Contrast control \n Affected value should be between 0 and 8. \note Not available for ZED X/X Mini cameras.*/
        HUE, /**< Hue control \n Affected value should be between 0 and 11. \note Not available for ZED X/X Mini cameras.*/
        SATURATION, /**< Saturation control \n Affected value should be between 0 and 8.*/
        SHARPNESS, /**< Digital sharpening control \n Affected value should be between 0 and 8.*/
        GAMMA, /**< ISP gamma control \n Affected value should be between 1 and 9.*/
        GAIN, /**< Gain control \n Affected value should be between 0 and 100 for manual control. \note If EXPOSURE is set to -1 (automatic mode), then GAIN will be automatic as well.*/
        EXPOSURE, /**< Exposure control \n Affected value should be between 0 and 100 for manual control.\n The exposition is mapped linearly in a percentage of the following max values.\n Special case for <code>EXPOSURE = 0</code> that corresponds to 0.17072ms.\n The conversion to milliseconds depends on the framerate: <ul><li>15fps & <code>EXPOSURE = 100</code> -> 19.97ms</li><li>30fps & <code>EXPOSURE = 100</code> -> 19.97ms</li><li>60fps & <code>EXPOSURE = 100</code> -> 10.84072ms</li><li>100fps & <code>EXPOSURE = 100</code> -> 10.106624ms</li></ul>*/
        AEC_AGC, /**< Defines if the GAIN and EXPOSURE are in automatic mode or not.\n Setting GAIN or EXPOSURE values will automatically set this value to 0.*/
        AEC_AGC_ROI, /**< Defines the region of interest for automatic exposure/gain computation.\n To be used with overloaded \ref Camera.setCameraSettings(VIDEO_SETTINGS,Rect,sl::SIDE,bool) "setCameraSettings()" / \ref Camera.getCameraSettings(VIDEO_SETTINGS,Rect&,sl::SIDE) "getCameraSettings()" methods.*/
        WHITEBALANCE_TEMPERATURE, /**< Color temperature control \n Affected value should be between 2800 and 6500 with a step of 100. \note Setting a value will automatically set WHITEBALANCE_AUTO to 0.*/
        WHITEBALANCE_AUTO, /**< Defines if the white balance is in automatic mode or not.*/
        LED_STATUS, /**< Status of the front LED of the camera.\n Set to 0 to disable the light, 1 to enable the light.\n Default value is on. \note Requires camera firmware 1523 at least.*/
        EXPOSURE_TIME, /**< Real exposure time control in microseconds. \note Only available for ZED X/X Mini cameras.\note Replace EXPOSURE setting.*/
        ANALOG_GAIN, /**< Real analog gain (sensor) control in mDB.\n The range is defined by Jetson DTS and by default [1000-16000]. \note Only available for ZED X/X Mini cameras.\note Replace GAIN settings.*/
        DIGITAL_GAIN, /**< Real digital gain (ISP) as a factor.\n The range is defined by Jetson DTS and by default [1-256]. \note Only available for ZED X/X Mini cameras.\note Replace GAIN settings.*/
        AUTO_EXPOSURE_TIME_RANGE, /**< Range of exposure auto control in microseconds.\n Used with \ref Camera.setCameraSettings(VIDEO_SETTINGS,int,int) "setCameraSettings()".\n Min/max range between max range defined in DTS.\n By default: [28000 - <fps_time> or 19000] us. \note Only available for ZED X/X Mini cameras.*/
        AUTO_ANALOG_GAIN_RANGE, /**< Range of sensor gain in automatic control.\n Used with \ref Camera.setCameraSettings(VIDEO_SETTINGS,int,int) "setCameraSettings()".\n Min/max range between max range defined in DTS.\n By default: [1000 - 16000] mdB. \note Only available for ZED X/X Mini cameras.*/
        AUTO_DIGITAL_GAIN_RANGE, /**< Range of digital ISP gain in automatic control.\n Used with \ref Camera.setCameraSettings(VIDEO_SETTINGS,int,int) "setCameraSettings()".\n Min/max range between max range defined in DTS.\n By default: [1 - 256]. \note Only available for ZED X/X Mini cameras.*/
        EXPOSURE_COMPENSATION, /**< Exposure-target compensation made after auto exposure.\n Reduces the overall illumination target by factor of F-stops.\n Affected value should be between 0 and 100 (mapped between [-2.0,2.0]).\n Default value is 50, i.e. no compensation applied. \note Only available for ZED X/X Mini cameras.*/
        DENOISING, /**< Level of denoising applied on both left and right images.\n Affected value should be between 0 and 100.\n Default value is 50. \note Only available for ZED X/X Mini cameras.*/
        ///@cond SHOWHIDDEN
        LAST
        ///@endcond
    };

int main(int argc, char** argv)
{
    ros::init(argc, argv, "zed2i_sdk_pub");
    ros::NodeHandle nh;
    image_transport::ImageTransport it(nh);
    image_transport::Publisher pub = it.advertise("camera/image_raw", 1);
    cv::Mat frame, left_raw, left_rect, right_raw, right_rect;
    
    sl::Camera zed;
    sl::Mat zed_image;
    
    sl::InitParameters init_parameters;
    init_parameters.sdk_verbose = true;
    init_parameters.camera_resolution= sl::RESOLUTION::HD720;
    init_parameters.camera_fps= 60;
    init_parameters.depth_mode = sl::DEPTH_MODE::NONE; // no depth computation required here
    init_parameters.async_grab_camera_recovery = true;
    init_parameters.enable_image_validity_check = true;

    auto returned_state = zed.open(init_parameters);
    if (returned_state != sl::ERROR_CODE::SUCCESS) {
        std::cout << "Camera Open" << returned_state << "Exit program." << std::endl;;
        return EXIT_FAILURE;
    }


    // zed.setCameraSetting
    std::cout << zed.getInitParameters().camera_fps << std::endl;
    

    returned_state = zed.grab();
    zed.retrieveImage(zed_image, sl::VIEW::SIDE_BY_SIDE);
///////////////////////////////////////////////////////////////////////////////////
    std::string calibration_file = "/home/cona/yhj/sensors/src/nvidia_zed/SN33204635.conf";

    cv::Mat map_left_x, map_left_y;
    cv::Mat map_right_x, map_right_y;
    cv::Mat cameraMatrix_left, cameraMatrix_right;
    initCalibration(calibration_file, cv::Size(zed_image.getWidth()/2, zed_image.getHeight()), map_left_x, map_left_y, map_right_x, map_right_y, cameraMatrix_left, cameraMatrix_right);

    std::cout << " Projection Matrix L: \n" << cameraMatrix_left << std::endl << std::endl;
    std::cout << " Projection Matrix R: \n" << cameraMatrix_right << std::endl << std::endl;

    char key = 'r';

    sensor_msgs::ImagePtr msg;

    while (ros::ok() && key != 'q')
    {
        returned_state = zed.grab();
        if (returned_state != sl::ERROR_CODE::SUCCESS)
            std::cout << "returned_state " << returned_state << std::endl;
        zed.retrieveImage(zed_image, sl::VIEW::SIDE_BY_SIDE);
        frame = cv::Mat((int) zed_image.getHeight(), (int) zed_image.getWidth(), CV_8UC4, zed_image.getPtr<sl::uchar1>(sl::MEM::CPU));

        left_raw = frame(cv::Rect(0, 0, frame.cols / 2, frame.rows));
        right_raw = frame(cv::Rect(frame.cols / 2, 0, frame.cols / 2, frame.rows));
        // Display images
        cv::imshow("left RAW", left_raw);

        // cv::remap(left_raw, left_rect, map_left_x, map_left_y, cv::INTER_LINEAR);
        // cv::remap(right_raw, right_rect, map_right_x, map_right_y, cv::INTER_LINEAR);

        // cv::imshow("right RECT", right_rect);
        // cv::imshow("left RECT", left_rect);

        msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", left_raw).toImageMsg();
        pub.publish(msg);

        ros::spinOnce();

        key = cv::waitKey(1);
    }
    return 0;
}