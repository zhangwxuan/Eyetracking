// this is update online at the sametime 

#include <sstream>
#include <string>
#include <iostream>
#include <time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2\video\tracking.hpp>
#include <stdio.h>
#include <tchar.h>
#include "SerialClass.h"	// Library described above
#include <Windows.h>
#include <cstdlib>
#include <conio.h>
 
using namespace cv;
using namespace std;
//initial min and max HSV filter values.
//these will be changed using trackbars


int H_MIN = 14;
int H_MAX = 256;
int S_MIN = 140;
int S_MAX = 256;
int V_MIN = 119;
int V_MAX = 256;


//default capture width and height
const int FRAME_WIDTH = 1280;
const int FRAME_HEIGHT = 720;

//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 20;                             //make sure no noise to interupt

//minimum and maximum object area
const int MIN_OBJECT_AREA = 20 * 20;                        //no need
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5; //no need

//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";
const string windowName4 = "Cursor Control Panel";
bool objectFound = false;          // if is tracking the object
bool cali = true;                  // true when need to calibtaion 

int x_min, x_max, y_min, y_max, x_min1, x_max1, y_min1, y_max1,x_o,y_o;
int dx, dy, x_1, x_2, y_1, y_2, Spacex_1, Spacex_2, Spacex_3, Spacey_1, Spacey_2;
int stop=0;  // to count the time to shut down system automatically 
int userOpt; // option for operation mode 
int cursor_x = 960, cursor_y = 540; 

POINT p; 

void on_trackbar(int, void*)
{//This function gets called whenever a
	// trackbar position is changed

}

string intToString(int number){

	std::stringstream ss;
	ss << number;
	return ss.str();
}

//functions: 

//to create the control HSV bar
void createTrackbars(){
	//create window for trackbars

	namedWindow(trackbarWindowName, 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf_s(TrackbarName, "H_MIN", H_MIN);
	sprintf_s(TrackbarName, "H_MAX", H_MAX);
	sprintf_s(TrackbarName, "S_MIN", S_MIN);
	sprintf_s(TrackbarName, "S_MAX", S_MAX);
	sprintf_s(TrackbarName, "V_MIN", V_MIN);
	sprintf_s(TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH),
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);


}

//to draw ROI 
void drawObject(int x, int y, Mat &frame){

	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!

	//UPDATE:JUNE 18TH, 2013
	//added 'if' and 'else' statements to prevent
	//memory errors from writing off the screen (ie. (-25,-25) is not within the window!)

	circle(frame, Point(x, y), 20, Scalar(0, 0, 255), 2);
	if (y - 25>0)
		line(frame, Point(x, y), Point(x, y - 25), Scalar(0, 0, 255), 2);
	else line(frame, Point(x, y), Point(x, 0), Scalar(0, 0, 255), 2);
	if (y + 25<FRAME_HEIGHT)
		line(frame, Point(x, y), Point(x, y + 25), Scalar(0, 0, 255), 2);
	else line(frame, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 0, 255), 2);
	if (x - 25>0)
		line(frame, Point(x, y), Point(x - 25, y), Scalar(0, 0, 255), 2);
	else line(frame, Point(x, y), Point(0, y), Scalar(0, 0, 255), 2);
	if (x + 25<FRAME_WIDTH)
		line(frame, Point(x, y), Point(x + 25, y), Scalar(0, 0, 255), 2);
	else line(frame, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 0, 255), 2);

	putText(frame, intToString(x) + "," + intToString(y), Point(x, y + 30), 1, 1, Scalar(0, 0, 255), 2);

}

//doing morphOps to the roi
void morphOps(Mat &thresh){
	/*
	Erode and Dialate:
	para1: shape: MORPH_RECT MORPH_CROSS  MORPH_ELLIPSE

	*/

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
	//dilate with larger element so make sure object is nicely visible                      
	Mat dilateElement = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);


	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);



}

//do the tracking things 
void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed){

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	//bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects<MAX_NUM_OBJECTS){
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea){                    //MAKE SURE IS TRACKING CORRECTLY
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
					refArea = area;
					if (x >= 1280 || y >= 720)
						objectFound = false; 
				}
				else objectFound = false;

			}
			//let user know you found an object
			if (objectFound == true){
				putText(cameraFeed, "Tracking Mode", Point(0, 50), 2, 1, Scalar(0, 0, 255), 2);
				//draw object location on screen
				drawObject(x, y, cameraFeed);
			}
		}
		else putText(cameraFeed, "PLEASE ADJUST FILTER", Point(0, 50), 2, 1, Scalar(0, 0, 255), 2);
	}
}

