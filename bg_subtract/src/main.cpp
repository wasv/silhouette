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
Mat frame; //current frame
Mat bgModel;
Mat fgMask;
char keyboard; //input from keyboard
void help()
{
    cout
    << "--------------------------------------------------------------------------" << endl
    << "This program shows how to use background subtraction methods provided by "  << endl
    << " OpenCV. You can process both videos (-vid) and images (-img)."             << endl
                                                                                    << endl
    << "Usage:"                                                                     << endl
    << "./bg_sub {-vid <video filename>|-img <image filename>}"                     << endl
    << "for example: ./bg_sub -vid video.avi"                                       << endl
    << "or: ./bg_sub -img /data/images/1.png"                                       << endl
    << "--------------------------------------------------------------------------" << endl
    << endl;
}

/** @function Dilation */
void Dilation( Mat *img, int dilation_type, int dilation_size)
{
  Mat element;
  element = getStructuringElement( dilation_type,
                                   Size( 3, dilation_size + 1 ),
                                   Point( 1, dilation_size/2 ) );
 
  /// Apply the dilation operation
  dilate( *img, *img, element );

  element = getStructuringElement( dilation_type,
                                       Size( 2*dilation_size + 1, 3 ),
                                       Point( dilation_size, 1 ) );
  erode( *img, *img, element );
}

void averageCapture(char* videoId) {
    //create the capture object
	int camId = videoId[0] - '0';
    VideoCapture capture(camId);
    while(!capture.isOpened()){}
    //read input data. ESC or 'q' for quitting
    keyboard = 0;
	int frameCnt = 1;
    while( keyboard != 'q' && keyboard != 27 ){
        //read the current frame
        if(!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
        }
        //update the background model
		blur(bgModel,bgModel,Size(3,3));
		bgModel = bgModel + (frame - bgModel) / frameCnt++;

        //get the frame number and write it on the current frame
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        stringstream ss;
        ss << frameCnt;
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //show the current frame and the fg masks
        //imshow("Frame", frame);
        //imshow("BG Model", bgModel);
        //get the input from the keyboard
        //keyboard = (char)waitKey( 30 );
    }
    //delete capture object
    capture.release();
}
void averageVideo(char* videoFilename) {
    //create the capture object
    VideoCapture capture(videoFilename);
    if(!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open video file: " << videoFilename << endl;
        exit(EXIT_FAILURE);
    }
    //read input data. ESC or 'q' for quitting
    keyboard = 0;
	int frameCnt;
    while( keyboard != 'q' && keyboard != 27 ){
        //read the current frame
        if(!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
        }
        //update the background model
		frameCnt = capture.get(CV_CAP_PROP_POS_FRAMES);
		bgModel = bgModel + (frame - bgModel) / frameCnt++;
		blur(bgModel,bgModel,Size(3,3));
        //get the frame number and write it on the current frame
        stringstream ss;
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        ss << frameCnt;
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //show the current frame and the fg masks
        //imshow("Frame", frame);
        //imshow("BG Model", bgModel);
        //get the input from the keyboard
        //keyboard = (char)waitKey( 30 );
    }
    //delete capture object
    capture.release();
}
void averageImages(char* firstFrameFilename) {
    //read the first file of the sequence
    frame = imread(firstFrameFilename);
    if(frame.empty()){
        //error in opening the first image
        cerr << "Unable to open first image frame: " << firstFrameFilename << endl;
        exit(EXIT_FAILURE);
    }
	Mat bkgImg;
    //current image filename
    string fn(firstFrameFilename);
    //read input data. ESC or 'q' for quitting
    keyboard = 0;
    while( keyboard != 'q' && keyboard != 27 ){
        //get the frame number and write it on the current frame
        size_t index = fn.find_last_of("0123456789");
        if(index == string::npos) {
            index = fn.find_last_of("\\");
        }
        size_t index2 = fn.find_last_of(".");
        string prefix = fn.substr(0,index);
        string suffix = fn.substr(index2);
        string frameNumberString = fn.substr(index, index2-index);
		cout << prefix << frameNumberString << suffix << endl;
        istringstream iss(frameNumberString);
        int frameNumber = 0;
        iss >> frameNumber;
        //rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
        //          cv::Scalar(255,255,255), -1);
        //putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
        //        FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));

        //update the background model
		if(bgModel.empty()) {
			bgModel = frame;
		} else {
			bgModel = (bgModel + (frame - bgModel)) / ++frameNumber;
		}
		blur(bgModel,bgModel,Size(3,3));
        //show the current frame and the fg masks
        //imshow("Frame", frame);
        //imshow("BG Model", bgModel);
        //get the input from the keyboard
        //keyboard = (char)waitKey( 30 );
        //search for the next image in the sequence
        ostringstream oss;
        oss << (frameNumber);
        string nextFrameNumberString = oss.str();
        string nextFrameFilename = prefix + nextFrameNumberString + suffix;
        //read the next frame
        frame = imread(nextFrameFilename);
        if(frame.empty()){
            //error in opening the next image in the sequence
            cerr << "Unable to open image frame: " << nextFrameFilename << endl;
			return;
        }
        //update the path of the current frame
        fn.assign(nextFrameFilename);
    }
}

