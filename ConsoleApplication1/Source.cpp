// in visual windows, make sure usb connected. 
//bound rate£½9600
//if can not work: properties-> configuration -> general -> character set -> multi-byte

#include <stdio.h>
#include <tchar.h>
#include "SerialClass.h"	// Library described above
#include <string>
#include <iostream>
#include <Windows.h>

using namespace std;

int main() {
	Serial* port = new Serial("COM4");    //change the serial port,
	//for bluetooth, if you have 2 com port, choose first one

	if (port->IsConnected()) cout << "Connection established!!!" << endl;

	char data[8] = "";
	char command[2] = "";
	int datalength = 8;  //length of the data,
	int readResult = 0;
	int n;
	char input;

	for (int i = 0; i < 8; ++i) { data[i] = 0; } //initial the data array

	while (1){

		input = '3';      // the key is this should be a char!!!!

		// std::cout << "Enter your command: ";
		command[0] = input;
		//std::cin.get(command, 2);// if machine can use char command[]="1";
		//std::cin.clear(); // to reset the stream state
		//std::cin.ignore(INT_MAX, '\n'); // to read and ignore all characters except 'EOF' 
		int msglen = strlen(command);
		if (port->WriteData(command, msglen));  //write to arduino
		printf("Wrtiting Success\n");





		//delay
		Sleep(1000);

		n = port->ReadData(data, 8);
		if (n != -1){
			data[n] = 0;
			cout << "arduino: " << data << endl;
		}

	}

	system("pause");
	return 0;
}
