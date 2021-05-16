#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>
#include <stdlib.h>

#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>
#include <robot_management.h>
#include <play_melody.h>
#include <code_to_music.h>






static uint16_t public_end = 0;
static uint16_t public_end_move = 0;
static volatile uint16_t data[BAR_CODE_SIZE] = {0};
static uint16_t public_begin = 0;
static uint16_t public_begin_move = 0; 
static uint16_t bar_code = 0;
static bool code_detected = false;
//semaphores
static BSEMAPHORE_DECL(image_ready_sem, TRUE);
//static BSEMAPHORE_DECL(bar_code_ready_sem, TRUE);
/*static MUTEX_DECL(bar_code_lock);
static CONDVAR_DECL(bar_code_condvar);
*/
//conditional variables


//reference
static thread_reference_t process_image_ref = NULL;

static bool process_image_done = false;
static uint16_t final_code[MELODY_SIZE_MAX] = {0};
static uint8_t code_indice_cnt;



/*
 *  Returns the line's width extracted from the image buffer given
 *  Returns 0 if line not found
 */
void extract_limits_bis(uint8_t *buffer){
	
	uint16_t   i = 0;
	uint16_t   j = IMAGE_BUFFER_SIZE - WIDTH_SLOPE;
	uint16_t   diff = 0, begin = 0, end = 0;
	uint8_t stop = 0;
	uint32_t   average_diff=0;



	average_diff = 10;

	//averaging the noise

	//searching for a beginning
	while(stop == 0 && i<(IMAGE_BUFFER_SIZE - WIDTH_SLOPE)){

		if( abs(buffer[i]) > abs(buffer[i+WIDTH_SLOPE]) ){
			diff = buffer[i] - buffer[i+WIDTH_SLOPE];
		} else {
			diff = buffer[i+WIDTH_SLOPE] - buffer[i];
		}

		if (diff > 3*average_diff){
			begin = i + WIDTH_SLOPE;
			stop = 1;
		}
		i++;
	}

	// if begin found, search for end

	if(i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE)  && begin){
		stop = 0;
		
		while(stop == 0 && j > WIDTH_SLOPE){

			if( (buffer[j-WIDTH_SLOPE]) < (buffer[j]) ){
				diff = buffer[j] - buffer[j-WIDTH_SLOPE];
			} else if((buffer[j-WIDTH_SLOPE]) > (buffer[j])) {
				diff = buffer[j-WIDTH_SLOPE] - buffer[j];
			} else { i = true;}

			if(diff > 4*average_diff && j != (IMAGE_BUFFER_SIZE - 2*WIDTH_SLOPE)){
				end = j - WIDTH_SLOPE;
				stop = 1;
			}
			j--;
		}
	}

	if(1){
		public_end = end;
		public_begin = begin;
	}
}

void extract_limits_move(uint8_t *buffer){
	
	uint16_t   i = 0;
	uint16_t   j = IMAGE_BUFFER_SIZE - WIDTH_SLOPE;
	uint16_t   diff_begin = 0, diff_end = 0, begin = 0, end = 0;
	uint8_t    stop_begin = 0, stop_end = 0;
	uint32_t   average_diff=0;

	average_diff = 10;

	//averaging the noise
	
	//searching for a beginning
	while (i < 120 && stop_begin == 0){

		if(abs(buffer[i]) > abs(buffer[i+WIDTH_SLOPE])){
			diff_begin = buffer[i] - buffer[i+WIDTH_SLOPE];
		} else {
			diff_begin = buffer[i+WIDTH_SLOPE] - buffer[i];
		}

		if (diff_begin > 3*average_diff){
			diff_begin = 0; 
			begin = i + WIDTH_SLOPE; 
			stop_begin = 1;
		} 
		i++;
	}

	// if begin found, search for end

	//if(i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE)  && begin != 0){
		//stop = 0;
		
		while(j > 500 && stop_end == 0){

			if( (buffer[j-WIDTH_SLOPE]) < (buffer[j]) ){
				diff_end = buffer[j] - buffer[j-WIDTH_SLOPE];
			} else if((buffer[j-WIDTH_SLOPE]) > (buffer[j])) {
				diff_end = buffer[j-WIDTH_SLOPE] - buffer[j];
			} //else { i = true;}

			if(diff_end > 3*average_diff){
				diff_end = 0; 
				end = j - WIDTH_SLOPE;
				stop_end = 1;
			}
			j--;
		}
	//}

	public_end_move = end;
	public_begin_move = begin;

}


