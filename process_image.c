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
//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

/*
 *  Returns the line's width extracted from the image buffer given
 *  Returns 0 if line not found
 */

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

		while(stop == 0 && i < IMAGE_BUFFER_SIZE)
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



		for (int i = 0; i < IMAGE_BUFFER_SIZE; ++i)
		{
			// bit by bit manipulation to have a variable data_pixel with the 6 bits Green values

			data_pixel = ((*img_buff_ptr) & 7)<<3; //gets the 3 MSB of the green data
			img_buff_ptr++; //pointing on the least significant byte
			temp_data= ((*img_buff_ptr) & 224) >> 5 ; // mask to get G0, G1, G2

			data_pixel |= temp_data;//merge of the information to get G0 --> G7
			image[i] = data_pixel;
			img_buff_ptr++;
		}

		//let's find the (public) end and begin variables
		extract_limits(image);

		chprintf((BaseSequentialStream *)&SD3, "begin=%ipixels\n",public_begin);
		chprintf((BaseSequentialStream *)&SD3, "end=%ipixels\n",public_end);

		//SendUint8ToComputer(image, IMAGE_BUFFER_SIZE);
    }


}



void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
