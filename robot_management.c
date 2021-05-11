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

static uint8_t regulator_start = 0;
static uint8_t corner = 0; 
static uint8_t move_straight = 0; 


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

// uint16_t most_frequent_tof(void) {

//     uint16_t distance_tof = 0;
//     uint32_t counter = 0;
//     uint32_t max_counter = 0;
//     uint16_t tab[TAB_SIZE];

//     for (uint16_t i = 0; i < TAB_SIZE; ++i)
//     {
//         tab[i] = VL53L0X_get_dist_mm();
//     }

//     for (uint16_t i = 0; i < TAB_SIZE; ++i) {
//         for(uint16_t j = 0; j < TAB_SIZE; ++j) {
//             if((tab[i] - tab[j]) <= TOF_THRESHOLD || (tab[i] - tab[j]) >= -TOF_THRESHOLD) {
//                 counter = counter + 1;
//                 if (counter >= max_counter) {
//                     max_counter = counter;
//                     distance_tof = tab[i];
//                 }
//             }

//         }
          
//     }

//     return distance_tof;
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

    static uint16_t counter_straight_line = 0;

    if (VL53L0X_get_dist_mm() < 80) {   
        if (counter_straight_line < 200) {
            chprintf((BaseSequentialStream *)&SD3, "counter_straight_line = %d\n\r",counter_straight_line);
            counter_straight_line = counter_straight_line + 1;
            move_straight = 1;
            regulator_start = 0;;
            corner = 0;
        } else {
            counter_straight_line = 0;
            move_straight = 0;
            regulator_start = 1;
            corner = 0;
        }
    } else {
        move_straight = 1; 
        regulator_start = 0;
        corner = 0;
    }
}

void speed_regulator(void) {

    static uint16_t counter_stay_still = 0;
    int16_t speed = 0;
    speed = pi_regulator(VL53L0X_get_dist_mm(), GOAL_DISTANCE);

    if (speed < 20) {
        if (counter_stay_still < 1500) {
            chprintf((BaseSequentialStream *)&SD3, "begin=%d end=%d distance = %d correction = %d\n\r", get_public_begin_move(), get_public_end_move(), VL53L0X_get_dist_mm(), speed_correction_regulator());
            left_motor_set_speed(0);
            right_motor_set_speed(0);  
            counter_stay_still = counter_stay_still + 1; 
            regulator_start = 1;
            move_straight = 0;
            corner = 0;            
        } else {
            counter_stay_still = 0;
            regulator_start = 0;
            corner = 1;
            move_straight = 0;
        }
    } else {
        left_motor_set_speed(speed*0.5 - speed_correction_regulator()); 
        right_motor_set_speed(speed*0.5 + speed_correction_regulator());  
        regulator_start = 1;
        move_straight = 0;
        corner = 0;
    }
}

int16_t speed_correction_regulator(void){
    int16_t speed_correction_regulator = 0;
    
    if(get_public_begin_move() > 100 && get_public_end_move() > 400) {
        speed_correction_regulator = 0;
        return speed_correction_regulator;

    } else if (get_public_begin_move() == 0 && get_public_end_move() != 0) {
        // turn left 
        speed_correction_regulator = ROTATION_COEFF;
        return speed_correction_regulator;
    } else if (get_public_end_move() == 0 && get_public_begin_move() != 0) {
        //turn right
        speed_correction_regulator = -ROTATION_COEFF;
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

    
    static uint16_t counter_corner = 0;  
    move_straight = 1; 
    int16_t speed = 0;
  

    /* Reader thread loop.*/
    while (1) {

        // speed = pi_regulator(VL53L0X_get_dist_mm(), GOAL_DISTANCE);
        // left_motor_set_speed(speed - speed_correction_regulator()); 
        // right_motor_set_speed(speed + speed_correction_regulator());

        // chprintf((BaseSequentialStream *)&SD3, "begin=%dend=%ddistance=%dcorrection=%d\n\r", get_public_begin(), get_public_end(), VL53L0X_get_dist_mm(), speed_correction_regulator());
        // chThdSleepMilliseconds(500);

        while (move_straight == 1 && regulator_start == 0 && corner == 0) {
            straight_line(); 
            left_motor_set_speed(MOTOR_SPEED - speed_correction()); 
            right_motor_set_speed(MOTOR_SPEED + speed_correction());
        } 

        while (move_straight == 0 && regulator_start == 1 && corner == 0) { 
            speed_regulator(); 
        }


        while (move_straight == 0 && regulator_start == 0 && corner == 1) {
            if (counter_corner < 1000) {
                chprintf((BaseSequentialStream *)&SD3, "counter_corner = %d\n\r",counter_corner);
                left_motor_set_speed(MOTOR_SPEED - speed_correction());
                right_motor_set_speed(MOTOR_SPEED + speed_correction());
                counter_corner = counter_corner + 1; 
                move_straight = 0;
                regulator_start = 0;
                corner = 1; 

            } else {
                counter_corner = 0;
                move_straight = 1; 
                regulator_start = 0;
                corner = 0; 
            }
        }
        
    }
    	
}


void robot_management_start(void){
	chThdCreateStatic(waRobotManagementThd, sizeof(waRobotManagementThd), NORMALPRIO, RobotManagementThd, NULL);
}
