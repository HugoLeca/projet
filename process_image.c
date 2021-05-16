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






static uint16_t public_end = 0;
static uint16_t public_end_move = 0;
static volatile uint16_t data[BAR_CODE_SIZE] = {0};
static uint16_t public_begin = 0;
static uint16_t public_begin_move = 0; 
static uint16_t code = 0; 
//semaphores
static BSEMAPHORE_DECL(image_ready_sem, TRUE);
//static BSEMAPHORE_DECL(bar_code_ready_sem, TRUE);
/*static MUTEX_DECL(bar_code_lock);
static CONDVAR_DECL(bar_code_condvar);
*/

/*
 *  Returns the line's width extracted from the image buffer given
 *  Returns 0 if line not found
 */
void extract_limits_bis(uint8_t *buffer){
	
	uint16_t   i = 0;
	uint16_t   j = IMAGE_BUFFER_SIZE - WIDTH_SLOPE;
	uint16_t   diff = 0, begin = 0, end = 0;
	uint8_t stop = 0;


	//searching for a beginning
	while(stop == 0 && i<(IMAGE_BUFFER_SIZE - WIDTH_SLOPE)){

		if( abs(buffer[i]) > abs(buffer[i+WIDTH_SLOPE]) ){
			diff = buffer[i] - buffer[i+WIDTH_SLOPE];
		} else {
			diff = buffer[i+WIDTH_SLOPE] - buffer[i];
		}

		if (diff > STEP_COEF*AVERAGE_DIFF){
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

			if(diff > STEP_COEF*AVERAGE_DIFF && j != (IMAGE_BUFFER_SIZE - 2*WIDTH_SLOPE)){
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
	
	uint16_t i = 0;
	uint16_t j = IMAGE_BUFFER_SIZE - WIDTH_SLOPE;
	uint16_t diff_begin = 0, diff_end = 0, begin = 0, end = 0;
	uint8_t    stop_begin = 0, stop_end = 0;

	
	// searching for a beginning
	// if the beginning is more than BEGINNING_THRESHOLD the value stays at zero 
	// that means the robot is not well oriented 
	while (i < BEGINNING_THRESHOLD && stop_begin == 0){

		if(abs(buffer[i]) > abs(buffer[i+WIDTH_SLOPE])){
			diff_begin = buffer[i] - buffer[i+WIDTH_SLOPE];
		} else {
			diff_begin = buffer[i+WIDTH_SLOPE] - buffer[i];
		}

		if (diff_begin > STEP_COEF*AVERAGE_DIFF){
			diff_begin = 0; 
			begin = i + WIDTH_SLOPE; 
			stop_begin = 1;
		} 
		i++;
	}

	// searching for an end 
	// if the end is less than END_THRESHOLD the value stays at zero
	// that means the robot is not well oriented 	
	while(j > 490 && stop_end == 0){

		if( (buffer[j-WIDTH_SLOPE]) < (buffer[j]) ){
			diff_end = buffer[j] - buffer[j-WIDTH_SLOPE];
		} else if((buffer[j-WIDTH_SLOPE]) > (buffer[j])) {
			diff_end = buffer[j-WIDTH_SLOPE] - buffer[j];
		}

		if(diff_end > STEP_COEF*AVERAGE_DIFF){
			diff_end = 0;
			end = j - WIDTH_SLOPE;
			stop_end = 1;
		}
		j--;
	}

	public_end_move = end;
	public_begin_move = begin;

}

uint16_t extract_code_ter(uint8_t *buffer){

	uint8_t count_size_data = 0;
	uint16_t new_begin = public_begin;
	uint16_t volatile i = 0;


	uint16_t volatile diff = 0;




	//data contain every step (positive or negative) on the image
	while(i<(IMAGE_BUFFER_SIZE - WIDTH_SLOPE)){

		if( abs(buffer[i]) > abs(buffer[i+WIDTH_SLOPE]) ){
			diff = buffer[i] - buffer[i+WIDTH_SLOPE];
		} else {
			diff = buffer[i+WIDTH_SLOPE] - buffer[i];
		}

		if (diff > STEP_COEF*AVERAGE_DIFF){

			//find the section new section width
			new_begin = i;

			//check step detection
			data[count_size_data] = new_begin;
			count_size_data++;

			//moves i forward enough so that the algo doesn't detect the same step
			i += 3*WIDTH_SLOPE;
			
		}
		i++;
	}

//////////////////////////////////////////////////////////////////////////
	//à voir si utile pour le code
	uint8_t null_rank = 0;

	for(uint8_t i = 0; i < BAR_CODE_SIZE ; i++){
		if(data[i] == 0){
			null_rank = i;
			break;
		}
	}

	//extract the limits of the barcode in pixels
	public_begin = data[0];
	public_end = data[null_rank - 1];

//////////////////////////////////////////////////////////////////////////




	//code filling algorithm
	bool bit_value = true; 
	uint8_t volatile nbre_bits = 0;
	uint16_t volatile mask = 0;
	uint16_t volatile code = 0;
	uint8_t code_indice = 0;
	uint8_t addition = 0;


	for(uint8_t i = 1; i < null_rank; i++){

		nbre_bits = get_size_bits(data[i] - data[i-1]);
		addition = 1;

		//putting the bits at the right place according to the number of bits
		for(uint8_t j = 0; j < nbre_bits ; j++){
			mask += addition;
			addition = addition*2;
		}
		if(bit_value){
			mask = mask << ((BAR_CODE_SIZE - nbre_bits) - code_indice);
			code |= mask;
		} 
		//reset the mask
		mask = 0;
		code_indice += nbre_bits;

		//toggle the bit value for the next step detected
		bit_value = !bit_value;
	}

	return code;
}

uint8_t get_size_bits(uint16_t width){


	uint8_t size = 0;
	float   pixels_per_bit = 0;
	uint16_t total_width = (public_end - public_begin + 1);

	//compute the number of pixels (float value) for one bits
	pixels_per_bit = (float)total_width/BAR_CODE_SIZE;

	//size takes the value of the number of pixels in width
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

	//THREAD ROUTINE : fill the image buffer and extracts the interesting data
    while(1){

    	for(uint8_t i = 0; i<10 ; i++){

	    	//waits until an image has been captured
	        chBSemWait(&image_ready_sem);
			//gets the pointer to the array filled with the last image in RGB565    
			img_buff_ptr = dcmi_get_last_image_ptr();


			//Extracts only the red pixels
			for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2) {
				//extracts first 5bits of the first byte

				//takes nothing from the second byte
				image[i/2] = (uint8_t)img_buff_ptr[i]&0xF8;
			}

			//let's find the (public) end and begin variables	

			//extracts the limits for decoding the barcode
			extract_limits_bis(image);

			//extract the limits for robot placement in front of the barcode
			extract_limits_move(image);

			//return the 16bits binary barcode
			code = extract_code_ter(image); 
		}
	}
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

uint16_t get_code(void){
	return code;
}


void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
