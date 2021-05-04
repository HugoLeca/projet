#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

void process_image_start(void);

//extracts the beginning (public_begin) and end (public end) of the bar code
void extract_limits(uint8_t *buffer);

#endif /* PROCESS_IMAGE_H */
