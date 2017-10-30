//opencv
#include <opencv2/opencv.hpp>

//C
#include <stdio.h>
//C++
#include <iostream>
#include <fstream>
#include <sstream>
using namespace cv;
using namespace std;
// Global variables
Mat bgModel;

void help()
{
	cout
	<< "--------------------------------------------------------------------------" << endl
	<< "This program shows how to use background subtraction methods provided by "  << endl
	<< " OpenCV. You can process both videos (-vid) and images (-img)."             << endl
																					<< endl
	<< "Usage:"                                                                     << endl
	<< "./bg_sub {-vid <video filename>|-img <image filename> | -cap <videoid>}"    << endl
	<< "for example: ./bg_sub -vid bg.avi -img fg0.png"                             << endl
	<< "--------------------------------------------------------------------------" << endl
	<< endl;
}

void morph( Mat &img, int type, int dilation_size, int erode_size)
{
  Mat element;
  if(dilation_size != 0) {
	  element = getStructuringElement( type,
									   Size( (4*dilation_size) + 1, (2*dilation_size) + 1 ),
									   Point( 2*dilation_size, dilation_size ) );
	  dilate( img, img, element );
  }
  
  if(erode_size != 0) {
	  element = getStructuringElement( type,
									   Size( (2*erode_size) + 1, (4*erode_size) + 1 ),
									   Point( 2*erode_size, erode_size ) );
	  erode( img, img, element );
  }
}

