#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <sensors/VL53L0X/VL53L0X.h>
#include <sensors/proximity.h>
#include <motors.h>

#include <pi_regulator.h>
#include <robot_management.h>
#include <process_image.h>

static uint8_t state = 0;
static bool start_reading_code = false; 


// void good_rotation(void){ 

// 	if(get_calibrated_prox(0) >= get_calibrated_prox(4)) {
// 		//turn left
// 		left_motor_set_speed(-MOTOR_SPEED);
// 	    right_motor_set_speed(MOTOR_SPEED);

// 	    //check the right rotation angle
// 		if ((get_calibrated_prox(2) > get_calibrated_prox(0)) && (get_calibrated_prox(4) > get_calibrated_prox(5))) {
// 			counter_rotation = counter_rotation + 1;
// 		}
// 	} else if (get_calibrated_prox(0) < get_calibrated_prox(4)) {
// 		//turn right
// 		left_motor_set_speed(MOTOR_SPEED);
// 	    right_motor_set_speed(-MOTOR_SPEED);

// 	    //check the right rotation angle
// 		if ((get_calibrated_prox(2) > get_calibrated_prox(0)) && (get_calibrated_prox(4) > get_calibrated_prox(0))) {
// 			counter_rotation = counter_rotation + 1; 
// 		}		
// 	} 

// }  


int16_t speed_correction(void) {
    
    int16_t speed_correction = 0;

    if (get_calibrated_prox(2) > WALL_THRESHOLD || get_calibrated_prox(1) > 200 || get_calibrated_prox(0) > 150) {
        // turn left
        speed_correction = ROTATION_COEFF;
        return speed_correction;
    } else if (get_calibrated_prox(5) > WALL_THRESHOLD || get_calibrated_prox(6) > 200) {
        // turn right
        speed_correction = - ROTATION_COEFF;
        return speed_correction;
    } else {
        speed_correction = 0;
        return speed_correction;
    } 
}

void straight_line(void) {

    uint16_t counter_straight_line = 0;
    state = 0; 

    if (VL53L0X_get_dist_mm() >= 90 && state == 0) {
        left_motor_set_speed(MOTOR_SPEED - speed_correction()); 
        right_motor_set_speed(MOTOR_SPEED + speed_correction()); 
        state = 0; 
    } else {
        while (VL53L0X_get_dist_mm() >= 90 && counter_straight_line < 100 && state == 0){
            counter_straight_line++;
            chprintf((BaseSequentialStream *)&SD3, "straight_line\n\r");
        }  
        state = 1;
    }
}

void speed_regulator(void) {

    uint16_t counter_too_far = 0;
    uint16_t counter_stay_still = 0;
    int16_t speed = 0;
    speed = pi_regulator(VL53L0X_get_dist_mm(), GOAL_DISTANCE);
    uint16_t counter_corner = 0;

    while (VL53L0X_get_dist_mm() - GOAL_DISTANCE >= ERROR_THRESHOLD && VL53L0X_get_dist_mm() <= 90 && counter_too_far < 20 && state == 1) {
        left_motor_set_speed(speed*0.6 - 0.5*speed_correction_regulator());
        right_motor_set_speed(speed*0.6 + 0.5*speed_correction_regulator()); 
        counter_too_far++; 
    }
           
    while ((VL53L0X_get_dist_mm() - GOAL_DISTANCE) <= ERROR_THRESHOLD && counter_stay_still <= 300 && state == 1) {
        left_motor_set_speed(-0.5*speed_correction_regulator());
        right_motor_set_speed(0.5*speed_correction_regulator());
        counter_stay_still++;
    }

    if ((VL53L0X_get_dist_mm() - GOAL_DISTANCE) <= ERROR_THRESHOLD && counter_stay_still >= 300 && state == 1) {
        left_motor_set_speed(0);
        right_motor_set_speed(0);
        start_reading_code = true; 
    }

    if (get_code_detected() == false && start_reading_code == true && state == 1) {
        start_reading_code = true; 
        chprintf((BaseSequentialStream *)&SD3, "not good code\n\r");
        state = 1; 
    }

    if (get_code_detected() == true && start_reading_code == true && state == 1) {
        start_reading_code = false; 
        chprintf((BaseSequentialStream *)&SD3, "good code\n\r");
        state = 2; 
    }

    while (counter_corner <= 500 && state == 2) {
        left_motor_set_speed(MOTOR_SPEED - speed_correction());
        right_motor_set_speed(MOTOR_SPEED + speed_correction());
        counter_corner++; 
        chprintf((BaseSequentialStream *)&SD3, "corner\n\r");
    } 

    if (counter_corner >= 500 && state == 2) {
        state = 0;
    }
    
}


int16_t speed_correction_regulator(void){
    int16_t speed_correction_regulator = 0;
    uint8_t good_position = 0;
    
    if(get_public_begin_move() > 80 && get_public_end_move() > 400 && good_position == 0) {
        speed_correction_regulator = 0;
        good_position = 1;
        return speed_correction_regulator;
    } else if (get_public_begin_move() == 0 && get_public_end_move() > 500 && good_position == 0) {
        // turn right 
        speed_correction_regulator = -ROTATION_COEFF;
        return speed_correction_regulator;
    } else if (get_public_end_move() == 0  && get_public_begin_move() > 0 && good_position == 0) {
        //turn left
        speed_correction_regulator = ROTATION_COEFF;
        return speed_correction_regulator;
    } else {
        speed_correction_regulator = 0;
        return speed_correction_regulator; 
    }

}


static THD_WORKING_AREA(waRobotManagementThd, 1024);
static THD_FUNCTION(RobotManagementThd, arg) {

	chRegSetThreadName("RobotManagement Thd");
	(void)arg;

    state = 0; 

    /* Reader thread loop.*/
    while (1) {

        straight_line(); 

        speed_regulator(); 

        
    }
    	
}

bool get_start_reading_code(void) {
    return start_reading_code; 
}


void robot_management_start(void){
	chThdCreateStatic(waRobotManagementThd, sizeof(waRobotManagementThd), NORMALPRIO + 10, RobotManagementThd, NULL);
}
