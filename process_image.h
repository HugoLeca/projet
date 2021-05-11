#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H


//start des threads CaptureImage et ProcessImage
void process_image_start(void);

//extracts the beginning (public_begin) and end (public end) of the bar code
void extract_limits(uint8_t *buffer);

//autre methode
void extract_limits_bis(uint8_t *buffer);

void extract_limits_move(uint8_t *buffer);

//analyse l'image et en extrait un code binaire
//uint16_t extract_code(uint8_t *buffer);

//encore une autre m�thode... renvoie le nombre de bits capt�s par la cam�ra.
uint8_t extract_code_ter(uint8_t *buffer);

//revoie le nombre de bits dans la section de taille width
uint8_t get_size_bits(uint16_t width);


//returns public_begin, i.e the first pixel of the code
uint16_t get_public_begin(void);

//returns public_end, i.e le last pixel of the code
uint16_t get_public_end(void);

uint16_t get_public_begin_move(void);

uint16_t get_public_end_move(void);



//retourne vraie si la moyenne de la section est inferrieur a� la moyenne de tout
//le code barre, retourne faux sinon
int get_section_value(uint16_t average_barcode, uint16_t average_section);

uint16_t get_public_begin(void);
uint16_t get_public_end(void) ;

#endif /* PROCESS_IMAGE_H */
