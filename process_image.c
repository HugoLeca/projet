#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>




static uint16_t public_end = 0;
static volatile uint8_t data[BAR_CODE_SIZE] = {0};
static uint16_t public_begin = 0;

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

/*
 *  Returns the line's width extracted from the image buffer given
 *  Returns 0 if line not found
 */

void extract_limits_bis(uint8_t *buffer){
	
	uint16_t   i = 0;
	uint16_t   j = IMAGE_BUFFER_SIZE - WIDTH_SLOPE;
	uint16_t   diff_begin = 0, diff_end = 0, begin = 0, end = 0;
	uint8_t    stop = 0;
	uint32_t   average_diff=0;

	average_diff = 10;

	//averaging the noise
	
	//searching for a beginning
	while(i < 320){

		if(abs(buffer[i]) > abs(buffer[i+WIDTH_SLOPE])){
			diff_begin = buffer[i] - buffer[i+WIDTH_SLOPE];
		} else {
			diff_begin = buffer[i+WIDTH_SLOPE] - buffer[i];
		}

		if (diff_begin > 3*average_diff){
			diff_begin = 0; 
			begin = i + WIDTH_SLOPE; 
			//stop = 1;
		} 
		i++;
	}

	// if begin found, search for end

	//if(i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE)  && begin != 0){
		//stop = 0;
		
		while(j > 400){

			if( (buffer[j-WIDTH_SLOPE]) < (buffer[j]) ){
				diff_end = buffer[j] - buffer[j-WIDTH_SLOPE];
			} else if((buffer[j-WIDTH_SLOPE]) > (buffer[j])) {
				diff_end = buffer[j-WIDTH_SLOPE] - buffer[j];
			} //else { i = true;}

			if(diff_end > 3*average_diff){
				diff_end = 0; 
				end = j - WIDTH_SLOPE;
				//stop = 1;
			}
			j--;
		}
	//}

	public_end = end;
	public_begin = begin;

}


uint8_t extract_code_ter(uint8_t *buffer){
	uint16_t width_pixels = 0;
	uint32_t average_barcode = 0;
	uint8_t count_size_data = 0, count_size_bits = 0;
	uint16_t new_begin = public_begin, old_begin = public_begin;
	uint16_t i = public_begin;


	width_pixels = public_end - public_begin + 1;

	uint16_t average_diff = 0;
	uint16_t diff = 0;

	average_diff = 10;
	uint8_t volatile data_volatile[BAR_CODE_SIZE] = {0};

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
			i += WIDTH_SLOPE;




			/*section_width = (new_begin - old_begin +1);
			old_begin = new_begin;

			//find how many bits there are in that section
			bits_size = get_size_bits(section_width);

			//insert the value into data table
			data[count_size_data] = bits_size;
			count_size_bits += bits_size;
			count_size_data++;
			i += WIDTH_SLOPE;
			*/
			
		}
		i++;
	}

	for(uint8_t i = 0; i<BAR_CODE_SIZE; i++){
		data_volatile[i]=data[i];
	}





/*	while(i < IMAGE_BUFFER_SIZE - WIDTH_SLOPE){
		if((buffer[i] > average_barcode && buffer[i+WIDTH_SLOPE] < average_barcode) ||
				(buffer[i] < average_barcode && buffer[i+WIDTH_SLOPE] > average_barcode)){
			uint16_t section_width = 0;
			uint8_t bits_size = 0;

			//find the section new section width
			new_begin = i;
			section_width = (new_begin - old_begin +1);
			old_begin = new_begin;

			//find how many bits there are in that section
			bits_size = get_size_bits(section_width);

			//insert the value into data table
			data[count_size_data] = bits_size;
			count_size_bits += bits_size;
			count_size_data++;
			i += WIDTH_SLOPE;
		}
		i++;

	}*/

	return count_size_bits;
}

uint8_t get_size_bits(uint16_t width){

	uint8_t size = 0;
	float   pixels_per_bit = 0;
	uint16_t total_width = (public_end - public_begin + 1);

	pixels_per_bit = (float)total_width/BAR_CODE_SIZE;

	if((float)width < 1.5*pixels_per_bit){
		size = 1;
		return size;
	} 

	for(uint8_t i = 1; i <= BAR_CODE_SIZE - 1; i++){
		if ((float)width > 1.5*i*pixels_per_bit){
			size = i+1;
			return size;
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


static THD_WORKING_AREA(waProcessImage, 2048);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_buff_ptr; 
	uint8_t image[IMAGE_BUFFER_SIZE] = {0};

	

	systime_t time;


    while(1){

    	time = chVTGetSystemTime();
    	//waits until an image has been captured
        chBSemWait(&image_ready_sem);
		//gets the pointer to the array filled with the last image in RGB565    
		img_buff_ptr = dcmi_get_last_image_ptr();

		//sends the data buffer of the given size to the computer
		//SendUint8ToComputer(uint8_t* data, uint16_t size);


		uint16_t code = 0;



		//Extracts only the red pixels
		for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){
		//extracts first 5bits of the first byte

		//takes nothing from the second byte
		image[i/2] = (uint8_t)img_buff_ptr[i]&0xF8;
		}

		/*if(send_to_computer){
			//sends to the computer the image
			SendUint8ToComputer(image, IMAGE_BUFFER_SIZE);
		}
		//invert the bool
		send_to_computer = !send_to_computer;
*/






		//let's find the (public) end and begin variables
		extract_limits_bis(image);
		//chprintf((BaseSequentialStream *)&SD3, "begin=%i pixels",public_begin);
		//chprintf((BaseSequentialStream *)&SD3, "end=%ipixels\r\n",public_end);


		//let's stabilize the limits in public_begin and public_end



		//using begin and end, let's extract the code seen by the camera
		if(public_end != 630){

			code = extract_code_ter(image);


			//chprintf((BaseSequentialStream *)&SD3, "code=%ipixels\r\n",code);



		}




	
		//chprintf((BaseSequentialStream *)&SD3, "code=%lxpixels\r\n",code);
		//if(public_begin != 0 && public_end != IMAGE_BUFFER_SIZE - WIDTH_SLOPE)
	}
}



uint16_t get_public_begin(void){
	return public_begin;
}

uint16_t get_public_end(void){
	return public_end;
}


void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
