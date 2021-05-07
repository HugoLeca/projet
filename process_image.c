#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>



static uint16_t public_begin = 0;
static uint16_t public_end = 0;
static uint8_t shorted_bar_code = 0;
static uint8_t bar_code_failed = 0;
static uint16_t public_average_diff = 0;
static uint8_t data[BAR_CODE_SIZE] = {0};
//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

/*
 *  Returns the line's width extracted from the image buffer given
 *  Returns 0 if line not found
 */

void extract_limits_bis(uint8_t *buffer){
	
	uint16_t i = 0, j = IMAGE_BUFFER_SIZE - WIDTH_SLOPE, diff = 0, begin = 0, end = 0;
	uint8_t stop = 0;
	uint32_t volatile average_diff=0;
	uint16_t temp_diff = 0;

	//averaging the noise
	for(uint16_t i = 0 ; i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE) ; i++){

		if( buffer[i] > buffer[i+WIDTH_SLOPE] ){
			temp_diff = buffer[i] - buffer[i+WIDTH_SLOPE];
		} else {
			temp_diff = buffer[i+WIDTH_SLOPE] - buffer[i];
		}

		average_diff += temp_diff;
	}
	average_diff /= (IMAGE_BUFFER_SIZE - WIDTH_SLOPE) +1 ;
	public_average_diff = average_diff;
	

	//searching for a beginning
	while(stop == 0 && i<(IMAGE_BUFFER_SIZE - WIDTH_SLOPE)){

		if( abs(buffer[i]) > abs(buffer[i+WIDTH_SLOPE]) ){
			diff = buffer[i] - buffer[i+WIDTH_SLOPE];
		} else {
			diff = buffer[i+WIDTH_SLOPE] - buffer[i];
		}

		if (diff > 2.5*average_diff){
			begin = i + WIDTH_SLOPE;
			stop = 1;
		}
		i++;
	}

	// if begin found, search for end
	//

	if(i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE)  && begin){
		stop = 0;
		
		while(stop == 0 && j > WIDTH_SLOPE){

			if( (buffer[j-WIDTH_SLOPE]) < (buffer[j]) ){
				diff = buffer[j] - buffer[j-WIDTH_SLOPE];
			} else if((buffer[j-WIDTH_SLOPE]) > (buffer[j])) {
				diff = buffer[j-WIDTH_SLOPE] - buffer[j];
			} else { i = true;}

			if(diff > 2.5*average_diff){
				end = j - WIDTH_SLOPE;
				stop = 1;
			}
			j--;
		}
	}

	public_begin = begin;
	public_end = end;



}

void extract_limits(uint8_t *buffer){

	//local variables we are going to use in this function :
	uint16_t i = 0, j = IMAGE_BUFFER_SIZE, begin = 0, end = 0;
	uint32_t mean = 0;
	uint8_t  stop = 0, line_not_found = 0;

	//averaging
	for(uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE ; i++){
		mean += buffer[i];
	}
	mean /= IMAGE_BUFFER_SIZE;


	//search for a begin, starting from the left
	while(stop == 0 && i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE))
	{
		//the slope must at least be WIDTH_SLOPE wide and is compared
		//to the mean of the image
		if(buffer[i] > mean && buffer[i+WIDTH_SLOPE] < mean)
		{
			begin = i;
			stop = 1;
		}
		i++;
	}

	//if a begin was found, search for an end,
	//starts from the right this time
	if (i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE) && begin)
	{
		stop = 0;

		while(stop == 0 && j > 5)
		{
			if(buffer[j] < mean && buffer[j-WIDTH_SLOPE] > mean)
			{
				end = j;
				stop = 1;
			}
			j--;
		}
		//if an end was not found
		if (j < i + WIDTH_SLOPE || !end)
		{
			line_not_found = 1;
		}
	}else//if no begin was found
	{
		line_not_found = 1;
	}

	//if the bar code is too small, gives the respected signal
	if(!line_not_found && (end-begin) < MIN_BAR_CODE_WIDTH){
		begin = 0;
		end = 0;
		stop = 0;
		shorted_bar_code = 1;
		return;
	}

	if(line_not_found){
		bar_code_failed = 1;
		return;
	}else {
		public_begin = begin;
		public_end = end;
	}


}

uint8_t extract_code_ter(uint8_t *buffer){
	uint16_t width_pixels = 0;
	uint32_t average_barcode = 0;
	uint8_t count_size_data = 0, count_size_bits = 0;
	uint16_t new_begin = public_begin, old_begin = public_begin;
	uint16_t i = public_begin;


	width_pixels = public_end - public_begin + 1;


	for(uint16_t i = public_begin ; i < public_end ; i++){
		average_barcode += buffer[i];
	}
	average_barcode /= width_pixels;

	while(i < IMAGE_BUFFER_SIZE - WIDTH_SLOPE){
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

	}

	return count_size_bits;
}

