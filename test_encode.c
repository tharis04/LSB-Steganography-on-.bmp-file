#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include <string.h>
#include <unistd.h> // for sleep()

int main(int argc, char *argv[])
{

    printf("\n");
    printf("=================================================\n");
    printf("        \033[1;33mLSB Steganography - Tharis Selvaraj\033[0m\n");
    printf("=================================================\n");

    // check the operational type (encoding or decoding)

    if(check_operation_type(argv) == e_encode)
    {
        EncodeInfo encInfo;
        printf("\033[32m*** Selected Encoding ***\033[0m\n");
        printf("=================================================\n");

        // Validating the input files like .bmp and .txt files

        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            encode_validate_progess();
            printf("\n");
            printf("\033[32mRead and validate encode arguments is successful\033[0m\n");
            printf("=================================================\n");
            
            // Encoding the data
            if(do_encoding(&encInfo) == e_success)
            {
                sleep(1);
                printf("=================================================\n");
                printf("Encoding...\n");
                for (int i = 0; i <= 100; i += 2)
                {
                    printf("\r");
                    for (int j = 0; j < 50; j++)
                        printf(j < i / 2 ? "█" : " ");
                    printf(" %d%%", i);
                    fflush(stdout);
                    usleep(100000);
                }
                printf("\n");
                printf("\n");
                printf("=================================================\n");
                printf("\033[1;2;32mEncoding Completed..\033[0m\n");
                printf("=================================================\n");
            }
            else
            {
                sleep(1);
                printf("Encoding Failed...\n");
            }

        }
        else
        {
            printf("\033[31mFailed to validate the input arguments\033[0m\n");
        }
        
        
    }
    else if(check_operation_type(argv) == e_decode)
    {
        DecodeInfo decInfo;
        printf("\033[32m*** Selected Decoding ***\033[0m\n");
        printf("=================================================\n");

        // Validate the stego .bmp file and the optional output file name
        if(read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            decode_validate_progess();  // optional: your fake progress bar (takes ~10 sec)
            printf("\033[32mRead and validate decode arguments is successful\033[0m\n");
            printf("=================================================\n");

            // Decoding the data
            if(do_decoding(&decInfo) == e_success)
            {
                printf("=================================================\n");
                printf("\033[1;2;32mDecoding Completed..\033[0m\n");
                printf("=================================================\n");
            }
            else
            {
                printf("Decoding Failed...\n");
            }
        }
        else
        {
            printf("\033[31mFailed to validate the input arguments\033[0m\n");
        }
    }
    else
    {
        printf("\033[31m*** Invalid option ***\033[0m\n");
        printf("=================================================\n");
        printf("\033[1;33mUSAGE OF CLA\033[0m\n");
        printf("Encoding: ./a.out -e beautiful.bmp secret.txt\n");
        printf("Decoding: ./a.out -d stego.bmp\n");
        printf("=================================================\n");
    }
    printf("\n");
    
    return 0;
}
OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1],"-e") == 0)
    {
        return e_encode;
    }
    else if(strcmp(argv[1],"-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}

void encode_validate_progess()
{
    for(int i=0 ; i <= 100 ; i++)
    {
        printf("Encoded Arguments --> Validation status: %d%%\r", i);
        fflush(stdout);
        usleep(100000); // microseconds

        printf("\r%-50s\r", " ");
        fflush(stdout);
        
    }
}

void decode_validate_progess()
{
    for(int i=0 ; i <= 100 ; i++)
    {
        printf("Decoded Arguments --> Validation status: %d%%\r", i);
        fflush(stdout);
        usleep(100000); // microseconds


        printf("\r%-50s\r", " ");
        fflush(stdout);
    }
}