uint16_t extract_code_ter(uint8_t *buffer){
	uint16_t width_pixels = 0;
	uint32_t average_barcode = 0;
	uint8_t count_size_data = 0, count_size_bits = 0;
	uint16_t new_begin = public_begin, old_begin = public_begin;
	uint16_t volatile i = 0;


	width_pixels = public_end - public_begin + 1;

	uint16_t average_diff = 0;
	uint16_t volatile diff = 0;

	average_diff = 10;
	uint16_t volatile data_volatile[BAR_CODE_SIZE] = {0};



	//data and data_volatile contain every step (positive or negative) on the image
	while(i<(IMAGE_BUFFER_SIZE - WIDTH_SLOPE)){



		if( abs(buffer[i]) > abs(buffer[i+WIDTH_SLOPE]) ){
			diff = buffer[i] - buffer[i+WIDTH_SLOPE];
		} else {
			diff = buffer[i+WIDTH_SLOPE] - buffer[i];
		}

		if (diff > 3*average_diff){
			uint16_t section_width = 0;
			uint8_t bits_size = 0;

			//find the section new section width
			new_begin = i;


			//check step detection

			data[count_size_data] = i;
			count_size_data++;

			//moves i enough so that the algo doesn't detect the same step
			i += 3*WIDTH_SLOPE;
			
		}
		i++;
	}

	for(uint8_t i = 0; i<BAR_CODE_SIZE; i++){
		data_volatile[i]=data[i];
	}

	uint8_t null_rank = 0;

	for(uint8_t i = 0; i < BAR_CODE_SIZE ; i++){
		if(data_volatile[i] == 0){
			null_rank = i;
			break;
		}
	}

	public_begin = data_volatile[0];
	public_end = data_volatile[null_rank - 1];


	//code filling algorithm
	bool bit_value = true; 
	uint8_t volatile nbre_bits = 0;
	uint16_t volatile mask = 0;
	uint16_t volatile code = 0;
	uint8_t code_indice = 0;
	uint8_t addition = 0;

	//� supprimer
	uint8_t volatile nbre_bits_tab[BAR_CODE_SIZE] = {0};

	for(uint8_t i = 1; i < null_rank; i++){

		nbre_bits = get_size_bits(data_volatile[i] - data_volatile[i-1]);
		nbre_bits_tab[i] = nbre_bits;
		addition = 1;

		for(uint8_t j = 0; j < nbre_bits ; j++){
			mask += addition;
			addition = addition*2;
		}


		if(bit_value){
			mask = mask << ((BAR_CODE_SIZE - nbre_bits) - code_indice);
			code |= mask;

		} 
		mask = 0;
		code_indice += nbre_bits;

		bit_value = !bit_value;
	}

	return code;
}

uint8_t get_size_bits(uint16_t width){

	uint8_t size = 0;
	float   pixels_per_bit = 0;
	uint16_t total_width = (public_end - public_begin + 1);

	pixels_per_bit = (float)total_width/BAR_CODE_SIZE;

	if((float)width < 1.5*pixels_per_bit){
		size = 1;
		return size;
	} else {
		for(uint8_t i = 1; i <= BAR_CODE_SIZE - 1; i++){
			if ((float)width > i*pixels_per_bit + 0.5*pixels_per_bit){
				size = i+1;
			} else {break;}
		}
	}
	return size;
}



static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

    while(1){
        //starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
    }
}


