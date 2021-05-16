/*
	NAME : code_to_music.c
	AUTHOR : HUGO LECA
	LAST MODIFICATION : 16/05/2021
*/

#ifndef CODE_TO_MUSIC_H
#define CODE_TO_MUSIC_H


/*
	returns the frequency in Hz to play according to the code
	previously read
*/
uint16_t code_to_frequency(uint16_t note);


/*
	returns the durations in milliseconds to play according to the
	code previously read
*/uint16_t code_to_durations(uint16_t code);


/*
	process the 16 bits barcode and exctract the right informations
	to fill the melody buffer only with notes frequencies
*/
void get_melody_from_code(uint16_t* buffer_melody, uint16_t* buffer_code, uint8_t null_rank);


/*
	process the 16 bits barcode and exctract the right informations
	to fill the durations buffer only with notes durations
*/
void get_durations_from_code(uint16_t* buffer_durations, uint16_t* buffer_code, uint8_t null_rank);



#endif /* CODE_TO_MUSIC_H */
