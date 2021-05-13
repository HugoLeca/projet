#ifndef CODE_TO_MUSIC_H
#define CODE_TO_MUSIC_H


uint16_t code_to_frequency(uint16_t note);
uint16_t code_to_durations(uint16_t code);


void get_melody_from_code(uint16_t* buffer_melody, uint16_t* buffer_code);
void get_durations_from_code(uint16_t* buffer_durations, uint16_t* buffer_code);

//creation du thread
//
//void wait_code_bar_ready(void);




#endif /* CODE_TO_MUSIC_H */
