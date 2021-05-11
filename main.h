#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "camera/dcmi_camera.h"
#include "msgbus/messagebus.h"
#include "parameter/parameter.h"


//constants for the differents parts of the project
#define COUNTED_ENOUGH			5
#define BAR_CODE_SIZE 			8
#define BLACK_THRESHOLD			50
#define WHITE_THRESHOLD			100
#define IMAGE_BUFFER_SIZE		640
#define WIDTH_SLOPE				5
#define MIN_BAR_CODE_WIDTH		40 //value we should test experimentaly
#define ROTATION_THRESHOLD		10
#define ROTATION_COEFF			300 
#define PXTOCM					1570.0f //experimental value
#define GOAL_DISTANCE 			60.0f
#define MAX_DISTANCE 			25.0f
#define ERROR_THRESHOLD			3.0f	
#define KP						30.0f
#define KI 						2.5f	//must not be zero
#define MAX_SUM_ERROR 			(MOTOR_SPEED_LIMIT/KI)
#define MOTOR_SPEED 			300.0f 
#define WALL_THRESHOLD			1200.0f
#define TAB_SIZE				100
#define	TOF_THRESHOLD			50

/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;

void SendUint8ToComputer(uint8_t* data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif
