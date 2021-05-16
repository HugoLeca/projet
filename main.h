#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "camera/dcmi_camera.h"
#include "msgbus/messagebus.h"
#include "parameter/parameter.h"


//constants for the differents parts of the project

//constants for image processing and music
#define BIP_FREQU				4186
#define BIP_DURATION			50
#define MELODY_SIZE_MAX			20
#define COUNTED_ENOUGH			5 // a checker
#define BAR_CODE_SIZE 			16
#define IMAGE_BUFFER_SIZE		640
#define WIDTH_SLOPE				5
#define PAUSE					0
#define TAB_SIZE				100// a checker

//constants for the robot's mouvement
#define MIN_BAR_CODE_WIDTH		40 //a checker
#define ROTATION_COEFF			300 
#define SPEED_BAR_CODE		    120
#define GOAL_DISTANCE 			60.0f
#define ERROR_THRESHOLD			4.0f	
#define KP						15.0f
#define MOTOR_SPEED 			300.0f 
#define WALL_THRESHOLD			1200.0f
#define WHEEL_PERIMETER 		13 
#define NSTEP_ONE_TURN          1000
#define DISTANCE_MIN            100

/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;

#ifdef __cplusplus
}
#endif

#endif