void processCapture(char* videoId) {
    //create the capture object
	int camId = videoId[0] - '0';
    VideoCapture capture(camId);
    while(!capture.isOpened()){}
    //read input data. SPC or 'q' for advancing 
    keyboard = 0;
	int frameCnt = 0;
    while( keyboard != ' ' && keyboard != 'q' ){
        //read the current frame
        if(!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }
        //update the foreground mask
		Mat diff;
		diff = frame - bgModel;
		diff = abs(diff);
		blur(diff,diff,Size(3,3));
		fgMask = Mat::zeros(diff.rows, diff.cols, CV_32F);
		for(int j=0; j<diff.rows; j++) {
			for(int i=0; i<diff.cols; i++) {
				Vec3b p = diff.at<Vec3b>(j,i);

				float dist = (p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
				dist = sqrt(dist);

				fgMask.at<float>(j,i) = dist;
			}
		}
		normalize(fgMask,fgMask,0.0,1.0,NORM_MINMAX,CV_32F);
		threshold(fgMask,fgMask,0.25,255.0,THRESH_BINARY);
		fgMask.convertTo(fgMask,CV_8UC1);
		Mat masked;
		bitwise_and(frame,frame,masked,fgMask);
        //get the frame number and write it on the current frame
        stringstream ss;
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        ss << frameCnt++;
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //show the current frame and the fg masks
        //imshow("Frame", frame);
        //imshow("FG Mask", fgMask);
        //imshow("Masked Frame", masked);
        //get the input from the keyboard
        //keyboard = (char)waitKey( 30 );
    }
    //delete capture object
    capture.release();
}
void processVideo(char* videoFilename) {
    //create the capture object
    VideoCapture capture(videoFilename);
    if(!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open video file: " << videoFilename << endl;
        exit(EXIT_FAILURE);
    }
    //read input data. ESC or 'q' for quitting
    keyboard = 0;
    while( keyboard != 'q' && keyboard != 27 ){
        //read the current frame
        if(!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }
        //update the background model
		Mat diff;
		diff = frame - bgModel;
		diff = abs(diff);
		blur(diff,diff,Size(3,3));
		fgMask = Mat::zeros(diff.rows, diff.cols, CV_32F);
		for(int j=0; j<diff.rows; j++) {
			for(int i=0; i<diff.cols; i++) {
				Vec3b p = diff.at<Vec3b>(j,i);

				float dist = (p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
				dist = sqrt(dist);

				fgMask.at<float>(j,i) = dist;
			}
		}
		normalize(fgMask,fgMask,0.0,1.0,NORM_MINMAX,CV_32F);
		threshold(fgMask,fgMask,0.25,255.0,THRESH_BINARY);
		fgMask.convertTo(fgMask,CV_8UC1);
		Mat masked;
		bitwise_and(frame,frame,masked,fgMask);
        //get the frame number and write it on the current frame
        stringstream ss;
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        ss << capture.get(CV_CAP_PROP_POS_FRAMES);
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //show the current frame and the fg masks
        //imshow("Frame", frame);
		//imshow("FG Mask", fgMask);
        //imshow("Masked Frame", masked);
        //get the input from the keyboard
        //keyboard = (char)waitKey( 30 );
    }
    //delete capture object
    capture.release();
}
void processImages(char* firstFrameFilename) {
    //read the first file of the sequence
    frame = imread(firstFrameFilename);
    if(frame.empty()){
        //error in opening the first image
        cerr << "Unable to open first image frame: " << firstFrameFilename << endl;
        exit(EXIT_FAILURE);
    }
	Mat bkgImg;
    //current image filename
    string fn(firstFrameFilename);
    //read input data. ESC or 'q' for quitting
    keyboard = 0;
    while( keyboard != 'q' && keyboard != 27 ){
        //update the background model
		Mat diff;
		diff = frame - bgModel;
		diff = abs(diff);
		blur(diff,diff,Size(3,3));
		fgMask = Mat::zeros(diff.rows, diff.cols, CV_32F);
		for(int j=0; j<diff.rows; j++) {
			for(int i=0; i<diff.cols; i++) {
				Vec3b p = diff.at<Vec3b>(j,i);

				float dist = (p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
				dist = sqrt(dist);

				fgMask.at<float>(j,i) = dist;
			}
		}
		normalize(fgMask,fgMask,0,255,NORM_MINMAX,CV_8UC1);
		//		fgMask.convertTo(fgMask,CV_8UC1);
		//threshold(fgMask,fgMask,0,255.0,THRESH_BINARY|THRESH_OTSU);
		blur(fgMask,fgMask,Size(3,3));
		Canny(fgMask,fgMask,30,100,3);
		Dilation(&fgMask, MORPH_RECT, 4);
        //imshow("FG Mask", fgMask);
		Mat masked;
		bitwise_and(frame,frame,masked,fgMask);
        //get the frame number and write it on the current frame
        size_t index = fn.find_last_of("0123456789");
        if(index == string::npos) {
            index = fn.find_last_of("\\");
        }
        size_t index2 = fn.find_last_of(".");
        string prefix = fn.substr(0,index);
        string suffix = fn.substr(index2);
        string frameNumberString = fn.substr(index, index2-index);
		cout << prefix << frameNumberString << suffix << endl;
        istringstream iss(frameNumberString);
        int frameNumber = 0;
        iss >> frameNumber;
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //show the current frame and the fg masks
		//        imshow("Frame", frame);
        //        imshow("FG Mask", fgMask);
		
		imwrite("mask-"+frameNumberString+suffix, masked);

		bitwise_not(fgMask,fgMask);
		normalize(fgMask,fgMask,0,1,NORM_MINMAX,CV_8UC1);
		ofstream myFile("binmask"+frameNumberString+".bin", ios::out | ios::binary);
		myFile.write((char*)fgMask.data, fgMask.rows*fgMask.cols);
		myFile.close();

        //  imshow("Masked Frame", masked);
        //keyboard = (char)waitKey( 0 )
        //get the input from the keyboard
        //search for the next image in the sequence
        ostringstream oss;
        oss << (frameNumber + 1);
        string nextFrameNumberString = oss.str();
        string nextFrameFilename = prefix + nextFrameNumberString + suffix;
        //read the next frame
        frame = imread(nextFrameFilename);
        if(frame.empty()){
            //error in opening the next image in the sequence
            cerr << "Unable to open image frame: " << nextFrameFilename << endl;
			//nextFrameFilename = firstFrameFilename;
			//frame = imread(nextFrameFilename);
			return;
        }
        //update the path of the current frame
        fn.assign(nextFrameFilename);
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
    //create GUI windows
    //namedWindow("Frame");
    //namedWindow("BG Model");
    //namedWindow("FG Mask");
    //namedWindow("Masked Frame");
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
