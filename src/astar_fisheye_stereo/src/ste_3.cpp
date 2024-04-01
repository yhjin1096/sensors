#include "astar_fisheye_stereo/ste_3.hpp"

int main(int argc, char **argv)
{
    ste_3 astar;
    std::string file_name = argc == 2 ? argv[1] : "/home/cona/yhj/sensor/src/astar_fisheye_stereo/ste-3_stereo.yml";
    astar.LoadParameters(file_name);
    astar.InitRectifyMap();

    cv::Mat raw_img, raw_imgl, raw_imgr;
    cv::Mat rect_imgl, rect_imgr;
    cv::VideoCapture vcapture;

    vcapture.open(0);

    if (!vcapture.isOpened())
    {
        std::cout << "Camera doesn't work" << std::endl;
        exit(-1);
    }

    while (1)
    {
        vcapture >> raw_img;
        
        if (raw_img.total() == 0)
        {
            std::cout << "Image capture error" << std::endl;
            exit(-1);
        }
        
        if (astar.cam_model == "stereo")
        {
            raw_img(cv::Rect(0, 0, astar.img_width, astar.cap_rows)).copyTo(raw_imgl);
            raw_img(cv::Rect(astar.img_width, 0, astar.img_width, astar.cap_rows)).copyTo(raw_imgr);

            cv::remap(raw_imgl, rect_imgl, astar.smap[0][0], astar.smap[0][1], 1, 0);
            cv::remap(raw_imgr, rect_imgr, astar.smap[1][0], astar.smap[1][1], 1, 0);
        }
        
        cv::imshow("rect_imgl", rect_imgl);
        cv::imshow("rect_imgr", rect_imgr);
        char key = cv::waitKey(1);
        if(key == 27)
            break;
    }

    return 0;
}