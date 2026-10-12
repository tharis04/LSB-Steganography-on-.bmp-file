#include <stdio.h>
#include <string.h>
#include <unistd.h> // for sleep()
#include "decode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

// Validating the input arguments for decoding: ./a.out -d stego.bmp [output.txt]
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // VALIDATE .bmp file (argv[2]) -> must exist and must end with ".bmp"
    // strrchr gives the LAST '.' in the name, and we check it is not NULL so strcmp never crashes
    if(argv[2] != NULL && strrchr(argv[2], '.') != NULL && strcmp(strrchr(argv[2], '.'), ".bmp") == 0)
    {
        // Fetch and store the file name in structure
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    // VALIDATE THE OPTIONAL OUTPUT FILE (argv[3])
    if(argv[3] != NULL)
    {
        // User gave a name -> remember it, we will use it as it is
        decInfo->output_fname = argv[3];
    }
    else
    {
        // User did not give a name -> default is output.txt
        // (NULL means "build the name later from output + decoded extension")
        decInfo->output_fname = NULL;
        strcpy(decInfo->output_fname_buf, "output.txt");
    }

    return e_success;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Stego Image file and Output file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_decode_files(DecodeInfo *decInfo)
{
    // Stego Image file in read mode
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    // Do Error handling
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

        return e_failure;
    }

    // Output file in write mode
    // If user gave a name use it, otherwise use the default (output.txt)
    if(decInfo->output_fname != NULL)
    {
        decInfo->fptr_output = fopen(decInfo->output_fname, "wb");
    }
    else
    {
        decInfo->fptr_output = fopen(decInfo->output_fname_buf, "wb");
    }
    // Do Error handling
    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open the output file\n");

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

// Skip the header: the encoded data starts after the 54 byte bmp header
Status skip_bmp_header(FILE *fptr_stego_image)
{
    // Move the file pointer to the 54th byte from the start of the file
    if(fseek(fptr_stego_image, 54, SEEK_SET) != 0)
    {
        return e_failure;
    }
    return e_success;
}

// Decode a number: reverse of encode_size_to_lsb
// Reads the LSB of 32 image bytes, MSB first, and builds an integer
Status decode_size_from_lsb(int *size, char *image_buffer)
{
    unsigned int value = 0;
    for(int i=0; i<32; i++)
    {
        // shift what we have to the left and add the next LSB at the right end
        value = (value << 1) | (image_buffer[i] & 1);
    }
    *size = (int)value;
    return e_success;
}

// Decode one character: reverse of encode_byte_to_lsb
// Reads the LSB of 8 image bytes, MSB first, and builds one character
Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    unsigned char ch = 0;
    for(int i=0; i<8; i++)
    {
        // shift what we have to the left and add the next LSB at the right end
        ch = (ch << 1) | (image_buffer[i] & 1);
    }
    *data = (char)ch;
    return e_success;
}

// CHARACTER DECODING: reverse of encode_data_to_image
// Decodes 'size' characters from the stego image and stores them in 'data'
Status decode_data_from_image(char *data, int size, FILE *fptr_stego, DecodeInfo *decInfo)
{
    for(int i=0 ; i<size ; i++)
    {
        // read 8 bytes of RGB from stego image
        if(fread(decInfo->image_data, 1, 8, fptr_stego) != 8)
        {
            return e_failure;
        }
        // get the LSB of each of the 8 bytes and form one character
        decode_byte_from_lsb(&data[i], decInfo->image_data);
    }
    return e_success;
}

// DECODING MAGIC STRING
Status decode_magic_string(const char *magic_str, DecodeInfo *decInfo)
{
    int size = strlen(magic_str);   // magic string size = 2
    char decoded[10];               // array to store the decoded magic string

    // Character decoding (reads 2 * 8 = 16 bytes from stego.bmp)
    if(decode_data_from_image(decoded, size, decInfo->fptr_stego_image, decInfo) == e_failure)
    {
        return e_failure;
    }
    decoded[size] = '\0';           // end the string so strcmp works

    // Compare the decoded magic string with the original MAGIC_STRING
    if(strcmp(decoded, magic_str) == 0)
    {
        return e_success;
    }
    return e_failure;
}

// Decode the size of the secret file extension (32 bytes -> number, example: 4)
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char str[32]; // to store 32 bytes of RGB data
    int size;

    if(fread(str, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }
    decode_size_from_lsb(&size, str);

    // Store the number in the structure variable
    decInfo->extn_size = size;

    // Safety: a wrong value here means the image is not stegged properly
    if(decInfo->extn_size > DECODE_FILE_SUFFIX)
    {
        return e_failure;
    }
    return e_success;
}

