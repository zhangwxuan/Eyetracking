# Eye tracking for disabled (FYP)

FYP project: Eye tracking for disabled people 

## Getting Started

This code is using with C++ programming language. 
Together with OPENCV library on visual studio. 

### Prerequisites

visual studio at least vision 2013 



### Instruction 

There are two options for eye tracking control, which can be chosen in the beginning console


```
enter 1 to control with arduino robot car 
enter 2 to mouse control option 
```

And in second cursor control mode,there are more options for quick tests: 

```
enter 1 to start a new calibration 
enter 2 to input calibration data
enter 3 to start a simple test with existing calibraion data 
```



### Clone project to local visual studio: 
1. Open Visual Studio: menu -> team -> manage connections 
2. In local git repositories: Clone
3. Enter url as: https://github.com/zhangwxuan/Eyetracking.git; 
4. Enter local file directory 
5. Clone and change settings (shown in the following section) 
6. Go to Solution Explorer window and run the project 

### Setting to change after installing for this project: 
```
1. Project right click - properties - general - character set - change to "not set" 
```
```
2. properties -linker - advanced - target machine - change to "x64"
```
```
3. menu bar - build - configuration manager - platfor, - change to "x64"
```
```
4. change the opencv library location
```


### Updated log: 
02/03: create github sharing and test project clone 
02/06: mouse control basic function finished 
02/07: change the mouse control panal bar and indicating area    
       stop control mouse when lose tracking objects 
02/08: cursor get postion funcstions for continuous controlling 

### to do list: 
1. testing with glasses on mouse control panel 
2. adjust the reflection color for a better tracking 
       
