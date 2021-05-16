#ifndef ROBOT_MANAGEMENT_H_
#define ROBOT_MANAGEMENT_H_


void robot_management_start(void);
int16_t speed_correction_function(void);
//uint16_t most_frequent_tof(void);
int16_t rotation_speed_bar_code(void);
bool get_start_reading_code(void); 
void extract_bar_code(void);
void rotate(int speed_r, int speed_l);



#endif /* ROBOT_MANAGEMENT_H_ */