// Decode the secret file extension (example: ".txt")
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    // Loop runs "extn_size" times (decoded in the last step)
    if(decode_data_from_image(decInfo->extn_output_file, decInfo->extn_size, decInfo->fptr_stego_image, decInfo) == e_failure)
    {
        return e_failure;
    }
    decInfo->extn_output_file[decInfo->extn_size] = '\0'; // end the string

    // OPTIONAL: user did not give an output name -> open the file as "output" + decoded extension
    if(decInfo->output_fname == NULL)
    {
        char new_name[100];
        strcpy(new_name, "output");
        strcat(new_name, decInfo->extn_output_file);

        // Only reopen when the name really changed (default was output.txt)
        if(strcmp(new_name, decInfo->output_fname_buf) != 0)
        {
            fclose(decInfo->fptr_output);
            remove(decInfo->output_fname_buf);       // delete the empty default file
            strcpy(decInfo->output_fname_buf, new_name);

            decInfo->fptr_output = fopen(decInfo->output_fname_buf, "wb");
            if(decInfo->fptr_output == NULL)
            {
                perror("fopen");
                fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->output_fname_buf);
                return e_failure;
            }
        }
    }
    return e_success;
}

// Decode the size of the secret file (32 bytes -> number)
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char str[32]; // to store 32 bytes of RGB data
    int size;

    if(fread(str, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }
    decode_size_from_lsb(&size, str);

    // Store the number in the structure variable
    decInfo->size_secret_file = size;
    return e_success;
}

// Decode the secret data and write it to the output file
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char ch;
    if(decInfo->fptr_stego_image == NULL || decInfo->fptr_output == NULL)
    {
        return e_failure;
    }

    // Loop runs "size_secret_file" times (decoded in the last step)
    for(int i=0 ; i < decInfo->size_secret_file; i++)
    {
        // read 8 bytes of RGB from stego image
        if(fread(decInfo->image_data, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
        // get the LSB of each byte and form one character
        decode_byte_from_lsb(&ch, decInfo->image_data);

        // write the character to the output file, one after another
        fwrite(&ch, 1, 1, decInfo->fptr_output);
    }
    return e_success;
}

// Rest of the decoding function is called here
Status do_decoding(DecodeInfo *decInfo)
{
    sleep(1);
    // Open stego image (read mode) and output file (write mode)
    if(open_decode_files(decInfo) == e_success)
    {
        printf("\nOpened all files sucessfully!\n");
        printf("\n\033[31m*** STARTED DECODINGGGG! *** \033[0m\n");printf("\n");
        sleep(1);

        // Skip the 54 byte header
        if(skip_bmp_header(decInfo->fptr_stego_image) == e_success)
        {
            printf("\033[32mSuccessfully skipped the header..\033[0m\n");

            // Decoding magic string
            if(decode_magic_string(MAGIC_STRING, decInfo) == e_success)
            {
                printf("\033[1;2;32mMagic String is decoded successfully\033[0m\n");
                if(decode_secret_file_extn_size(decInfo) == e_success)
                {
                    printf("\n");
                    printf("Sucessfully decoded the secret file extension size\n");
                    sleep(1);
                    if(decode_secret_file_extn(decInfo) == e_success)
                    {
                        printf("Sucessfully decoded the secret file extension\n");
                        if(decode_secret_file_size(decInfo) == e_success)
                        {
                            printf("\n");
                            printf("Sucessfully decoded the secret file size!\n");
                            printf("\n"); sleep(1);
                            if(decode_secret_file_data(decInfo) == e_success)
                            {
                                printf("Decoded Secret data ***\n");
                            }
                            else
                            {
                                printf("Failed to decode secret data ***\n");
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf("\n");
                            printf("Failed to decode the secret file size!\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Failed to decode the secret file extension\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("\n");
                    printf("Failed to decode the secret file extension size\n");
                    return e_failure;
                }
            }
            else
            {
                printf("\n");
                printf("\033[1;2;31mNo secret data found in this image\033[0m\n");
                return e_failure;
            }
        }
        else
        {
            printf("\033[31mFailed to skip the header..\033[0m\n");
            return e_failure;
        }
    }
    else
    {
        printf("Failed to open the files\n");
        return e_failure;
    }

    // Close the files (this also makes sure the output is fully written to disk)
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);
    return e_success;
}