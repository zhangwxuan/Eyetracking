// this is update online at the sametime 
// still the control one 
// make the program looks simple 

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


int H_MIN = 30;
int H_MAX = 247;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 16;
int V_MAX = 256;
int DilateV = 3;
int ErodeV = 6;

//default capture width and height
const int FRAME_WIDTH = 800;   // this is the camera viode size 
const int FRAME_HEIGHT = 600;
const int Mouse_Speed_UD = 13; //mouse moving speed up and down 
const int Mouse_Speed_LR = 11; //mouse moving speed left and right 
const double SCREEN_WIDTH = 1680;   // this is the dispaly resolution 
const double SCREEN_HEIGHT = 1050;
const double X_RANGE = 25; //smaller value gives a faster change
const double Y_RANGE = 1;
const double RESOLUTION_X = 1920;
const double RESOLUTION_Y = 1080;
const double CTRL_X = 1000;
const double CTRL_Y = 200; 
bool changeY = false; //this is to set if need to track y position

// the string can be changed for control instruction 
string command1 = "LEFT";
string command2 = "UP";
string command3 = "DOWN";
string command4 = "Right";
string command5 = "Click"; 
string command6 = ""; 

//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 20;                             //make sure no noise to interupt
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
//minimum and maximum object area
const int MIN_OBJECT_AREA = 5 * 5;                        //no need
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5; //no need

//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";
const string windowName4 = "Cursor Control Panel";
bool objectFound = false;          // if is tracking the object
bool cali = true;                  // true when need to calibtaion 
bool clickOne = true;

int x_min, x_max, y_min, y_max, x_min1, x_max1, y_min1, y_max1, x_o, y_o;
int x_1, x_2, y_1, y_2;
double Spacex_1, Spacex_2, Spacex_3, Spacey_1, Spacey_2;
int x_screen, y_screen; // real coordinate in screen
int x_screen1, y_screen1; // coordinate for control panel 
int stop = 0;  // to count the time to shut down system automatically 
int userOpt; // option for operation mode 
int cursor_x = 960, cursor_y = 540;

double dx=0, dy=0;
POINT p;

void on_trackbar(int, void*)
{//This function gets called whenever a
	// trackbar position is changed
}

string intToString(int number) {

	std::stringstream ss;
	ss << number;
	return ss.str();
}

//functions: 
//to create the control HSV bar
void createTrackbars() {
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
	sprintf_s(TrackbarName, "Dilate", DilateV);
	sprintf_s(TrackbarName, "Erode", ErodeV);

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
	createTrackbar("Dilate", trackbarWindowName, &DilateV, 10, on_trackbar);
	createTrackbar("Erode", trackbarWindowName, &ErodeV, 10, on_trackbar);

}

//to draw ROI 
void drawObject(int x, int y, Mat &frame) {

	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!

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
void morphOps(Mat &thresh) {
	/*
	Erode and Dialate:
	para1: shape: MORPH_RECT MORPH_CROSS  MORPH_ELLIPSE

	*/

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_ELLIPSE, Size(ErodeV, ErodeV));
	//dilate with larger element so make sure object is nicely visible                      
	Mat dilateElement = getStructuringElement(MORPH_ELLIPSE, Size(DilateV, DilateV));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);

	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);

}

//do the tracking things 
void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed) {

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
		if (numObjects<MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea) {                    //MAKE SURE IS TRACKING CORRECTLY
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
					refArea = area;
					if (x >= 1280 || y >= 720)                                                       //need to change according to camera size 
						objectFound = false;
				}
				else objectFound = false;

			}
			//let user know you found an object
			if (objectFound == true) {
				putText(cameraFeed, "Tracking Mode", Point(0, 50), 2, 1, Scalar(0, 0, 255), 2);
				//draw object location on screen
				drawObject(x, y, cameraFeed);
			}
		}
		else putText(cameraFeed, "PLEASE ADJUST FILTER", Point(0, 50), 2, 1, Scalar(0, 0, 255), 2);
	}
}

// this function is to create a green window for main panel 
void createPanel(Mat &panel, int height, int weight) {
	rectangle(panel, Point(0, 0), Point(height, weight), Scalar(26, 255, 40), CV_FILLED, 8);
}

int control(int x, int y) { //      need to go back and check!!!!
	if (y < 500){
		if (x > 0 && x < 210)
		{
			return 1;
		}
		else if (x > 0 && x < 420)
		{
			return 2;
		}
		else if (x > 0 && x < 580)
		{
			return 3;
		}
		else if (x > 0 && x < 810)
		{
			return 4;
		}
		else if (x > 0 && x < 1000)
		{
			return 5;
		}
		else
			return 0;
	}
	 else
	   	return 0;
}

