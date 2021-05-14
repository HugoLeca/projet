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

//contitional variables
static MUTEX_DECL(process_image_lock);
static CONDVAR_DECL(process_image_condvar);

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

    if (VL53L0X_get_dist_mm() >= 80 && state == 0) {
        left_motor_set_speed(MOTOR_SPEED - speed_correction()); 
        right_motor_set_speed(MOTOR_SPEED + speed_correction()); 
        //chprintf((BaseSequentialStream *)&SD3, "straight_line far distance = %d\n\r", VL53L0X_get_dist_mm());
        state = 0; 
    } else {
        while (VL53L0X_get_dist_mm() >= 80 && counter_straight_line < 200 && state == 0){
            counter_straight_line++;
        }
            
        state = 1;
        //chprintf((BaseSequentialStream *)&SD3, "from straight line to regulator\n\r");
    }
}

void speed_regulator(void) {

    uint16_t counter_too_far = 0;
    uint16_t counter_stay_still = 0;
    int16_t speed = 0;

    
    speed = pi_regulator(VL53L0X_get_dist_mm(), GOAL_DISTANCE);

    // while (VL53L0X_get_dist_mm() - GOAL_DISTANCE > ERROR_THRESHOLD && VL53L0X_get_dist_mm() < 80 && counter_too_far < 200 && state == 1) {
    //     left_motor_set_speed(speed*0.6 - 0.5*speed_correction_regulator());
    //     right_motor_set_speed(speed*0.6 + 0.5*speed_correction_regulator()); 
    //     counter_too_far++; 
    //     chprintf((BaseSequentialStream *)&SD3, "trop loin\n\r");
    // }
           
    while ((VL53L0X_get_dist_mm() - GOAL_DISTANCE) <= ERROR_THRESHOLD && counter_stay_still <= 100 && state == 1) {
        left_motor_set_speed(-0.5*speed_correction_regulator());
        right_motor_set_speed(0.5*speed_correction_regulator());
        counter_stay_still++;
        //chprintf((BaseSequentialStream *)&SD3, "se positionne counter_stay_still=%d VL53L0X_get_dist_mm = %d\n\r", counter_stay_still, VL53L0X_get_dist_mm());
    }


    start_reading_code = true;


    //wake le thread ProcessImage
    chCondBroadcast(&process_image_condvar);




    

    
    start_reading_code = false; 
    corner();

/*    if (VL53L0X_get_dist_mm() - GOAL_DISTANCE <= ERROR_THRESHOLD  && state == 1) {
        start_reading_code = true; 
        chprintf((BaseSequentialStream *)&SD3, "start_reading_code\n\r");
    } else {
        start_reading_code = false; 
    }*/
    /*
    if (get_code_detected() == false && start_reading_code == true && state == 1) {
        chprintf((BaseSequentialStream *)&SD3, "code = %i\n\r", get_bar_code());
    } 
*/
    /*if (get_code_detected() == true && state == 1) {
        start_reading_code = false; 
        chprintf((BaseSequentialStream *)&SD3, "good code\n\r");
        state = 2; 
        corner();
    }*/
    //chprintf((BaseSequentialStream *)&SD3, "from regulator to corner\n\r");         

}

void corner(void) {

    uint16_t counter_corner = 0;  

    while (counter_corner < 1500 && state == 2) {
        //chprintf((BaseSequentialStream *)&SD3, "counter_corner = %d\n\r" ,counter_corner);
        left_motor_set_speed(MOTOR_SPEED - speed_correction());
        right_motor_set_speed(MOTOR_SPEED + speed_correction());
        counter_corner++; 
    } 

    state = 0; 

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


mutex_t* get_processImage_lock(void){
    return &process_image_lock;
}

condition_variable_t* get_processImage_condvar(void){
    return &process_image_condvar;
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
	chThdCreateStatic(waRobotManagementThd, sizeof(waRobotManagementThd), NORMALPRIO, RobotManagementThd, NULL);
}