uint8_t get_size_bits(uint16_t width){
	uint8_t size = 0;
	float pixels_per_bit = 0;
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

/*uint16_t extract_code_bis(uint8_t *buffer){
	uint16_t i = public_begin;
	uint8_t stop = 0;
	uint16_t black_average = 0;
	uint16_t bottom_average = 0;


	while(stop == 0 && i < public_end){
		if( abs(buffer[i]) > abs(buffer[i+WIDTH_SLOPE]) ){
			diff = buffer[i] - buffer[i+WIDTH_SLOPE];
		} else {
			diff = buffer[i+WIDTH_SLOPE] - buffer[i];
		}

		if (diff > 2*average_diff){
			stop = 1;
		}


		black_average += buffer[i];
		i++;
	}

	black_average /= (i - public_begin) - 1;
	//division enti�re pour avoir un seuil minuimum
	bottom_average = (100*black_average)/3;

	if(stop == 1){

		stop = 0;
	}


}
*/

/*uint16_t extract_code(uint8_t *buffer){

	//taille du code barre en pixels.
	//16 bits nécessaires (valeur max 640)
	uint16_t width_pixels = 0, code = 0;
	uint16_t pix_per_section = 0;
	uint16_t count = 1;
	uint16_t mask = 0;
	uint16_t average_section = 0; 
	uint16_t average_barcode = 0;

	mask = 1 << (BAR_CODE_SIZE - 1);
	width_pixels = (public_end - public_begin) + 1;
	//division entière pour avoir le nombre de compartiments
	pix_per_section = width_pixels/BAR_CODE_SIZE; 

	//averaging
	for(uint16_t i = public_begin ; i < public_end ; i++){
		average_barcode += buffer[i];
	}
	average_barcode /= width_pixels;


	//remplissage du code binaire
	for(uint16_t i= public_begin; i <= public_end ; i++){

		//sortie de boucle si les derniers pixels ne sont pas utiles 
		if ((public_end - i) + 1 < pix_per_section){
			break;
		} else {
			average_section += buffer[i];
			//fin de section --> prochain bit du code est disponible
			//reset du count et de la moyenne de section
			if (count == pix_per_section)
			{
				average_section /= pix_per_section;
				if (get_section_value(average_barcode, average_section))
				{
					code |= mask;
				}
				mask = mask >> 1;
				count = 0;
				average_section = 0;
			}
			count++;
		}	
	}

	return code;
}
*/

int get_section_value(uint16_t average_barcode, uint16_t average_section){

	if (average_section > average_barcode)
		{return false;}
	else {return true;}
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


static THD_WORKING_AREA(waProcessImage, 1024);
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

		uint8_t data_pixel=0;
		uint8_t temp_data=0;
		uint16_t code = 0;
		bool send_to_computer = true;
		uint8_t count_begin = 0;
		uint8_t count_end = 0;



		/*for (int i = 0; i < IMAGE_BUFFER_SIZE; ++i)
		{
			// bit by bit manipulation to have a variable data_pixel with the 6 bits Green values

			data_pixel = ((*img_buff_ptr) & 7)<<3; //gets the 3 MSB of the green data
			img_buff_ptr++; //pointing on the least significant byte
			temp_data= ((*img_buff_ptr) & 224) >> 5 ; // mask to get G0, G1, G2

			data_pixel |= temp_data;//merge of the information to get G0 --> G7
			image[i] = data_pixel;
			img_buff_ptr++;
		}*/
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

		//let's stabilize the limits in public_begin and public_end


		/*
		//using begin and end, let's extract the code seen by the camera
		if(public_begin != 0 && public_end < 600){

			code = extract_code_ter(image);

			if(code == 16){
				uint8_t volatile data_volatile[BAR_CODE_SIZE] = {0};
				uint8_t rank = 0;
				for(uint16_t i = 0; i < BAR_CODE_SIZE; i++){
					if(data[i] == 0){
						rank = i - 1;
						break;
					}else {
						data_volatile[i]=data[i];
					}
				}
				if(data_volatile[0] == 2 && data_volatile[rank] == 4){
					chprintf((BaseSequentialStream *)&SD3, "code=%ipixels\r\n",code);
				}
			}
		}*/

	
		//chprintf((BaseSequentialStream *)&SD3, "code=%lxpixels\r\n",code);
		//if(public_begin != 0 && public_end != IMAGE_BUFFER_SIZE - WIDTH_SLOPE)


	    chprintf((BaseSequentialStream *)&SD3, "begin=%ipixels\n\r",public_begin);
		chprintf((BaseSequentialStream *)&SD3, "end=%ipixels\n\r",public_end);
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
