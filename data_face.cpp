﻿#include <opencv2/opencv.hpp>
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/objdetect.hpp"
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>

using namespace std;
using namespace cv;


void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame)
{
	rectangle(frame, Point(left, top), Point(right, bottom), Scalar(0, 255, 0));

	std::string label = format("%.2f %d", conf, classId);

	int baseLine;
	Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

	top = max(top, labelSize.height);
	rectangle(frame, Point(left, top - labelSize.height),
		Point(left + labelSize.width, top + baseLine), Scalar::all(255), FILLED);
	putText(frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.5, Scalar());
}

int main(int argc, char** argv)
{
	int left, right, top, bottom;
	int count = 0;
	int Xsize = 250;
	int Ysize = 250;
	int smh = 75;
	//int col = atoi(argv[1]);
	int col = 500;
	Mat img, frame;
	//string face_id = argv[2];
	string face_id = "Chicha_";
	string folder = "D:/Bacha/";

	float confThreshold = 0.25;
	dnn::Net net = dnn::readNetFromCaffe("deploy.prototxt", "res10_300x300_ssd_iter_140000.caffemodel");
	// dnn::Net net = dnn::readNetFromTensorflow("c:/data/mdl/opencv_face_detector_uint8.pb","c:/data/mdl/opencv_face_detector.pbtxt");

	VideoCapture cap(0);
	static bool once = false;
	while (1) {
		Mat f, img;
		cap.read(f);
		Mat blob = dnn::blobFromImage(f, 1, Size(128, 96), Scalar(104, 177, 123, 0), false, false);
		net.setInput(blob);
		Mat res = net.forward("detection_out");

		// just for debugging; --  print out the network layers (once)
		if (!once) {
			dnn::MatShape ms1{ blob.size[0], blob.size[1] , blob.size[2], blob.size[3] };
			vector<String> lnames = net.getLayerNames();
			for (size_t i = 1; i<lnames.size(); i++) { // skip __NetInputLayer__
				Ptr<dnn::Layer> lyr = net.getLayer((unsigned)i);
				vector<dnn::MatShape> in, out;
				net.getLayerShapes(ms1, i, in, out);
				cerr << format("%-38s %-13s", lyr->name.c_str(), lyr->type.c_str());
				for (auto j : in) cerr << "i" << Mat(j).t() << "\t";
				for (auto j : out) cerr << "o" << Mat(j).t() << "\t";
				cerr << endl;
			}
			once = true;
		}
		Mat faces(res.size[2], res.size[3], CV_32F, res.ptr<float>());
		//cout << res.size << " " << faces.size() << endl;
		// cout << faces << endl;
		for (int i = 0; i<faces.rows; i++)
		{
			float *data = faces.ptr<float>(i);
			float confidence = data[2];
			if (confidence > confThreshold)
			{
				left = (int)(data[3] * f.cols);
				top = (int)(data[4] * f.rows);
				right = (int)(data[5] * f.cols);
				bottom = (int)(data[6] * f.rows);
				int classId = (int)(data[1]) - 1;  // Skip 0th background class id.
				//drawPred(classId, confidence, left, top, right, bottom, f);
				cout << classId << " " << confidence << " " << left << " " << top << " " << right << " " << bottom << endl;
			}
		}

		Rect roi(left - smh, top - smh, right - left + smh * 2, bottom - top + smh * 2);
		
		if (0 <= roi.x && 0 <= roi.width && roi.x + roi.width <= f.cols && 0 <= roi.y && 0 <= roi.height && roi.y + roi.height <= f.rows) {
				img = f(roi);
				resize(img, f, Size(Xsize, Ysize), 0, 0, 3);
				imwrite(folder + face_id + to_string(count) + ".jpg", f);
				imshow("F", f);

				count++;
				cout << count << endl;
				if (count == col)
					break;
		}
		
		int k = waitKey(19);
		if (k>0) break;
		frame.release();
		faces.release();
		img.release();
		f.release();

	}
	return 0;
}