static THD_WORKING_AREA(waProcessImage, 1028);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_buff_ptr; 
	uint8_t image[IMAGE_BUFFER_SIZE] = {0};
	bool code_detected = false;

	

	systime_t time;
	systime_t new_time;

	bool* process = NULL;

    while(1){

    //wait for RobotManagement's signal
    


    	time = chVTGetSystemTime();
    	//waits until an image has been captured
        chBSemWait(&image_ready_sem);
		//gets the pointer to the array filled with the last image in RGB565    
		img_buff_ptr = dcmi_get_last_image_ptr();

		//sends the data buffer of the given size to the computer
		//SendUint8ToComputer(uint8_t* data, uint16_t size);

		bool send_to_computer = true;

		uint16_t volatile code = 0;



		//Extracts only the red pixels
		for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){
		//extracts first 5bits of the first byte

		//takes nothing from the second byte
		image[i/2] = (uint8_t)img_buff_ptr[i]&0xF8;
		}
		/*
		if(send_to_computer){
			//sends to the computer the image
			SendUint8ToComputer(image, IMAGE_BUFFER_SIZE);
		}
		//invert the bool
		send_to_computer = !send_to_computer;
		*/



		//let's find the (public) end and begin variables
		extract_limits_move(image); 
		

		extract_limits_bis(image);
		//chprintf((BaseSequentialStream *)&SD3, "begin=%i pixels",public_begin);
		//chprintf((BaseSequentialStream *)&SD3, "end=%ipixels\r\n",public_end);




		//code is send only if the hash is respected and if the code is recognized
		// with sufficient repetability. i.e bit0(code) = 1, bit15(code) = 1, and compteur = 10.
		uint8_t compteur = 0;
		uint16_t compteur_stability = 0;
		uint16_t temp_code = 0;
		uint16_t mask_bit15 = 0b1000000000000000;
		uint16_t mask_bit0  = 0b0000000000000001;
		bool code_stable = true;
		bool fill_code = false;

		chMtxLock(get_processImage_lock());
			while(!get_start_reading_code()){
				//wait for the robot to be in a good position
				chCondWait(get_processImage_condvar());
			}

		while(code_stable){
			
			code = extract_code_ter(image);
			
			if((code & mask_bit15) && (code & mask_bit0)){
				if(code == temp_code){
					compteur++;
					if(compteur == 50){
						fill_code = true;
						break;
					}
				} else {
					temp_code = code;
					compteur = 1;
				}				
			}
			compteur_stability++;

			if(compteur_stability > 1000){
				code_stable = false;
				chprintf((BaseSequentialStream *)&SD3, "CODE_NOT_STABLE");
				break;
			}
		}

		if(fill_code){
			
			//chMtxLock(&bar_code_lock);

			bar_code = code;
			chprintf((BaseSequentialStream *)&SD3, "barcode=%i\r\n",bar_code);

			playNote(BIP_FREQU, BIP_DURATION);

			final_code[code_indice_cnt] = bar_code;
			code_indice_cnt++;

			code_detected = true;

			





			//chThdSleepMilliseconds(4000);



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

				get_melody_from_code(notes_ptr, final_code);
				get_durations_from_code(durations_ptr, final_code);


				process_image_done = true;



				uint8_t notes_null_rank = 0;
				for(uint8_t i = 0; i < MELODY_SIZE_MAX ; i++){
					if(project_notes[i] == 0){
						notes_null_rank = i;
						break;
					}
				}

				//enters an infinite loop

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



			//tells the threads waiting that the image has been processed correctly
			


			//chMtxUnlock(&bar_code_lock);
			//signal que un code barre a été trouvé
			//chCondSignal(&bar_code_condvar);

			//signals the bar_code has been captured
			//chBSemSignal(&bar_code_ready_sem);

			//new_time = chVTGetSystemTime();
			//chprintf((BaseSequentialStream *)&SD3, "time_to_finish_thread=%ld\r\n",new_time - time);

		}
	}
	
	chMtxUnlock(get_processImage_lock());
}


uint16_t get_public_begin_move(void){
	return public_begin_move;
}

uint16_t get_public_end_move(void){
	return public_end_move;
}

uint16_t get_public_begin(void){
	return public_begin;
}

uint16_t get_public_end(void){
	return public_end;
}

uint16_t get_bar_code(void){
	return bar_code;
}

bool get_code_detected(void) {
	return code_detected;
}

thread_reference_t* get_processImage_ref(void){
	return &process_image_ref;
}


/*condition_variable_t* get_barcode_condvar(void){
	return &bar_code_condvar;
}

mutex_t* get_barcode_mtx(void){
	return &bar_code_lock;
}
*/

void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
