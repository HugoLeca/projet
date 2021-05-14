#ifndef ROBOT_MANAGEMENT_H_
#define ROBOT_MANAGEMENT_H_


void robot_management_start(void);
void good_rotation(void);
void straight_line(void);
int16_t speed_correction(void);
//uint16_t most_frequent_tof(void);
void speed_regulator(void);
int16_t speed_correction_regulator(void);
bool get_start_reading_code(void);



#endif /* ROBOT_MANAGEMENT_H_ */
