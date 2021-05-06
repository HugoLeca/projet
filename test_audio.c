#include "ch.h"
#include "hal.h"
#include <test_audio.h>
#include <play_melody.h>

#include <main.h>


//creation d'une suite de notes exterieur au fichier play_melody.c/.h
static const uint16_t gamme_do_majeur_melody[] = {

	NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4,
	NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5,
};

static const float gamme_do_majeur_tempo[] = {

	16, 16, 16, 16,
	16, 16, 16, 16,
};


static const melody_t melody[1] = {
  //gamme de do majeur
  {
    .notes = gamme_do_majeur_melody,
    .tempo = gamme_do_majeur_tempo,
    .length = sizeof(gamme_do_majeur_melody)/sizeof(uint16_t),
  },

};


//joue simplement la gamme de do majeur en tant que external melody
void test_audio_external(void){


	while(1){
		playMelody(EXTERNAL_SONG, ML_SIMPLE_PLAY, &melody[0]);
		chThdSleepMilliseconds(3000);
	}
}


// test de mm fonction mais avec un thread

/*static THD_WORKING_AREA(waTestAudioThd, 526);
static THD_FUNCTION(TestAudioThd, arg) {

  chRegSetThreadName("TestAudio Thd");

	(void)arg;



	while(1){

		playMelody(EXTERNAL_SONG, ML_WAIT_AND_CHANGE, &melody[0]);
		waitMelodyHasFinished();
		chThdSleepMilliseconds(WAIT_BETWEEN_SONGS_MS);
	}
}

void TestAudioStart(void){

	//create the thread
	chThdCreateStatic(waTestAudioThd, sizeof(waTestAudioThd), NORMALPRIO, TestAudioThd, NULL);
}*/






