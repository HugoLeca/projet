#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <sensors/VL53L0X/VL53L0X.h>
#include <sensors/proximity.h>
#include <motors.h>

#include <p_regulator.h>
#include <robot_management.h>
#include <process_image.h>
#include <play_melody.h>
#include <code_to_music.h>

//static variables that are needed in the file 
static int16_t speed_correction = 0; 
static int16_t rotation_speed = 0;
static uint16_t begin_move = 0;
static uint16_t end_move = 0; 
static uint16_t bar_code = 0; 
static bool start_reading_code = false; 
static bool good_position = false; 
static bool code_detected = false; 


//final code tab of size MELODY_SIZE_MAX
static uint16_t final_code[MELODY_SIZE_MAX] = {0};
//code_indice_cnt correspond to the indice of final code to fill
//it is post incremented everytime a code is filled
static uint8_t code_indice_cnt;


int16_t speed_correction_function(void) {
    
    int proximity_zero = 0; 
    int proximity_one = 0;
    int proximity_two = 0;
    int proximity_five = 0;
    int proximity_six = 0;
    proximity_zero = get_calibrated_prox(0); 
    proximity_one = get_calibrated_prox(1); 
    proximity_two = get_calibrated_prox(2); 
    proximity_five = get_calibrated_prox(5);
    proximity_six = get_calibrated_prox(6); 


    // if the wall is to close turn left or turn right to avoid it 
    if (proximity_two > WALL_THRESHOLD || proximity_one > 200 || proximity_zero > 150) {
        // turn left
        speed_correction = ROTATION_COEFF;
        return speed_correction;
    } else if (proximity_five > WALL_THRESHOLD || proximity_six > 200) {
        // turn right
        speed_correction = - ROTATION_COEFF;
        return speed_correction;
    } else {
        // the position is good
        speed_correction = 0;
        return speed_correction;
    } 
}

int16_t rotation_speed_bar_code(void){

    int proximity_one = 0;

    proximity_one = get_calibrated_prox(1);

    // read a beginning and an end and turn left or turn right until good values are detected
    if(begin_move > 0 && end_move > 400) {
        // the position is fine
        rotation_speed = 0;
        good_position = true;
        return rotation_speed;
    } else if (begin_move == 0 && end_move > 400) {
        // turn right 
        rotation_speed = -SPEED_BAR_CODE;
        good_position = false; 
        return rotation_speed;
    } else if (end_move == 0  && begin_move > 0) {
        //turn left
        rotation_speed = SPEED_BAR_CODE;
        good_position = false; 
        return rotation_speed;
    } else {
        // if there is no begin or end use the proximity sensor to be in front of the code 
        if(proximity_one > 120) {
            //turn left
            rotation_speed = SPEED_BAR_CODE;
            good_position = false;
            return rotation_speed; 
        } else {
            // turn right 
            rotation_speed = -SPEED_BAR_CODE;
            good_position = false; 
            return rotation_speed;
        }   
    }
}


