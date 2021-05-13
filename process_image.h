#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H


//start des threads CaptureImage et ProcessImage
void process_image_start(void);

//extracts the beginning (public_begin) and end (public end) of the bar code
void extract_limits(uint8_t *buffer);

//autre methode
void extract_limits_bis(uint8_t *buffer);

//analyse l'image et en extrait un code binaire
//uint16_t extract_code(uint8_t *buffer);

//encore une autre méthode... renvoie le nombre de bits captés par la caméra.
uint16_t extract_code_ter(uint8_t *buffer);

//revoie le nombre de bits dans la section de taille width
uint8_t get_size_bits(uint16_t width);


//returns public_begin, i.e the first pixel of the code
uint16_t get_public_begin(void);

//returns public_end, i.e le last pixel of the code
uint16_t get_public_end(void);

//return the bar code
uint16_t get_bar_code(void);

//returns the adress of bar_code_ready_sem so that playProjectThd in code_to_music file
/*//can access this ressource
binary_semaphore_t* get_bar_code_sem(void);
*/

//scheduling functions


/*mutex_t* get_barcode_mtx(void);
condition_variable_t* get_barcode_condvar(void);
*/

//retourne vraie si la moyenne de la section est inferrieur a  la moyenne de tout
//le code barre, retourne faux sinon
int get_section_value(uint16_t average_barcode, uint16_t average_section);

uint16_t* get_melody_ptr(void);
bool get_process_image_done(void);

#endif /* PROCESS_IMAGE_H */
