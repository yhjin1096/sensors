#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

class ste_3
{
    public:
        int cap_cols, cap_rows, img_width;
        double fov;
        cv::Mat Translation, Kl, Kr, Dl, Dr, xil, xir, Rl, Rr, smap[2][2], Knew;
        std::string cam_model;

        void LoadParameters(std::string file_name) {
            cv::FileStorage fs(file_name, cv::FileStorage::READ);
            if (!fs.isOpened()) {
                std::cout << "Failed to open ini parameters" << std::endl;
                exit(-1);
            }

            cv::Size cap_size;
            fs["cam_model"] >> cam_model;
            fs["cap_size" ] >> cap_size;
            fs["Kl"       ] >> Kl;
            fs["Dl"       ] >> Dl;
            fs["xil"      ] >> xil;
            fs["fov"] >> fov;
            Rl = cv::Mat::eye(3, 3, CV_64F);
            if (cam_model == "stereo") {
                fs["Rl"       ] >> Rl;
                fs["Kr"       ] >> Kr;
                fs["Dr"       ] >> Dr;
                fs["xir"      ] >> xir;
                fs["Rr"       ] >> Rr;
                fs["T"        ] >> Translation;
            }
            fs.release();

            img_width = cap_size.width;
            cap_cols  = cap_size.width;
            cap_rows  = cap_size.height;

            if (cam_model == "stereo")
                img_width  = cap_size.width / 2;
            }

            inline double MatRowMul(cv::Mat m, double x, double y, double z, int r) {
            return m.at<double>(r,0) * x + m.at<double>(r,1) * y + m.at<double>(r,2) * z;
        }

        void InitUndistortRectifyMap(cv::Mat K, cv::Mat D, cv::Mat xi, cv::Mat R, 
                                    cv::Mat P, cv::Size size, 
                                    cv::Mat& map1, cv::Mat& map2) {
            map1 = cv::Mat(size, CV_32F);
            map2 = cv::Mat(size, CV_32F);

            double fx = K.at<double>(0,0);
            double fy = K.at<double>(1,1);
            double cx = K.at<double>(0,2);
            double cy = K.at<double>(1,2);
            double s  = K.at<double>(0,1);

            double xid = xi.at<double>(0,0);

            double k1 = D.at<double>(0,0);
            double k2 = D.at<double>(0,1);
            double p1 = D.at<double>(0,2);
            double p2 = D.at<double>(0,3);

            cv::Mat KRi = (P * R).inv();

            for (int r = 0; r < size.height; ++r) {
                for (int c = 0; c < size.width; ++c) {
                double xc = MatRowMul(KRi, c, r, 1., 0);
                double yc = MatRowMul(KRi, c, r, 1., 1);
                double zc = MatRowMul(KRi, c, r, 1., 2);    

                double rr = sqrt(xc * xc + yc * yc + zc * zc);
                double xs = xc / rr;
                double ys = yc / rr;
                double zs = zc / rr;

                double xu = xs / (zs + xid);
                double yu = ys / (zs + xid);

                double r2 = xu * xu + yu * yu;
                double r4 = r2 * r2;
                double xd = (1+k1*r2+k2*r4)*xu + 2*p1*xu*yu + p2*(r2+2*xu*xu);
                double yd = (1+k1*r2+k2*r4)*yu + 2*p2*xu*yu + p1*(r2+2*yu*yu);
                
                double u = fx * xd + s * yd + cx;
                double v = fy * yd + cy;

                map1.at<float>(r,c) = (float) u;
                map2.at<float>(r,c) = (float) v;
                }
            }
        }

        void InitRectifyMap() {
            double vfov_rad = fov * CV_PI / 180.;
            double focal = cap_rows / 2. / tan(vfov_rad / 2.);
            Knew = (cv::Mat_<double>(3, 3) << focal, 0., img_width  / 2. - 0.5,
                                                0., focal, cap_rows / 2. - 0.5,
                                                0., 0., 1.);

            cv::Size img_size(img_width, cap_rows);

            InitUndistortRectifyMap(Kl, Dl, xil, Rl, Knew, 
                                    img_size, smap[0][0], smap[0][1]);

            std::cout << "Width: "  << img_width  << "\t"
                        << "Height: " << cap_rows << "\t"
                        << "V.Fov: "  << fov   << "\n";
            std::cout << "K Matrix: \n" << Knew << std::endl;

            if (cam_model == "stereo") {
                InitUndistortRectifyMap(Kr, Dr, xir, Rr, Knew, 
                                        img_size, smap[1][0], smap[1][1]);
            }
            std::cout << std::endl;
        }
    private:
};