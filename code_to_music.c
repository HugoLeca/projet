#include "ch.h"
#include "hal.h"

#include <play_melody.h>
#include <code_to_music.h>
#include <main.h>




uint16_t code_to_frequency(uint16_t note){

	uint16_t frequency = 0;

	switch(note){

		case 0:
			frequency = NOTE_C4;
			break;

		case 1:
			frequency = NOTE_CS4;
			break;

		case 2:
			frequency = NOTE_D4;
			break;

		case 3:
			frequency = NOTE_DS4;
			break;

		case 4:
			frequency = NOTE_E4;
			break;

		case 5:
			frequency = NOTE_F4;
			break;

		case 6:
			frequency = NOTE_FS4;
			break;

		case 7:
			frequency = NOTE_G4;
			break;

		case 8:
			frequency = NOTE_GS4;
			break;

		case 9:
			frequency = NOTE_A4;
			break;

		case 10:
			frequency = NOTE_AS4;
			break;

		case 11:
			frequency = NOTE_B4;
			break;

		case 12:
			frequency = NOTE_C5;
			break;

		case 13:
			frequency = NOTE_CS5;
			break;

		case 14:
			frequency = NOTE_D5;
			break;

		case 15:
			frequency = NOTE_DS5;
			break;

		case 16:
			frequency = NOTE_E5;
			break;

		case 17:
			frequency = NOTE_F5;
			break;

		case 18:
			frequency = NOTE_FS5;
			break;

		case 19:
			frequency = NOTE_G5;
			break;

		case 20:
			frequency = NOTE_GS5;
			break;

		case 21:
			frequency = NOTE_A5;
			break;

		case 22:
			frequency = NOTE_AS5;
			break;

		case 23:
			frequency = NOTE_B5;
			break;

		case 24:
			frequency = NOTE_C6;
			break;

		case 25:
			frequency = NOTE_CS6;
			break;

		case 26:
			frequency = NOTE_D6;
			break;

		case 27:
			frequency = NOTE_DS6;
			break;

		case 28:
			frequency = NOTE_E6;
			break;

		case 29:
			frequency = NOTE_F6;
			break;

		case 30:
			frequency = NOTE_FS6;
			break;

		case 31:
			frequency = PAUSE;
			break;
	}
	return frequency;
}

uint16_t code_to_durations(uint16_t code){

	//si tempo = 1 --> note jouee pendant 1 seconde
	//si tempo = 2 --> note jouee pendant 1/2 sec
	//etc...
	uint16_t duration = 0;
	
	switch(code){

		case 0:
			duration = 500;
			break;

		case 1:
			duration = 1000;
			break;

		case 2:
			duration = 2000;
			break;

		case 3:
			duration = 4000;
			break;
	}

	return duration;

}




static uint16_t code[] = {
	
	0b1000000000010001, 0b1001000000101001, 0b1001110001001001, 0b1010110001100001,

};



void get_durations_from_code(uint16_t* buffer){
	uint16_t mask_durations_1 = 0;
	uint16_t mask_durations_2 = 0;

	mask_durations_1 = 0b0000001100000000; //extract first durations
	mask_durations_2 = 0b0000000000000110;


	uint16_t temp_durations_1 = 0, temp_durations_2 = 0;


	uint8_t indice_durations = 0;
	uint8_t indice_code = 0;
	uint8_t stop = 0;


	while(stop == 0){

		if(indice_code == (sizeof(code)/2)){
			stop = 1;
		} else {
			temp_durations_1 = (mask_durations_1 & code[indice_code]) >> 8;
			temp_durations_2 = (mask_durations_2 & code[indice_code]) >> 1;

			if(indice_durations == 6){
				stop = 0;
			}
			buffer[indice_durations] = code_to_durations(temp_durations_1);
			indice_durations++;
			buffer[indice_durations] = code_to_durations(temp_durations_2);
			indice_durations++;

		}
		indice_code++;
	}
}


void get_melody_from_code(uint16_t* buffer){

	uint16_t mask_note_1 = 0;
	uint16_t mask_note_2 = 0;


	mask_note_1 = 0b0111110000000000; //extracts first note
	mask_note_2 = 0b0000000011111000; //extract second note 
 	//extract second durations




	uint16_t temp_note_1 = 0, temp_note_2 = 0;

	uint8_t indice_notes = 0;
	uint8_t indice_code = 0;
	uint8_t stop = 0;

	while(stop == 0){

		if(indice_code == (sizeof(code)/2)){
			stop = 1;
		} else {
			temp_note_1 = (mask_note_1 & code[indice_code]) >> 10;
			temp_note_2 = (mask_note_2 & code[indice_code]) >> 3;


			buffer[indice_notes] = code_to_frequency(temp_note_1);
			indice_notes++;
			buffer[indice_notes] = code_to_frequency(temp_note_2);
			indice_notes++;

		}
		indice_code++;
	}
}


static THD_WORKING_AREA(waPlayProjectThd, 1028);
static THD_FUNCTION(PlayProjectThd, arg) {

  chRegSetThreadName("PlayMelody Thd");

	(void)arg;

	uint16_t project_notes[MELODY_SIZE_MAX] = {0};
	uint16_t project_durations[MELODY_SIZE_MAX] = {0};

	uint16_t* notes_ptr = project_notes;
	uint16_t* durations_ptr = project_durations;

	

	get_melody_from_code(notes_ptr);
	get_durations_from_code(durations_ptr);

	while(1){



		for(uint8_t ThisNote = 0 ; ThisNote < 8 ; ThisNote++){

			playNote(project_notes[ThisNote], project_durations[ThisNote]);

			uint16_t pauseBetweenNotes = 100;
			chThdSleepMilliseconds(pauseBetweenNotes);
		}

		chThdSleepMilliseconds(2000);



	}
}

void playProjectStart(void){

	//create the thread
	chThdCreateStatic(waPlayProjectThd, sizeof(waPlayProjectThd), NORMALPRIO, PlayProjectThd, NULL);
}




