/*
	NAME : process_image.h
	AUTHOR : HUGO LECA and MARGUERITE FAUROUX
	LAST MODIFICATION : 16/05/2021
*/

#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H


/*start des threads CaptureImage et ProcessImage*/
void process_image_start(void);

/*
extracts the beginning (public_begin) and end (public end) of the bar code
modified variables : static variables public_begin and public_end
*/
void extract_limits(uint8_t *buffer);

/*
extracts the beginning (public_begin) and end (public end) of the bar code
the returned values are different than in the function extract_limts so the robot
can adapt et reorient itself in front of the barcode
modified : static variables public_begin and public_end
*/
void extract_limits_move(uint8_t *buffer);

/*
brief : process the image data and returns the corresponding barcode
param : intensity buffer 
*/
uint16_t extract_code(uint8_t *buffer);


/*return the number of bits in the section of size width*/
uint8_t get_size_bits(uint16_t width);

/*returns public_begin, i.e the first pixel of the code*/
uint16_t get_public_begin_move(void);

/*returns public_end, i.e le last pixel of the code*/
uint16_t get_public_end_move(void);

/*return the 16 bits barcode*/
uint16_t get_code(void);


#endif /* PROCESS_IMAGE_H */