// doing eye stare  calibration 
void calibration(int x, int y, Mat &cameraFeed, double duration) {
	putText(cameraFeed, "Calibration:", Point(0, 90), 2, 1, Scalar(0, 0, 255), 2);

	if (duration < 2)
	{
		putText(cameraFeed, "Doing Calibration:", Point(0, 110), FONT_HERSHEY_PLAIN, 1.5, Scalar(0, 0, 255), 2);

	}
	else if (duration < 4) 
	{
		x_o = x;
		y_o = y;
	}
	else if (duration < 5 && objectFound == true)
	{
		dx = SCREEN_WIDTH / 2 - x_screen;
		dy = SCREEN_HEIGHT / 2 - y_screen;
		x_screen = (x - x_o) / X_RANGE*(RESOLUTION_X / 2) + SCREEN_WIDTH / 2 + dx;
		y_screen = (y - y_o) / Y_RANGE*(RESOLUTION_Y / 2) + SCREEN_HEIGHT / 2 + dy;		


	}
}

//screensize is used to calculate the x,y coordinate in real screen display 
void screensize(int & x_screen, int & y_screen, int x, int y){
	x_screen = (x - x_o) / X_RANGE*(RESOLUTION_X / 2) + SCREEN_WIDTH / 2 + dx;
	y_screen = (y - y_o) / X_RANGE*(RESOLUTION_Y / 2) + SCREEN_HEIGHT / 2 + dy;
}

//control box program is used to divide the control panel into num region 
void controlbox(Mat &output, int num){	
	rectangle(output, Point(0, 0), Point(CTRL_X, CTRL_Y), Scalar(0, 0, 0), CV_FILLED, 8);
	for (int n= 0; n < num;n++ ){
		line(output, Point(n*CTRL_X / num, 0), Point(n*CTRL_X / num, CTRL_Y), Scalar(0,0,255));
	}

	putText(output, command1, Point(70, 90), 2, 1, Scalar(0, 0, 255), 2);
	putText(output, command2, Point(270, 90), 2, 1, Scalar(0, 0, 255), 2);
	putText(output, command3, Point(470, 90), 2, 1, Scalar(0, 0, 255), 2);
	putText(output, command4, Point(670, 90), 2, 1, Scalar(0, 0, 255), 2);
	putText(output, command5, Point(870, 90), 2, 1, Scalar(0, 0, 255), 2);
}

//this function is used to control cursor 
void cursor(int input) {
	GetCursorPos(&p);
	cursor_x = p.x;
	cursor_y = p.y;
	switch (input) {
	case 1:  //key left
		cursor_x = cursor_x - Mouse_Speed_LR;
		SetCursorPos(cursor_x, cursor_y);
		break;
	case 2: // key up
		cursor_y = cursor_y - Mouse_Speed_UD;
		SetCursorPos(cursor_x, cursor_y);
		break;
	case 3:  // key down
		cursor_y = cursor_y + Mouse_Speed_UD;
		SetCursorPos(cursor_x, cursor_y);
		break;
	case 4:  // key right
		cursor_x = cursor_x + Mouse_Speed_LR;
		SetCursorPos(cursor_x, cursor_y);
		break;
	case 5:
		mouse_event(MOUSEEVENTF_LEFTDOWN, cursor_x, cursor_y, 0, 0); // moving left click
		mouse_event(MOUSEEVENTF_LEFTUP, cursor_x, cursor_y, 0, 0); // moving cursor leftup //for accessing your required co-ordinate
		break;

	case 6:
		mouse_event(MOUSEEVENTF_RIGHTDOWN, cursor_x, cursor_y, 0, 0); // cursor right click
		mouse_event(MOUSEEVENTF_RIGHTUP, cursor_x, cursor_y, 0, 0); 
		break;

	default: // not arrow
		break;
	}
	Sleep(20);


}
// showing comnnad content for control panel 