void extract_bar_code(void) {

    //code is send only if the hash is respected and if the code is recognized
    // with sufficient repetability. i.e bit0(code) = 1, bit15(code) = 1, and compteur = 10.
    uint8_t counter = 0;
    uint16_t temp_code = 0;
    uint16_t mask_bit15 = 0b1000000000000000;
    uint16_t mask_bit0  = 0b0000000000000001;
    bool fill_code = false; 
    uint16_t code = 0;

    while(start_reading_code == true && code_detected == false){
            
        code = get_code();

        left_motor_set_speed(0);
        right_motor_set_speed(0);
            
        if((code & mask_bit15) && (code & mask_bit0)){
            if(code == temp_code){
                counter++;
                if(counter == 50){
                    fill_code = true;
                    break;
                }
            } else {
                temp_code = code;
                counter = 1;
            }               
        }

    }

    if(fill_code){
        bar_code = code;
        chprintf((BaseSequentialStream *)&SD3, "code=%i\n\r", bar_code);

        playNote(BIP_FREQU, BIP_DURATION);
        code_detected = true;

        final_code[code_indice_cnt] = bar_code;
        code_indice_cnt++;

        //enters an infinite loop ("SINGING") if the detected code correspond to the end pattern 0b1000000000000001
        uint8_t null_rank = 0;
        for(uint8_t i = 0; i < MELODY_SIZE_MAX ; i++){
            if(final_code[i] == 0){
                null_rank = i;
                break;
            }
        }

        if(final_code[null_rank - 1] == 0b1000000000000001){


            uint16_t project_notes[MELODY_SIZE_MAX] = {0};
            uint16_t project_durations[MELODY_SIZE_MAX] = {0};

            uint16_t* notes_ptr = project_notes;
            uint16_t* durations_ptr = project_durations;

            get_melody_from_code(notes_ptr, final_code, null_rank);
            get_durations_from_code(durations_ptr, final_code, null_rank);


            uint8_t notes_null_rank = 0;
            for(uint8_t i = 0; i < MELODY_SIZE_MAX ; i++){
                if(project_notes[i] == 0){
                    notes_null_rank = i;
                    break;
                }
            }

            //enters an infinite loop : SINGING

            while(1){
                for(uint8_t ThisNote = 0 ; ThisNote < notes_null_rank ; ThisNote++){

                    if(project_notes[ThisNote] == PAUSE){
                        __asm__ volatile ("nop");
                    } else{
                        playNote(project_notes[ThisNote], project_durations[ThisNote]);
                    }
                    uint16_t pauseBetweenNotes = 50;
                    chThdSleepMilliseconds(pauseBetweenNotes);
                }
            }
        }
    }
}

void rotate(int speed_r, int speed_l) {

    int speed_r_step_s,speed_l_step_s;

    //transform the speed from cm/s into step/s
    speed_r_step_s = -speed_r * NSTEP_ONE_TURN / WHEEL_PERIMETER;
    speed_l_step_s = speed_l * NSTEP_ONE_TURN / WHEEL_PERIMETER;

    left_motor_set_speed(speed_l_step_s);
    right_motor_set_speed(speed_r_step_s); 
    chThdSleepMilliseconds(1500); 
    left_motor_set_speed(0);
    right_motor_set_speed(0); 

}


static THD_WORKING_AREA(waRobotManagementThd, 2048);
static THD_FUNCTION(RobotManagementThd, arg) {

	chRegSetThreadName("RobotManagement Thd");
	(void)arg;


    uint8_t state = 0;
    int16_t speed = 0; ;  
    uint16_t distance = 0;


    /* Reader thread loop.*/
    while (1) {

        distance = VL53L0X_get_dist_mm();

        if(distance != 0) {


            switch(state){
                case 0:
                    // the robot moves straight and avoid the walls until it reaches a distance close to the bar code
                    distance = VL53L0X_get_dist_mm();
                    speed_correction = speed_correction_function();
                    left_motor_set_speed(MOTOR_SPEED - speed_correction); 
                    right_motor_set_speed(MOTOR_SPEED + speed_correction); 
                    if (distance < DISTANCE_MIN)
                        state = 1;
                    break;

                case 1: 
                    // the speed of the robot is defined by the regulator so that the robot stops at GOAL_DISTANCE of the bar code
                	distance = VL53L0X_get_dist_mm();
                	speed = p_regulator(distance, GOAL_DISTANCE);
                	left_motor_set_speed(speed);
                	right_motor_set_speed(speed);
                    if (distance <= GOAL_DISTANCE) 
                        state = 2;
                    break;

                case 2: 
                    // the robot rotates until in front of the bar code 
                    begin_move = get_public_begin_move();
                    end_move = get_public_end_move(); 
                    rotation_speed = rotation_speed_bar_code();
                    left_motor_set_speed(-rotation_speed);
                    right_motor_set_speed(rotation_speed);
                        
                    if(good_position == true) {
                        state = 3;
                        start_reading_code = true; 
                        good_position = false; 
                    }
                    break;
                
                case 3:
                    // extract the bar code
                    extract_bar_code();
                    if (code_detected == true) {
                        code_detected = false; 
                        start_reading_code = false; 
                        state = 4;   
                    }
                    break;

                case 4:
                    // turns and avoid walls 
                    rotate(-8,2);
                    state = 0;  
                    break;

            }
        }

    }
}

bool get_start_reading_code(void) {
    return start_reading_code; 
}

void robot_management_start(void){
	chThdCreateStatic(waRobotManagementThd, sizeof(waRobotManagementThd), NORMALPRIO, RobotManagementThd, NULL);
}
