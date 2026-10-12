#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h" // Contains user defined types

/*
 * Structure to store information required for
 * decoding the secret file from the stego image
 * Info about output and intermediate data is
 * also stored
 */

#define DECODE_IMAGE_BUF_SIZE 8   // 8 image bytes carry 1 secret byte
#define DECODE_FILE_SUFFIX 4      // ".txt" -> 4 characters

typedef struct _DecodeInfo
{
    /* Stego Image info */
    char *stego_image_fname;
    FILE *fptr_stego_image;
    char image_data[DECODE_IMAGE_BUF_SIZE];

    /* Output (decoded secret) file info */
    char *output_fname;                          // name given by user (or NULL)
    char output_fname_buf[100];                  // buffer to build "output" + extension
    FILE *fptr_output;
    char extn_output_file[DECODE_FILE_SUFFIX + 1]; // +1 for the '\0' at the end
    uint extn_size;                              // decoded size of the extension
    long size_secret_file;                       // decoded size of the secret data

} DecodeInfo;


/* Decoding function prototype */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_decode_files(DecodeInfo *decInfo);

/* Skip the 54 byte bmp header of the stego image */
Status skip_bmp_header(FILE *fptr_stego_image);

/* Decode and verify the Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/* Decode secret file extension size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data and write it to the output file */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode 'size' characters from the stego image into 'data' */
Status decode_data_from_image(char *data, int size, FILE *fptr_stego_image, DecodeInfo *decInfo);

/* Decode a number (32 bits) from the LSB of 32 image bytes */
Status decode_size_from_lsb(int *size, char *image_buffer);

/* Decode one byte (8 bits) from the LSBs of 8 image bytes */
Status decode_byte_from_lsb(char *data, char *image_buffer);

#endif