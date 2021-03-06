/*
	NAME : main.h
	AUTHOR : HUGO LECA and MARGUERITE FAUROUX
	LAST MODIFICATION : 16/05/2021
*/
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
#define BAR_CODE_SIZE 			16
#define IMAGE_BUFFER_SIZE		640
#define WIDTH_SLOPE				5
#define PAUSE					0
#define AVERAGE_DIFF			10
#define STEP_COEF				3

//constants for the robot's mouvement
#define ROTATION_COEFF			450 
#define SPEED_BAR_CODE		    70
#define BEGINNING_THRESHOLD     150
#define END_THRESHOLD           490
#define GOAL_DISTANCE 			65
#define ERROR_THRESHOLD			4.0f	
#define KP						15.0f
#define MOTOR_SPEED 			450
#define WALL_THRESHOLD_ONE		1200 // three different values for the different proximity sensors 
#define WALL_THRESHOLD_TWO      200
#define WALL_THRESHOLD_THREE    150
#define WHEEL_PERIMETER 		13 
#define NSTEP_ONE_TURN          1000
#define DISTANCE_MIN            130


/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;

#ifdef __cplusplus
}
#endif

#endif
