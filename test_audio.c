#include "ch.h"
#include "hal.h"
#include <test_audio.h>
#include <play_melody.h>



//creation d'une suite de notes exterieur au fichier play_melody.c/.h
static const uint16_t gamme_do_majeur_melody[] = {

	NOTE_C1, NOTE_D1, NOTE_E1, NOTE_F1, 
	NOTE_G1, NOTE_A1, NOTE_B1, NOTE_C2
};

static const float gamme_do_majeur_tempo[] = {

	4, 4, 4, 4,
	4, 4, 4, 4, 
};


static const melody_t melody[1] = {
  //gamme de do majeur
  {
    .notes = gamme_do_majeur_melody,
    .tempo = gamme_do_majeur_tempo ,
    .length = sizeof(gamme_do_majeur_melody)/sizeof(uint16_t),
  },

};


//joue simplement la gamme de do majeur en tant que external melody
void test_audio_external(){

	while(1){
		playMelody(EXTERNAL_SONG, ML_WAIT_AND_CHANGE, &melody[0]);
		waitMelodyHasFinished();
		chThdSleepMilliseconds(WAIT_BETWEEN_SONGS_MS);
	}
}