int main(int argc, char* argv[])
{
	//first to initial varibales 

#pragma region variables
	//some boolean variables for different functionality within this
	bool trackObjects = true;     //if need to track objects 
	bool useMorphOps = true;      // tracking shadows 
	//bool isTracking = false;    
	bool start_time = true;       //calibration time function, true when calibration starts 
	bool start_time2 = true;      //this is for selection output 
	bool startControl = false;    // if start to control the arduino, true when calibration finish, false when track lost 
	bool ArduinoConnect = false;  //state if arduino is connected to computer 
	bool flipI = true;            // if need to flip the camera 
	bool move = true;                    //to move for once only 
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
	double duration, duration2;
	clock_t start, startx;
	int timex;
	//is for counitng time display
	int input=0; // control input 
	int input1=0; // for compare with previous input 
	char calibrationOption;
	bool Option1 = true;
	string command; // for user input 
#pragma endregion

#pragma region console_display

	// to select the opertation mode: 
	cout << "Please select the mode " << endl;
	cout << "1. Control arduino" << endl;
	cout << "2. Control mouse cursur " << endl;
	cout << "please enter the number: ..." << endl;
	cin >> userOpt;

	cout << "loading..." << endl << endl;

	VideoCapture capture;
	//open capture object at location zero (default location for webcam)
	capture.open(0);
	//set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
#pragma endregion 
	cout << "you may enter your instruction "; 


	while (1) {

#pragma region basic tracking function 

		//store image to matrix and flip the image
		capture.read(cameraFeed);

		// to flip over image 
		if (flipI)
		{
			cameraFeed.copyTo(inverse);
			flip(inverse, cameraFeed, 1);
		}

		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
		inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);


		//MorphOps
		if (useMorphOps)
			morphOps(threshold);


		//pass in thresholded frame to our object tracking function
		//this function will return the x and y coordinates of the
		if (trackObjects)
			trackFilteredObject(x, y, threshold, cameraFeed);

#pragma endregion 
		//for option 1: 
		//control and send to arduino 


		if (userOpt == 1) {
#pragma region option1_Control



			Mat ctrlPanel;
			ctrlPanel = Mat::zeros(Size(1000, 200), CV_8UC3);
			createPanel(ctrlPanel, 1000, 200);
			// this is a reflection on whole screen display 
			Mat wholePanel;
			wholePanel = Mat::zeros(Size(SCREEN_WIDTH, SCREEN_HEIGHT), CV_8UC3);

			screensize(x_screen, y_screen, x, y);
			circle(wholePanel, Point(x_screen, y_screen), 5, Scalar(0, 0, 255), 2);
			circle(wholePanel, Point(840, 525), 20, Scalar(0, 0, 255), 2);
			//calibration process 
#pragma region calibration

			if (cali == true && objectFound == true)
			{
				if (start_time) {
					start = clock();
					start_time = false;
				}

				duration = (double)(clock() - start) / CLOCKS_PER_SEC;

				if (duration < 5)
					calibration(x, y, wholePanel, duration);
				if (duration > 5)
					cali = false;
			}

			else if (cali == false){
				startControl = true;
			}

			else
				start = clock();
#pragma endregion 



			// this is for control panel 
			if (objectFound == true)
			{
				x_screen1 = (x - x_o) / X_RANGE*(RESOLUTION_X / 2) + 1000 / 2 + dx;
				
				if (changeY)
					y_screen1 = y_screen;
				else
					y_screen1 = 100; 

				circle(ctrlPanel, Point(x_screen1, y_screen1), 5, Scalar(0, 0, 255), 2);
			}
			//function on ctrlbox and corresponding indicate: 
			controlbox(ctrlPanel,5); 




			// this is for counting time for seletion 
			if (startControl == true)
			{

				input = control(x_screen1, y_screen1);
				rectangle(ctrlPanel, Point((input - 1) * 200, 0), Point(input * 200, 200), Scalar(156, 195, 165), CV_FILLED, 8);

				if (start_time2) {
					startx = clock();
					start_time2 = false;
				}

				duration2 = (double)(clock() - startx) / CLOCKS_PER_SEC;

				if (input == input1){

					if (input == 5 && clickOne && duration2 >1)
					{   
						cursor(input);
						rectangle(ctrlPanel, Point((input - 1) * 200, 0), Point(input * 200, 200), Scalar(232, 144, 144), CV_FILLED, 8);
						clickOne = false; 
					}
						
					else if (input != 5 && duration2 > 1)
					{
						cursor(input);
						rectangle(ctrlPanel, Point((input - 1) * 200, 0), Point(input * 200, 200), Scalar(232, 144, 144), CV_FILLED, 8);
					}
					else if (input != 5)
						clickOne = true; 
				}
				else
				{
					input1 = input;
					startx = clock();
				}
			}

			else if (startControl == false)
				input = 0; 

			imshow("wholePanel", wholePanel);
			imshow("ctrlPanel", ctrlPanel);

#pragma endregion 		
		}



#pragma region screen 

		circle(cameraFeed, Point(400, 300), 20, Scalar(0, 0, 255), 2);

		Mat colorPanel;
		colorPanel = Mat::zeros(Size(600, 600), CV_8UC3);
		createPanel(colorPanel, 600, 600);

		//show frames
		if (!trackObjects){
			imshow(windowName2, threshold);
			imshow(windowName, cameraFeed);
		}

		imshow("panel", colorPanel);
		if (move) {
			moveWindow(windowName2, 10, 70);
			moveWindow(windowName, 550, 215);
			moveWindow("panel", 1260, 220);
			moveWindow("wholePanel", 120, 0);
			moveWindow("ctrlPanel", 460, 0);
			move = false;
		}
		//image will not appear without this waitKey() command
		waitKey(1);




#pragma endregion 
	}

	return 0;
}