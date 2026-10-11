# LSB Steganography on .bmp file
Implementation of LSB Image Steganography in C to securely hide and retrieve secret text messages within BMP image files with minimal visual changes.

The aim of this project is to implement **Least Significant Bit (LSB) image steganography** on a `.bmp` (Bitmap) image. The program hides a secret message or text file inside the image by encoding the message bits into the least significant bits of the image's pixel data. The encoded image should look almost the same as the original image.

## Main Operations
- **Encoding:** Hide a secret file inside a BMP image.
- **Decoding:** Extract the hidden file from the encoded BMP image.

## Steganography

Steganography is the practice of concealing a file, message, image, or video within another file, message, image, or video.

In this project, digital steganography is used: secret data is hidden inside the pixel data of an image.

## Use of LSB Bit

LSB stands for **Least Significant Bit**. In an 8-bit binary value, it is the rightmost bit. Changing this bit changes the value by only one, whereas changing the most significant bit can cause a much larger change.

For example:

```text
Original:  10010110  = 150
Modified:  10010111  = 151
```

Only the last bit changed, so the numerical difference is small. By encoding message bits in pixel-component LSBs, the program can hide data while keeping visual changes to the image minimal.

The project requirements describe encoding message bits in the LSBs of the image's RGB data. The exact capacity depends on the image and the encoding information stored by the implementation.

> **Note:** LSB steganography hides the presence of data; it does not, by itself, encrypt the message.

## Pixel

A **pixel** is an individual picture element in a digital raster image. An image consists of rows and columns of pixels.

In a typical color image, each pixel is represented by color components such as **Red (R), Green (G), and Blue (B)**. In a common 24-bit RGB representation, each component uses 8 bits, so each pixel uses 24 bits in total.

For example, an RGB pixel could have the values:

```text
Red   = 255
Green = 100
Blue  = 50
```

Each 8-bit component has its own LSB. A program can store message bits in these least significant bits. The uploaded project document discusses using the LSBs of RGB data for encoding.

## `.bmp` File?

`.bmp` stands for **Bitmap Image File**. BMP is an image-file format that stores information describing an image and its pixel data.

The project document describes BMP files as typically uncompressed and lossless, with a simple structure and broad support in Windows programs. This makes suitable BMP files convenient for a basic LSB steganography implementation because their pixel data can be accessed and modified.

BMP files can have different pixel formats and header types, so an implementation must handle the format it supports correctly.

## Basic BMP File Structure

A BMP file generally contains header information followed by pixel data.

### 1. File Header

The file header includes information such as:

- **File signature:** Identifies the file as a BMP.
- **File size:** The size of the file in bytes.
- **Pixel-data offset:** The location where the pixel data begins.

### 2. DIB (Bitmap Information) Header

The DIB header contains information describing the image, such as:

- Image width and height
- Number of bits per pixel
- Compression method
- Image-size and resolution information

The exact header layout depends on the BMP variant. The project document illustrates a common 54-byte combined header for a basic BMP layout; not every BMP uses that exact header size.

### 3. Pixel Data

The pixel-data section stores the image's pixel information. In a typical 24-bit BMP, each pixel has three 8-bit color components. BMP files commonly store these components in **BGR** byte order, even though colors are often described as RGB.

Some BMP rows include padding bytes, so code must account for row layout rather than assuming every row contains only `width × bytes_per_pixel` bytes.

## How LSB Encoding Works

1. Open the cover BMP image and the secret text file.
2. Read the BMP header and locate the pixel data.
3. Check that the image has enough capacity for the secret data and required encoding information.
4. Encode the project's magic-string signature and secret-file information, as required by the implementation.
5. Convert the secret data into bits and embed those bits in the pixel-data LSBs.
6. Copy the remaining image data and write the encoded BMP file.

## How Decoding Works

1. Open the encoded BMP image.
2. Read the embedded magic-string signature to identify the hidden data.
3. Recover the secret file's extension and size, as implemented in the project.
4. Read the embedded data bits and reconstruct the secret file.
5. Write the recovered data to the output file.

## Command-Line Usage

The project requirements specify the following command forms:

### Encoding

```bash
./lsb_steg -e <.bmp_file> <.text_file> [output_file]
```

Example:

```bash
./lsb_steg -e beautiful.bmp secret.txt
```

If an output filename is not provided, the implementation may use its default output filename.

### Decoding

```bash
./lsb_steg -d <.bmp_file> [output_file]
```

Example:

```bash
./lsb_steg -d steged_img.bmp
```

If an output filename is not provided, the implementation may use its default decoded filename.

## Error Handling

The project requirements identify checks such as:

- Invalid command-line options or missing arguments
- Invalid or unavailable input files
- Empty secret files
- Cover images with insufficient capacity
- Invalid encoding or decoding inputs

The implementation should report appropriate errors when these conditions occur.

## Concepts Used

- C programming
- File handling
- Binary file processing
- Bitwise operations
- BMP image structure
- Command-line arguments
- Encoding and decoding

## Summary

This project demonstrates how secret data can be embedded in a BMP image using the Least Significant Bit technique. Because modifying an LSB causes only a small change to an 8-bit component, the resulting image can retain nearly the same visual appearance. The decoder reads the embedded information to recover the hidden file.

##
Implemented by Tharis Selvaraj, Advanced embedded systems Trainee, Emertxe Information Technologies, Bengaluru.