void processFrame(Mat frame, int frameNumber){
	//update the background model
	Mat diff;
	diff = frame - bgModel;
	diff = abs(diff);
	blur(diff,diff,Size(3,3));
	Mat fgMask = Mat::zeros(diff.rows, diff.cols, CV_32F);
	for(int j=0; j<diff.rows; j++) {
		for(int i=0; i<diff.cols; i++) {
			Vec3b p = diff.at<Vec3b>(j,i);

			float dist = (p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
			dist = sqrt(dist);

			fgMask.at<float>(j,i) = dist;
		}
	}
	normalize(fgMask,fgMask,0,255,NORM_MINMAX,CV_8UC1);
	blur(fgMask,fgMask,Size(3,3));
	Mat masked = Mat::zeros(frame.rows,frame.rows,CV_8UC1);
	threshold(fgMask, masked, 24, 255, CV_THRESH_TOZERO);
	morph(masked,MORPH_RECT,4,8);

	imwrite(to_string(frameNumber)+"fgmask.png", fgMask);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	
	findContours( masked, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

    int maxArea = 0;
	int contourIndex = 0;
    // test each contour
    for( size_t i = 0; i < contours.size(); i++ )
    {
        // Find largest contour
        if(fabs(contourArea(contours[i])) > maxArea)
        {
            maxArea = fabs(contourArea(contours[i]));
			contourIndex = i;
        }
    }

	masked = Mat::zeros(frame.rows,frame.rows,CV_8UC1);
	drawContours( masked, contours, contourIndex, Scalar(255), 2, 8, hierarchy, 0, Point() );
	
	imwrite(to_string(frameNumber)+"edges.png", masked);

	bitwise_not(fgMask,fgMask);
	normalize(fgMask,fgMask,0,1,NORM_MINMAX,CV_8UC1);
	ofstream myFile("binmask"+to_string(frameNumber)+".bin", ios::out | ios::binary);
	myFile.write((char*)fgMask.data, fgMask.rows*fgMask.cols);
	myFile.close();
}

// Takes in old filename, replaces filename with new one, returns framenumber.
int getNextFilename(string &fn) {
	//get the frame number and write it on the current frame
	size_t base = fn.find_last_of("/");
	if(base == string::npos) {
		base = fn.find_last_of("\\");
	}
	size_t index = fn.find_first_of("0123456789", base);
	size_t index2 = fn.find_last_of(".");

	string prefix = fn.substr(0,index);
	string suffix = fn.substr(index2);
	string frameNumberString = fn.substr(index, index2-index);

	int frameNumber = stoi(frameNumberString);
	fn = prefix + to_string(frameNumber + 1) + suffix;
	return frameNumber;
}

void updateModel(Mat frame, int frameCount) {
	if(bgModel.empty()) {
		bgModel = frame.clone();
	} else {
	    bgModel = bgModel + ((frame - bgModel) / ++frameCount);
	}
}

void averageCapture(char* videoId) {
	Mat frame;
	//create the capture object
	int camId = videoId[0] - '0';
	VideoCapture capture(camId);
	while(!capture.isOpened()){}
	int frameNumber = 0;
	while(1){
		if(!capture.read(frame)) {
			cerr << "Stopped on frame " << frameNumber << endl;
			break;
		}

		updateModel(frame,frameNumber);

		frameNumber += 1;
	}
	//delete capture object
	capture.release();
}
void averageVideo(char* videoFilename) {
	Mat frame;
	//create the capture object
	VideoCapture capture(videoFilename);
	if(!capture.isOpened()){
		//error in opening the video input
		cerr << "Unable to open video file: " << videoFilename << endl;
		exit(EXIT_FAILURE);
	}
	int frameNumber;
	while(1){
		if(!capture.read(frame)) {
			cerr << "Stopped on frame " << frameNumber << endl;
			break;
		}

		frameNumber = capture.get(CV_CAP_PROP_POS_FRAMES);

		updateModel(frame,frameNumber);
	}
	//delete capture object
	capture.release();
}
void averageImages(string filename) {
	//read the first file of the sequence
	Mat frame = imread(filename);
	if(frame.empty()){
		//error in opening the first image
		cerr << "Unable to open first image frame: " << filename << endl;
		exit(EXIT_FAILURE);
	}
	//current image filename
	while(1){
		int frameNumber = getNextFilename(filename);

		//update the background model
		updateModel(frame,frameNumber);

		//read the next frame
		frame = imread(filename);
		if(frame.empty()){
			//error in opening the next image in the sequence
			cerr << "Background model complete on frame: " << frameNumber << endl;
			break;
		}
	}
}

void processCapture(char* videoId) {
	Mat frame;
	//create the capture object
	int camId = videoId[0] - '0';
	VideoCapture capture(camId);
	while(!capture.isOpened()){}
	int frameNumber;
	while(1){
		//read the current frame
		if(!capture.read(frame)) {
			cerr << "Stopped on frame " << frameNumber << endl;
			break;
		}

		frameNumber = capture.get(CV_CAP_PROP_POS_FRAMES) - 1;

		processFrame(frame,frameNumber);
		frameNumber += 1;
	}
	//delete capture object
	capture.release();
}
void processVideo(char* videoFilename) {
	Mat frame;
	int frameNumber;
	//create the capture object
	VideoCapture capture(videoFilename);
	if(!capture.isOpened()){
		//error in opening the video input
		cerr << "Unable to open video file: " << videoFilename << endl;
		exit(EXIT_FAILURE);
	}
	while(1){
		//read the current frame
		if(!capture.read(frame)) {
			cerr << "Stopped on frame " << frameNumber << endl;
			break;
		}

		frameNumber = capture.get(CV_CAP_PROP_POS_FRAMES) - 1;

		processFrame(frame,frameNumber);
	}
	//delete capture object
	capture.release();
}
void processImages(string filename) {
	//read the first file of the sequence
	Mat frame = imread(filename);
	if(frame.empty()){
		//error in opening the first image
		cerr << "Unable to open first image frame: " << filename << endl;
		exit(EXIT_FAILURE);
	}
	while(1){

		int frameNumber = getNextFilename(filename);

		processFrame(frame,frameNumber);

		//read the next frame
		frame = imread(filename);
		if(frame.empty()){
			//error in opening the next image in the sequence
			cerr << "Image processing complete on frame: " << frameNumber << endl;
			break;
		}
	}
}
int main(int argc, char* argv[])
{
	//print help information
	help();
	//check for the input parameter correctness
	if(argc != 5) {
		cerr <<"Incorret input list" << endl;
		cerr <<"exiting..." << endl;
		return EXIT_FAILURE;
	}
	//create Background Subtractor objects
	if(strcmp(argv[1], "-cap") == 0) {
		//input data coming from a video
		averageCapture(argv[2]);
	}
	else if(strcmp(argv[1], "-vid") == 0) {
		//input data coming from a video
		averageVideo(argv[2]);
	}
	else if(strcmp(argv[1], "-img") == 0) {
		//input data coming from a sequence of images
		averageImages(argv[2]);
	}
	else {
		//error in reading input parameters
		cerr <<"Please, check the input parameters." << endl;
		cerr <<"Exiting..." << endl;
		return EXIT_FAILURE;
	}
	if(strcmp(argv[3], "-cap") == 0) {
		//input data coming from a video
		processCapture(argv[4]);
	}
	else if(strcmp(argv[3], "-vid") == 0) {
		//input data coming from a video
		processVideo(argv[4]);
	}
	else if(strcmp(argv[3], "-img") == 0) {
		//input data coming from a sequence of images
		processImages(argv[4]);
	}
	else {
		//error in reading input parameters
		cerr <<"Please, check the input parameters." << endl;
		cerr <<"Exiting..." << endl;
		return EXIT_FAILURE;
	}
	//destroy GUI windows
	destroyAllWindows();
	return EXIT_SUCCESS;
}
