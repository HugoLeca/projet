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



// ToF deja configuré => si on veut changer le mode (continous/single/continuous timed) ou l'accuracy
// (default mode/high accuracy/long range/high spped) aller dans VL53L0X.c

// VL53L0X_start(); //si le ToF est déjà configuré : ne fait rien - sinon intialise la communication et configure le ToF dans une thread
// VL53L0X_stop(); // stop la thread
// VL53L0X_get_dist_mm(); // retourne la distance en mm

//proximity sensors : get_calibrated_prox => à la lumiere ambiante prox = 0; plus on approche un objet plus la valeur augmente (jusqu'à > 1000)

//static uint16_t counter = 0;



// void good_rotation(void){ 

// 	if(get_calibrated_prox(0) >= get_calibrated_prox(4)) {
// 		//turn left
// 		left_motor_set_speed(-MOTOR_SPEED);
// 	    right_motor_set_speed(MOTOR_SPEED);

// 	    //check the right rotation angle
// 		if ((get_calibrated_prox(2) >= get_calibrated_prox(0)) && (get_calibrated_prox(4) >= get_calibrated_prox(5))) {
// 			counter = counter + 1;
// 		}
// 	} else if (get_calibrated_prox(0) < get_calibrated_prox(4)) {
// 		//turn right
// 		left_motor_set_speed(MOTOR_SPEED);
// 	    right_motor_set_speed(-MOTOR_SPEED);

// 	    //check the right rotation angle
// 		if ((get_calibrated_prox(2) >= get_calibrated_prox(0)) && (get_calibrated_prox(4) >= get_calibrated_prox(0))) {
// 			counter = counter + 1; 
// 		}		
// 	} 

// }  

// int16_t speed_correction(void) {
// 	    int16_t speed_correction = 0;
// 	    speed_correction = (get_public_begin() - get_public_end())/2 - (IMAGE_BUFFER_SIZE - 5)/2;
// 	    return speed_correction;
// }

uint16_t mean_distance_tof(void) {
    uint16_t distance_tof = 0;
    uint16_t counter = 0;
    uint16_t max_counter = 0;
    uint16_t tab[TAB_SIZE];

    for (uint8_t i = 0; i < TAB_SIZE; ++i)
    {
        tab[i] = VL53L0X_get_dist_mm();
    }

    for (uint8_t i = 0; i < TAB_SIZE; ++i) {
        for(uint8_t j = 0; j < TAB_SIZE; ++j) {
            if((tab[i] - tab[j]) <= TOF_THRESHOLD || (tab[i] - tab[j]) >= -TOF_THRESHOLD) {
                counter = counter + 1;
                if (counter >= max_counter) {
                    max_counter = counter;
                    distance_tof = tab[i];
                }
            }

        }
          
    }

    return distance_tof;
}

int16_t speed_correction(void) {
    
    int16_t speed_correction = 0;

    if (get_calibrated_prox(2) > WALL_THRESHOLD || get_calibrated_prox(1) > 200 || get_calibrated_prox(0) > 100) {
        speed_correction = ROTATION_COEFF;
        return speed_correction;
    } else if (get_calibrated_prox(5) > WALL_THRESHOLD || get_calibrated_prox(6) > 200) {
         speed_correction = -ROTATION_COEFF;
         return speed_correction;
    } else {
            speed_correction = 0;
            return speed_correction;
    } 
}

static THD_WORKING_AREA(waRobotManagementThd, 512);
static THD_FUNCTION(RobotManagementThd, arg) {

	chRegSetThreadName("RobotManagement Thd");
	(void)arg;

	int16_t speed = 0;
	uint8_t robot_case = 0;  

    /* Reader thread loop.*/
    while (1) {
        

        // chprintf((BaseSequentialStream *)&SD3, "prox(0)␣=␣%d␣prox(1)␣=␣%d␣prox(2)␣=%d␣prox(3)␣=%d␣prox(4)␣=%d␣prox(5)␣=%d␣prox(6)␣=%d␣prox(7)␣=%d\r\n", 
        //     get_calibrated_prox(0), get_calibrated_prox(1), get_calibrated_prox(2), get_calibrated_prox(3), get_calibrated_prox(4), 
        //      get_calibrated_prox(5), get_calibrated_prox(6), get_calibrated_prox(7));

        left_motor_set_speed(MOTOR_SPEED - speed_correction());
        right_motor_set_speed(MOTOR_SPEED + speed_correction());

        chprintf((BaseSequentialStream *)&SD3, "prox(0)␣=␣%d␣prox(7)␣=␣%d␣distance␣=␣%d\r\n", get_calibrated_prox(0), get_calibrated_prox(7), VL53L0X_get_dist_mm());


    	switch (robot_case) {

    		// case 0 :
    		// 	if (counter > 300) {
    		// 		counter = 0;
            //      good_position = 1;
    		// 		robot_case = 1;
    		// 		break;
    		// 	} else {
    		// 		good_rotation();
            //      good_position = 0;
    		// 		robot_case = 0;
    		// 		//chprintf((BaseSequentialStream *)&SD3, "counter␣=␣%d\r\n", counter);
    		// 		break;
    		// 	}

    		case 0 :
                if (mean_distance_tof() < 200) {
                    //chprintf((BaseSequentialStream *)&SD3, "count␣=␣%d\r\n", counter_straight_line);
                    left_motor_set_speed(MOTOR_SPEED - speed_correction());
                    right_motor_set_speed(MOTOR_SPEED + speed_correction());
                    robot_case = 0;
                    break;
                } else {
                    robot_case = 1;
                    break;
                } 

            case 1 :         
    			if ((VL53L0X_get_dist_mm() - GOAL_DISTANCE) > ERROR_THRESHOLD) {
    				speed = pi_regulator(VL53L0X_get_dist_mm(), GOAL_DISTANCE);
                    left_motor_set_speed(speed - speed_correction()); 
    				right_motor_set_speed(speed + speed_correction());
					//chprintf((BaseSequentialStream *)&SD3, "counter␣=␣%d␣speed␣=␣%d␣robot_case␣=%d\r\n", counter, speed, robot_case); 
					robot_case = 1;
					break;
    			} else {
    				right_motor_set_speed(0);
    				left_motor_set_speed(0);
    				chThdSleepMilliseconds(1000);
                    robot_case = 0;
    				break; 
    			}

    		default : 
    			left_motor_set_speed(0);
    			right_motor_set_speed(0);
    			break;

    	}
    }
    	
}


void robot_management_start(void){
	chThdCreateStatic(waRobotManagementThd, sizeof(waRobotManagementThd), NORMALPRIO, RobotManagementThd, NULL);
}






