#include <stdio.h>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <string>

using namespace cv;
using namespace std;
using namespace chrono;

double load_img();
Mat unfisheye(Mat input_frame);
void click_event (int event, int x, int y, int, void*);
void find_coords();

int main(int argc, char *argv[]) {
	char * args[argc];
	cout << argc << "\n";
	for (int i = 0; i < argc; ++i) {
        cout << argv[i] << "\n";
		args[i - 1] = argv[i];
    }
	cout << *args[0] << "\n";
	// find_coords();
	load_img();
	// double t_taken = 0;
	// int num_repeats = stoi(argv[1]);
	// setNumThreads(0);
	// for (int i = 0; i < num_repeats; i++) {
	// 	t_taken += load_img();
	// }
	// printf("Average time taken: %f\n", t_taken / num_repeats);
	return 0;
}

double load_img() {

	auto start_time = high_resolution_clock::now();
	Mat img;
	img = imread("image2.png");
	namedWindow("Display image", WINDOW_NORMAL);
	imshow("Display image", img);
	// waitKey(0);
	Mat unfisheyed = unfisheye(img);
	// namedWindow("Display image", WINDOW_AUTOSIZE);
	// imshow("Display image", unfisheyed);
	// waitKey(0);
	Mat grey;
	cvtColor(img, grey, COLOR_BGR2GRAY);
	imshow("Display image", grey);
	// waitKey(0);
	Mat chessboard_corners;
	findChessboardCorners(grey, Size(9, 6), chessboard_corners);
	cout << chessboard_corners << endl;
	// namedWindow("Display image", WINDOW_AUTOSIZE);
	// imshow("Display image", chessboard_corners);
	// waitKey(0);
	// undistort(img, img, Mat(), Mat(), Mat());
	auto end_time = high_resolution_clock::now();
	duration<double, milli> time_taken = end_time - start_time;
	printf("Time taken: %f\n", time_taken.count());

	return time_taken.count();
}

void find_coords() {
	Mat img;
	img = imread("image.png");
	cout << img.size() << endl;
	namedWindow("Display image", WINDOW_NORMAL);
	setMouseCallback("Display image", click_event, 0);
	imshow("Display image", img);
	while (true) {
		waitKey(0);
	}
}

void click_event (int event, int x, int y, int, void*) {
	if (event == EVENT_LBUTTONDOWN) {
		cout << "Left button of the mouse is clicked - position (" << x << "," << y << ")" << endl;
	}
}

Mat unfisheye (Mat input_frame) {
	Mat img_coords = Mat(20, 2, CV_32FC3);
	img_coords.at<double>(0, 0) = 74;
	img_coords.at<double>(0, 1) = 53;
	img_coords.at<double>(1, 0) = 94;
	img_coords.at<double>(1, 1) = 50;
	img_coords.at<double>(2, 0) = 116;
	img_coords.at<double>(2, 1) = 51;
	img_coords.at<double>(3, 0) = 136;
	img_coords.at<double>(3, 1) = 54;
	img_coords.at<double>(4, 0) = 69;
	img_coords.at<double>(4, 1) = 71;
	img_coords.at<double>(5, 0) = 93;
	img_coords.at<double>(5, 1) = 68;
	img_coords.at<double>(6, 0) = 119;
	img_coords.at<double>(6, 1) = 68;
	img_coords.at<double>(7, 0) = 141;
	img_coords.at<double>(7, 1) = 72;
	img_coords.at<double>(8, 0) = 67;
	img_coords.at<double>(8, 1) = 94;
	img_coords.at<double>(9, 0) = 91;
	img_coords.at<double>(9, 1) = 94;
	img_coords.at<double>(10, 0) = 121;
	img_coords.at<double>(10, 1) = 95;
	img_coords.at<double>(11, 0) = 144;
	img_coords.at<double>(11, 1) = 96;
	img_coords.at<double>(12, 0) = 68;
	img_coords.at<double>(12, 1) = 120;
	img_coords.at<double>(13, 0) = 93;
	img_coords.at<double>(13, 1) = 123;
	img_coords.at<double>(14, 0) = 120;
	img_coords.at<double>(14, 1) = 123;
	img_coords.at<double>(15, 0) = 142;
	img_coords.at<double>(15, 1) = 120;
	img_coords.at<double>(16, 0) = 73;
	img_coords.at<double>(16, 1) = 139;
	img_coords.at<double>(17, 0) = 95;
	img_coords.at<double>(17, 1) = 143;
	img_coords.at<double>(18, 0) = 118;
	img_coords.at<double>(18, 1) = 143;
	img_coords.at<double>(19, 0) = 138;
	img_coords.at<double>(19, 1) = 139;

	// img size is 204x192
	Mat target_img_coords = Mat(20, 2, CV_32FC2);
	int coord_count = 0;
	for (int i = 1; i < 6; i++) {
		for (int k = 4; k < 8; k++) {
			target_img_coords.at<double>(coord_count, 0) = ((double) 192 / 7) * (double) i;
			target_img_coords.at<double>(coord_count, 1) = ((double) 204 / 10) * (double) k;
			coord_count += 1;
		}
	}
	
	vector<Mat> rvecs_vec, tvecs_vec;
	vector<Mat> objpoints, imgpoints;
	objpoints.push_back(img_coords);
	imgpoints.push_back(target_img_coords);
	Matx33d K;
	Vec4d D;

	cout << img_coords << endl;
	cout << img_coords.at<double>(10, 0) << endl;
	cout << target_img_coords << endl;
	cout << target_img_coords.at<double>(10, 0) << endl;
	cout << img_coords.size() << endl;
	cout << target_img_coords.size() << endl;
	
	cout << img_coords.total() << endl;
	cout << target_img_coords.total() << endl;
	fisheye::calibrate(objpoints, imgpoints, Size(204, 192), K, D, rvecs_vec, tvecs_vec);
	
	cout << K << "\n";
	cout << D << "\n";
	// cout << rvecs_vec << "\n";
	// cout << tvecs_vec << "\n";

	Mat cameraMatrix = Mat(3,3, DataType<double>::type);
    Mat distortionCoeffs = Mat(4,1, DataType<double>::type);

    cameraMatrix.at<double>(0, 0) = 10;
    cameraMatrix.at<double>(0, 1) = 0;
    cameraMatrix.at<double>(0, 2) = 20;
    cameraMatrix.at<double>(1, 0) = 0;
    cameraMatrix.at<double>(1, 1) = 0;
    cameraMatrix.at<double>(1, 2) = 10;
    cameraMatrix.at<double>(2, 0) = 0;
    cameraMatrix.at<double>(2, 1) = 0;
    cameraMatrix.at<double>(2, 2) = 1;

    distortionCoeffs.at<double>(0,0) = -0.01078350003808737;
    distortionCoeffs.at<double>(1,0) = 0.04842806980013847;
    distortionCoeffs.at<double>(2,0) = -0.04542399942874908;
    distortionCoeffs.at<double>(3,0) = 0.008737384341657162;

    Mat E = Mat::eye(3, 3, DataType<double>::type);

    Size size = { input_frame.cols, input_frame.rows };

    Mat map1;
    Mat map2;

	fisheye::initUndistortRectifyMap(cameraMatrix, distortionCoeffs, E, cameraMatrix, size, CV_16SC2, map1, map2);

    Mat undistort;

    remap(input_frame, undistort, map1, map2, INTER_LINEAR,
              CV_HAL_BORDER_CONSTANT);

	return undistort;
}