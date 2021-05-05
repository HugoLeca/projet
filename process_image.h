#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

//start des threads CaptureImage et ProcessImage
void process_image_start(void);

//extracts the beginning (public_begin) and end (public end) of the bar code
void extract_limits(uint8_t *buffer);

//autre methode
void extract_limits_bis(uint8_t *buffer);

//analyse l'image et en extrait un code binaire
uint16_t extract_code(uint8_t *buffer);


//retourne vraie si la moyenne de la section est inferrieur a  la moyenne de tout
//le code barre, retourne faux sinon
int get_section_value(uint16_t average_barcode, uint16_t average_section);

#endif /* PROCESS_IMAGE_H */