// doing eye stare  calibration 
void calibration(int x, int y, Mat &cameraFeed, double duration){
	putText(cameraFeed, "Calibration:", Point(0, 90), 2, 1, Scalar(0, 0, 255), 2);

	if (duration < 5)
	{
		putText(cameraFeed, "Doing Calibration:", Point(450, 600), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
		putText(cameraFeed, "-Please keep staring at the red dot", Point(450, 630), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
		putText(cameraFeed, "-Please avoid blinking eyes during staring", Point(450, 660), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	}
	else if (duration < 8)
	{
		x_min = x;
		y_min = y;
		circle(cameraFeed, Point(20, 20), 20, Scalar(0, 0, 255), CV_FILLED, 8);
	}
	else if (duration < 11)
	{
		x_max = x;
		y_min1 = y;
		circle(cameraFeed, Point(1260, 20), 20, Scalar(0, 0, 255), CV_FILLED, 8);
	}
	else if (duration < 14)
	{
		x_max1 = x;
		y_max = y;
		circle(cameraFeed, Point(1260, 700), 20, Scalar(0, 0, 255), CV_FILLED, 8);
	}
	else if (duration < 17)
	{
		x_min1 = x;
		y_max1 = y;
		circle(cameraFeed, Point(20, 700), 20, Scalar(0, 0, 255), CV_FILLED, 8);
	}
	else if (duration < 20)
	{
		x_o = x;
		y_o = y;
		circle(cameraFeed, Point(630, 350), 20, Scalar(0, 0, 255), CV_FILLED, 8);
	}
	else if (duration < 23)
	{
		x_min = (x_min + x_min1) / 2;
		x_max = (x_max + x_max1) / 2;
		y_min = (y_min + y_min1) / 2;
		y_max = (y_max + y_max1) / 2;
		dx = (x_max - x_min) / 3;
		dy = (y_max - y_min) / 3;
		x_1 = x_o- dx;
		y_1 = y_o - dy;
		x_2 = x_o + dx;
		y_2 = y_o + dy;
		Spacex_1= dx;
		Spacex_2 = dx * 2;
		Spacex_3 = 480;
		Spacey_1 = (y_max - y_min) / 2;
		Spacey_2 = 300;
        cali = false;
	}

}

// doing eye stare  calibration2: fir cursor control 
void calibration2(int x, int y, Mat &cameraFeed, double duration){
	putText(cameraFeed, "Calibration:", Point(20, 20), 2, 1, Scalar(0, 0, 0), 2);

	if (duration < 5)
	{
		putText(cameraFeed, "Follow red dots :", Point(20, 50), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 0), 2);

	}
	else if (duration < 8)
	{
		x_min = x;
		y_min = y;
		circle(cameraFeed, Point(10, 10), 10, Scalar(0, 0, 255), CV_FILLED, 8);
	}
	else if (duration < 11)
	{
		x_max = x;
		y_min1 = y;
		circle(cameraFeed, Point(470, 10), 10, Scalar(0, 0, 255), CV_FILLED, 8);
	}
	else if (duration < 14)
	{
		x_max1 = x;
		y_max = y;
		circle(cameraFeed, Point(470, 280), 10, Scalar(0, 0, 255), CV_FILLED, 8);
	}
	else if (duration < 17)
	{
		x_min1 = x;
		y_max1 = y;
		circle(cameraFeed, Point(10, 280), 10, Scalar(0, 0, 255), CV_FILLED, 8);
	}
	else if (duration < 20)
	{
		x_o = x;
		y_o = y;
		circle(cameraFeed, Point(230, 140), 20, Scalar(0, 0, 255), CV_FILLED, 8);
	}
	else if (duration < 23)
	{
		x_min = (x_min + x_min1) / 2;
		x_max = (x_max + x_max1) / 2;
		y_min = (y_min + y_min1) / 2;
		y_max = (y_max + y_max1) / 2;
		dx = (x_max - x_min) / 3;
		dy = (y_max - y_min) / 3;
		Spacex_1 = x_min + dx;
		Spacex_2 = x_min + dx * 2;
		Spacex_3 = x_min + dx * 3;
		Spacey_1 = (y_max - y_min) / 2+ y_min;
		Spacey_2 = y_max;
		cali = false;
	}

}

//devide the screen for different area  
void screen(int x_1, int x_2, int y_1, int y_2, Mat &frame ){

	line(frame, Point(x_1, 0), Point(x_1, 720), Scalar(255), 1, 8, 0);
	line(frame, Point(x_2, 0), Point(x_2, 720), Scalar(255), 1, 8, 0);
	line(frame, Point(0, y_1), Point(1280, y_1), Scalar(255), 1, 8, 0);
	line(frame, Point(0,y_2), Point(1280, y_2), Scalar(255), 1, 8, 0);
}

void screen2(int x_min, int y_min, int spacex_3, int spacey_2, Mat &frame){

	line(frame, Point(x_min, y_min), Point(Spacex_3, y_min), Scalar(255), 1, 8, 0);
	line(frame, Point(x_min, Spacey_1), Point(Spacex_3, Spacey_1), Scalar(255), 1, 8, 0);
	line(frame, Point(x_min, Spacey_2), Point(Spacex_3, Spacey_2), Scalar(255), 1, 8, 0);

	line(frame, Point(x_min, y_min), Point(x_min, Spacey_2), Scalar(255), 1, 8, 0);
	line(frame, Point(Spacex_1, y_min), Point(Spacex_1, Spacey_2), Scalar(255), 1, 8, 0);
	line(frame, Point(Spacex_2, y_min), Point(Spacex_2, Spacey_2), Scalar(255), 1, 8, 0);
	line(frame, Point(Spacex_3, y_min), Point(Spacex_3, Spacey_2), Scalar(255), 1, 8, 0);
}

//write on X,Y coordinates after calibration  
void PutXY(Mat &cameraFeed){
	putText(cameraFeed, "Xmin=" + intToString(x_min), Point(1000, 540), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	putText(cameraFeed, "Ymin=" + intToString(y_min), Point(1000, 560), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	putText(cameraFeed, "X1=" + intToString(Spacex_1), Point(1000, 580), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	putText(cameraFeed, "X2=" + intToString(Spacex_2), Point(1000, 600), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	putText(cameraFeed, "X3=" + intToString(Spacex_3), Point(1000, 620), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	putText(cameraFeed, "Y1=" + intToString(Spacey_1), Point(1000, 640), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	putText(cameraFeed, "Y2=" + intToString(Spacey_2), Point(1000, 660), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	putText(cameraFeed, "Controlling", Point(0, 80), 2, 1, Scalar(0, 255, 0), 2);
}

//to generate the input to arduino 
char control(int x, int y){ //      need to go backk and check!!!!

	if ((x - x_1) < 0)
	{
		return '4'; //return left
	}
	else if ((x - x_2) > 0)
	{
		return '3'; // return right;
	}
	else if ((y - y_1) < 0)
	{
		return '1'; // return forward;
	}
	else if ((y - y_2) > 0)
	{
		return '2'; //backward 
	}

	else
	{
		return '5'; // stop; 
	}
}

// cursor control panel : control 2: 
char control2(int x, int y){

	if (x_min < x && x < Spacex_1){
		if (y_min<y && y <Spacey_1)
		{
			return '5';  //x1 y1: left click 
		}

		else if (Spacey_1< y && y<Spacey_2)
		{
			return '3'; //x1 y2: left 
		}
		else return'7';

	}
	else if (Spacex_1 < x && x < Spacex_2){
		if (y_min<y &&  y < Spacey_1)
		{
			return '1';//x2 y1: up 
		} 
		else if (Spacey_1< y && y < Spacey_2)
		{
			return '2';//x2 y2: down 
		} 
		else return'7';
	}
	else  if (Spacex_2< x && x < Spacex_3){
		if (y_min<y &&  y < Spacey_1)
		{
			return '6'; //x3 y1: right click 
		}
		else if (Spacey_1< y && y < Spacey_2)
		{
			return '4'; //x3 y2: right 
		}
		else return'7';
	}
	else return ' 7';

}

//display control instruction 
void instruction(int input, int x, int y, Mat &frame){
	double alpha = 0.3; 
	if (input == '4')    //turning left
	{
	 Mat roi = frame(Rect(Point(0, 0), Point(x_1, 720)));
	 Mat color(roi.size(), CV_8UC3, Scalar(0, 125, 125));
	 addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);
	 putText(frame, "Turn Left", Point(x+20,y+20), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	}

	else if (input == '3')      //turning right 
	{
		Mat roi = frame(Rect(Point(x_2, 0), Point(1280, 720)));
		Mat color(roi.size(), CV_8UC3, Scalar(0, 125, 125));
		addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);
		putText(frame, "Turn Right", Point(x + 20, y + 20), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	}

	else if (input == '2')     //back 
	{
		Mat roi = frame(Rect(Point(x_1, y_2), Point(x_2, 720)));
		Mat color(roi.size(), CV_8UC3, Scalar(0, 125, 125));
		addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);
		putText(frame, "Going Back", Point(x + 20, y + 20), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	}

	else if (input == '1')    //forward 
	{
		Mat roi = frame(Rect(Point(x_1, 0), Point(x_2, y_1)));
		Mat color(roi.size(), CV_8UC3, Scalar(0, 125, 125));
		addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);
		putText(frame, "Going Forward", Point(x + 20, y + 20), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	}
	else if (input == '5')    //stop 
	{
		Mat roi = frame(Rect(Point(x_1, y_1), Point(x_2, y_2)));
		Mat color(roi.size(), CV_8UC3, Scalar(0, 125, 125));
		addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);
		putText(frame, "Stop", Point(x + 20, y + 20), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	}

}

// display selected area for control panel 2:
void instruction2(int input,  Mat &frame){
	double alpha = 0.3;
	if (input == '1')    // up
	{
		Mat roi = frame(Rect(Point(160, 0), Point(320, 150)));
		Mat color(roi.size(), CV_8UC3, Scalar(0, 0, 0));
		addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);
	}

	else if (input == '2')      //down
	{
		Mat roi = frame(Rect(Point(160, 150), Point(320, 300)));
		Mat color(roi.size(), CV_8UC3, Scalar(0, 0, 0));
		addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);
	}

	else if (input == '3')     //left 
	{
		Mat roi = frame(Rect(Point(0, 150), Point(160, 300)));
		Mat color(roi.size(), CV_8UC3, Scalar(0, 0, 0));
		addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);
	}

	else if (input == '4')    //right 
	{
		Mat roi = frame(Rect(Point(320, 150), Point(480, 300)));
		Mat color(roi.size(), CV_8UC3, Scalar(0, 0, 0));
		addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);
	}
	else if (input == '5')    //stop 
	{
		Mat roi = frame(Rect(Point(0, 0), Point(160, 150)));
		Mat color(roi.size(), CV_8UC3, Scalar(0, 0, 0));
		addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);
	}

	else if (input == '6')    //stop 
	{
		Mat roi = frame(Rect(Point(320, 0), Point(480, 150)));
		Mat color(roi.size(), CV_8UC3, Scalar(0, 0, 0));
		addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);
	}
}


//add filter for reflection 
void addColor( Mat &cameraFeed){
	Mat roi = cameraFeed(Rect(Point(0, 0), Point(1280, 720)));
	Mat color(roi.size(), CV_8UC3, Scalar(0, 125, 125));
	addWeighted(color, 0.5, roi, 0.5, 0.0, roi);

}

// draw the control box 
void ctrlBox( Mat &output){
	//output = Scalar(0, 255, 0);
	rectangle(output, Point(0, 0), Point(480,300), Scalar(130,240,120), CV_FILLED,8);
	line(output, Point(160, 0), Point(160, 300), Scalar(255));
	line(output, Point(320, 0), Point(320, 300), Scalar(255));
	line(output, Point(0, 150), Point(500, 150), Scalar(255));

	putText(output, "Left", Point(50,240), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	putText(output, "Right", Point(370, 240), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	putText(output, "Up", Point(220, 90), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	putText(output, "Down", Point(200,240), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);

	putText(output, "Click(L)", Point(30, 90), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
	putText(output, "Click(R)", Point(350, 90), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);


}

//call cursor events: 
void cursor(int input){
	GetCursorPos(&p);
	cursor_x = p.x;
	cursor_y = p.y;
	switch ((input)) {
	case '1':  //key up
		cursor_y = cursor_y - 5;
		SetCursorPos(cursor_x, cursor_y);
		break;
	case '2': // key down
		cursor_y = cursor_y + 5;
		SetCursorPos(cursor_x, cursor_y);
		break;
	case '3':  // key left
		cursor_x = cursor_x - 5;
		SetCursorPos(cursor_x, cursor_y);
		break;
	case '4':  // key right
		cursor_x = cursor_x + 5;
		SetCursorPos(cursor_x, cursor_y);
		break;
	case '5':
		mouse_event(MOUSEEVENTF_LEFTDOWN, cursor_x, cursor_y, 0, 0); // moving cursor leftdown
		mouse_event(MOUSEEVENTF_LEFTUP, cursor_x, cursor_y, 0, 0); // moving cursor leftup //for accessing your required co-ordinate
		break;

	case '6':
		mouse_event(MOUSEEVENTF_RIGHTDOWN, cursor_x, cursor_y, 0, 0); // moving cursor leftdown
		mouse_event(MOUSEEVENTF_RIGHTUP, cursor_x, cursor_y, 0, 0); // moving cursor leftup //for accessing your required co-ordinate
		break; 
   
	default:
		cout << "null" << endl;  // not arrow
		break;
	}
	Sleep(20);

}



//MAIN PROGRAM


int main(int argc, char* argv[])
{
	//first to initial varibales 

#pragma region variables
	//some boolean variables for different functionality within this
	bool trackObjects = true;     //if need to track objects 
	bool useMorphOps = true;      // tracking shadows 
	//bool isTracking = false;    
	bool start_time = true;       //calibration time function, true when calibration starts 
	bool startControl = false;    // if start to control the arduino, true when calibration finish, false when track lost 
	bool ArduinoConnect = false;  //state if arduino is connected to computer 
	bool flipI = false;
	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	//matrix storage for HSV image
	Mat HSV;
	//matrix storage for binary threshold image
	Mat threshold, temp, inverse;
	//x and y values for the location of the object
	int x = 0, y = 0;
	//create slider bars for HSV filtering
	int x_left, x_right, y_up, y_down, x_center, y_center;
	//these are for calibration
	createTrackbars();
	//video capture object to acquire webcam feed
	double duration;
	clock_t start, startx;
	int timex;
	//is for counitng time display
	char input; // arduino input 
	char calibrationOption; 
	bool Option1 = true;

#pragma endregion

	// to select the opertation mode: 
	cout << "Please select the mode " << endl;
	cout << "1. Control arduino" << endl;
	cout << "2. Control mouse cursur " << endl;
	cout << "please enter the number: ..." << endl;
	cin >> userOpt;

		cout << "loading..." << endl<<endl;
#pragma region arduinoSetUp

			Serial* port = new Serial("COM4"); //connect to arduino        Here to change the port number 

			//Arduino Communication set up:

			char data[8] = "";
			char command[2] = "";
			int datalength = 8;  //length of the data,
			int readResult = 0;
			int n;
			int opt; // to choose functions 
			for (int i = 0; i < 8; ++i) { data[i] = 0; }
#pragma endregion 

		VideoCapture capture;
		//open capture object at location zero (default location for webcam)
		capture.open(0);
		//set height and width of capture frame
		capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
		capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
		//start an infinite loop where webcam feed is copied to cameraFeed matrix
		//all of our operations will be performed within this loop


		while (1){

#pragma region basic tracking function 

			//store image to matrix and flip the image
			capture.read(cameraFeed);


			// to flip over image 
			if (flipI)
			{
				cameraFeed.copyTo(inverse);
				flip(inverse, cameraFeed, 0);
			}

			cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
			inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);

			
			//MorphOps
			if (useMorphOps)
				morphOps(threshold);


			//check port connection:  
			if (userOpt == 1){
				if (port->IsConnected())
				{
					putText(cameraFeed, "Arduino Connected", Point(1000, 60), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
					ArduinoConnect = true;
				}
				else
				{
					putText(cameraFeed, "Not Connected", Point(1000, 60), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
					ArduinoConnect = false;
				}
			}


			//pass in thresholded frame to our object tracking function
			//this function will return the x and y coordinates of the
			if (trackObjects)
				trackFilteredObject(x, y, threshold, cameraFeed);



#pragma endregion 
			//for option 1: 
			//control and send to arduino 


			if (userOpt == 1){
#pragma region option1_Control

				//calibration 
				if (cali == true && objectFound == true)
				{
					if (start_time){
						start = clock();
						start_time = false;
					}

					duration = (double)(clock() - start) / CLOCKS_PER_SEC;

					if (duration < 23)
						calibration(x, y, cameraFeed, duration);
				}

				else
					start = clock();


				//after calibration and can start to control 
				if (cali == false){
					PutXY(cameraFeed);
					startControl = true;
				}

				if (startControl == true){

					input = control(x, y);

					if (objectFound == false)
						input = '5';

					command[0] = input;
					int msglen = strlen(command);
					if (port->WriteData(command, msglen));  //write to arduino
					printf("Wrtiting Success\n");
					Sleep(15);
					n = port->ReadData(data, 8);
					if (n != -1){
						data[n] = 0;
						cout << "arduino: " << data << endl;
					}
				}

				// showing dividing lines: 
				screen(x_1, x_2, y_1, y_2, cameraFeed);

				//adding shadow to the selected area 
				instruction(input, x, y, cameraFeed);
#pragma endregion 		
			}


			//for option 2: 
			//control the cursor: 

          

			if (userOpt == 2){


#pragma region   option2_Control

				Mat output;
				output = Mat::zeros(Size(480, 300), CV_8UC3);
				ctrlBox(output);

				//if catch the calibration or by input: 
				if (Option1){
					cout << "Please select calibration method " << endl;
					cout << "1. By system measure " << endl;
					cout << "2. By user input " << endl;
					cout << "3. By example input " << endl;
					cout << "please enter the number: ..." << endl;
					cin >> calibrationOption;
					Option1 = false;
				}

				//calibration 
				if (!startControl){
					//option 1: control by system calibraion 
					if (calibrationOption == '1'){
						cout << "start calibration process";
						if (cali == true && objectFound == true)
						{
							if (start_time){
								start = clock();
								start_time = false;
							}

							duration = (double)(clock() - start) / CLOCKS_PER_SEC;

							if (duration < 23)
								calibration2(x, y, output, duration);
						}

						else
							start = clock();


						//after calibration and can start to control 
						if (cali == false){
							PutXY(cameraFeed);
							startControl = true;
						}

					}

					//option 2: control by input numbers 
					if (calibrationOption == '2'){
						cout << "please enter number:";
						cout << "x_min:";  	cin >> x_min;
						cout << "SpaceX_1: ";  cin >> Spacex_1;
						cout << "SpaceX_2: ";  cin >> Spacex_2;
						cout << "SpaceX_3: ";  cin >> Spacex_3;
						cout << "y_min:";      cin >> y_min;
						cout << "SpaceY_1: ";  cin >> Spacey_1;
						cout << "SpaceY_2: ";   cin >> Spacey_2;
						PutXY(cameraFeed);
						startControl = true;
					}

					if (calibrationOption == '3'){
						x_min = 510; Spacex_1 = 625;  Spacex_2 = 740; Spacex_3 = 855;
						y_min = 411; Spacey_1 = 516;  Spacey_2 = 622;
						startControl = true;
					}

				}

				if (startControl){
					input = control2(x, y); 
					// to control the cursor movement 
				//   	cursor(input);
					//high light selected area 
					//cout << input; 
					if (objectFound)
					{
						instruction2(input, output);
						cursor(input);
					}
				
					}
				

				if (objectFound)
					putText(output, "Tracking:", Point(350, 20), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0), 2);

				putText(cameraFeed, "control =" + intToString(input), Point(1000, 500), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);
				screen2(x_min, y_min, Spacex_3, Spacey_2, cameraFeed);
				imshow(windowName4, output);

#pragma endregion	
			}

#pragma region screen 

			//Drawing rectangle shape: 
			addColor(cameraFeed);

			//show frames
			imshow(windowName2, threshold);
			imshow(windowName, cameraFeed);

			moveWindow("threshold", 10, 70);

			//image will not appear without this waitKey() command
			waitKey(1);

#pragma endregion 
		}

		return 0; 
	}
