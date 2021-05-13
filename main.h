#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "camera/dcmi_camera.h"
#include "msgbus/messagebus.h"
#include "parameter/parameter.h"


//constants for the differents parts of the project
#define BIP_FREQU				4186
#define BIP_DURATION			50
#define MELODY_SIZE_MAX			20
#define COUNTED_ENOUGH			5
#define BAR_CODE_SIZE 			16
#define BLACK_THRESHOLD			50
#define WHITE_THRESHOLD			100
#define IMAGE_BUFFER_SIZE		640
#define WIDTH_SLOPE				5
#define MIN_BAR_CODE_WIDTH		40 //value we should test experimentaly
#define ROTATION_THRESHOLD		10
#define ROTATION_COEFF			2 
#define PXTOCM					1570.0f //experimental value
#define GOAL_DISTANCE 			100.0f
#define MAX_DISTANCE 			25.0f
#define ERROR_THRESHOLD			5.0f	
#define KP						60.0f
#define KI 						2.5f	//must not be zero
#define MAX_SUM_ERROR 			(MOTOR_SPEED_LIMIT/KI)
#define DISTANCE_THRESHOLD      200.0f 
#define PROX_MAX				200.0f
#define PROX_THRESHOLD			20.0f
#define MOTOR_SPEED 			90.0f 


/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;

void SendUint8ToComputer(uint8_t* data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif
