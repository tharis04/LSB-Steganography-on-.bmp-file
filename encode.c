#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"
#include <unistd.h> // for sleep()

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

// Validating the input file .bmp file and .txt file
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // VALIDATE .bmp file 
    if(argv[2] != NULL && strcmp(strstr(argv[2], "."), ".bmp") == 0)
    {
        // Fetch and store the file name in structure
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    // VALIDATE .txt file 
    if(argv[3] != NULL && strcmp(strstr(argv[3], "."), ".txt") == 0)
    {
        // Fetch and store the file name in structure
        encInfo->secret_fname = argv[3];
    }
    else
    {
        return e_failure;
    }

    // VALIDATE THE OPTIONAL OUTPUT FILE
    if(argv[4] != NULL)
    {
        // Fetch and store the file name in structure
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }

    return e_success;
}
uint get_file_size(FILE *fptr_secr)
{
    fseek(fptr_secr,0,SEEK_END);
    long size = ftell(fptr_secr);
    fseek(fptr_secr,0,SEEK_SET);
    return size;
}

Status check_capacity(EncodeInfo *encInfo)
{
    // Get the size of image file
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    // Get the size of secret file
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    if(encInfo->image_capacity > (54 + ((2 + 4 + 4 + 4 + encInfo->size_secret_file) *8)))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

// copy the header from source to destination
Status copy_bmp_header(FILE *fptr_src, FILE *fptr_stego)
{
    char header[54];
    fseek(fptr_src,0,SEEK_SET);

    // read 54 bytes from source
    fread(header, 54, sizeof(char), fptr_src);

    // writing 54 byts to destination
    fwrite(header, 54, sizeof(char), fptr_stego);
    return e_success;
}

// Encode the character in the LSB of RGB
Status encode_size_to_lsb(int data, char *image_buffer)
{
    unsigned int mask = 1 << 31;
    for(int i=0; i<32 ; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data & mask) >> (31-i));
        mask = mask >> 1;
    }
    return e_success;
}

// Encode one byte (8 bits) into the LSBs of 8 image bytes
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i=0; i<8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> (7-i)) & 1);
    }
    return e_success;
}

// CHARACTER ENCODING
Status encode_data_to_image(const char *data, int size, FILE *fptr_src, FILE *fptr_stego, EncodeInfo *encInfo)
{
    // call encode_byte_to_lsb to encode the data one after the other
    for(int i=0 ; i<size ; i++)
    {
        // read 8 bytes of RGB from source image
        if(fread(encInfo->image_data, 1, 8, fptr_src) != 8)
        {
            return e_failure;
        }
        encode_byte_to_lsb(data[i],encInfo->image_data);
        fwrite(encInfo->image_data,8,sizeof(char), fptr_stego);
    }
    return e_success;
}




// ENCODING MAGIC STRING
Status encode_magic_string(const char *magic_str, EncodeInfo *encInfo)
{
    // Character encoding
    encode_data_to_image(magic_str, strlen(magic_str), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}

Status encode_size(int size, FILE *fptr_src, FILE *fptr_stego)
{
    char str[32]; // to store 32 bytes of RGB data to store the .txt
    fread(str, 32, sizeof(char),fptr_src);
    encode_size_to_lsb(size,str);
    fwrite(str,32,sizeof(char),fptr_stego);
    return e_success;
}

Status encode_secret_file_size(int size, EncodeInfo *encInfo)
{
    char str[32]; // to store 32 bytes of RGB data to store the .txt
    fread(str, 32, sizeof(char),encInfo->fptr_src_image);
    encode_size_to_lsb(size,str);
    fwrite(str,32,sizeof(char),encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    file_extn = ".txt";
    encode_data_to_image(file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    if(encInfo->fptr_secret == NULL || encInfo->fptr_src_image == NULL || encInfo->fptr_stego_image == NULL)
    {
        return e_failure;
    }
    fseek(encInfo->fptr_secret,0,SEEK_SET);
    for(int i=0 ; i< encInfo->size_secret_file; i++)
    {
        if(fread(encInfo->image_data, 1, 8, encInfo->fptr_src_image) != 8)
        {
            return e_failure;
        }
        if(fread(&ch,1,1,encInfo->fptr_secret) != 1)
        {
            return e_failure;
        }
        encode_byte_to_lsb(ch,encInfo->image_data);
        fwrite(encInfo->image_data,8,sizeof(char), encInfo->fptr_stego_image);
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch,1,1, fptr_src) > 0 )
    {
        fwrite(&ch,1,1,fptr_dest);
    }
    return e_success;
}

// Rest of the encoding fucnction is called hereeee
Status do_encoding(EncodeInfo *encInfo)
{
    // Open all files needed for encoding
    sleep(1);
    if(open_files(encInfo) == e_success)
    {
        printf("\nOpened all files sucessfully!\n");
        printf("\n\033[31m*** STARTED ENCODINGGGG! *** \033[0m\n");printf("\n");
        sleep(1);
        
        if(check_capacity(encInfo) == e_success)
        {
            printf("Image has enough capacity to encode\n");
            if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("\n");
                printf("\033[32mSuccessfully copied the header..\033[0m\n");

                // Encoding magic string
                if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("\033[1;2;32mMagic String is encoded successfully\033[0m\n");
                    if(encode_size(strlen(".txt"), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                    {
                        printf("\n");
                        printf("Sucessfully encoded the secret file extension size\n");
                        sleep(1);
                        if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            printf("Sucessfully encoded the secret file extension\n");
                            if(encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("\n");
                                printf("Sucessfully encoded the secret file size!\n");
                                printf("\n");
                                if(encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("Encoded Secret data ***\n");
                                    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                                    {
                                        printf("\nCopied Remaining RGB data successfully!\n");
                                    }
                                    else
                                    {
                                        printf("\nFailed to copy the remaining RGB data\n");
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("Failed to encode secret data ***\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("\n");
                                printf("Failed to encode the secret file size!\n");
                            }
                        }
                        else
                        {
                            printf("Failed to encode the secret file extension\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("\n");
                        printf("Failed to encode the secret file extension size\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("\n");
                    printf("\033[1;2;31mFailed to encode the magic string\033[0m\n");
                    return e_failure;
                }
            }
            else
            {
                printf("\033[31mFailed to copy the header..\033[0m\n");
                return e_failure;
            }
        }
        else
        {
            printf("Sorry, RGB Data is not enough to encode\n");
            return e_failure;
        }
    }
    else
    {
        printf("Failed to open the files\n");
        return e_failure;
    }
    return e_success;
}