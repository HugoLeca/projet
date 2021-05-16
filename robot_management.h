/*
	NAME : process_image.h
	AUTHOR : HUGO LECA and MARGUERITE FAUROUX
	LAST MODIFICATION : 16/05/2021
*/

#ifndef ROBOT_MANAGEMENT_H_
#define ROBOT_MANAGEMENT_H_

/*starts the thread RobotManagement*/
void robot_management_start(void);
/*returns the rotation correction needed to avoid the walls */
int16_t speed_correction_function(void);
/*returns the rotation correction needed to be in front of the bar code */
void rotation_speed_bar_code(void); 
/* process the bar code, check if the code is stable, fill final code and if 
the code detected correspond to the end pattern, play the music indefinetly*/
void process_bar_code(void);
/* rotate in the corner*/
void rotate(int speed_r, int speed_l);



#endif /* ROBOT_MANAGEMENT_H_ */
