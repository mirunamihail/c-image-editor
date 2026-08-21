# Image Editor

A command-line image editor written in C that supports image
loading, editing, filtering, and saving.

## Features

- Supports grayscale and color images.
- Supports P2, P3, P5, and P6 Netpbm image formats.
- Loads and saves images in ASCII or binary format.
- Supports selecting a custom rectangular region or the full image.
- Supports rotating images or selected square regions.
- Supports cropping an image to the current selection.
- Supports grayscale histogram calculation and visualization.
- Supports histogram equalization for grayscale images.
- Supports image filters using 3x3 convolution kernels.
- Manages image memory dynamically using malloc and free.

## Image Formats

The editor supports the following Netpbm formats:

- P2 - ASCII grayscale
- P3 - ASCII RGB
- P5 - Binary grayscale
- P6 - Binary RGB

## Commands

### LOAD

Loads an image from the specified file.

    LOAD <filename>

### SELECT

Selects a rectangular region of the image.

    SELECT <x1> <y1> <x2> <y2>

### SELECT ALL

Selects the entire image.

    SELECT ALL

### ROTATE

Rotates the selected region or the entire image.

Supported angles are 90, 180, and 270 degrees.

    ROTATE <angle>

### CROP

Crops the image to the currently selected region.

    CROP

### HISTOGRAM

Calculates and displays the histogram of a grayscale image.

    HISTOGRAM <x> <y>

The number of bins must be a power of two between 2 and 256.

### EQUALIZE

Applies histogram equalization to a grayscale image.

    EQUALIZE

### APPLY

Applies a convolution filter to a color image.

Supported filters:

- EDGE
- SHARPEN
- BLUR
- GAUSSIAN_BLUR

Usage:

    APPLY <filter>

### SAVE

Saves the current image.

    SAVE <filename>

Images can be saved in ASCII format using:

    SAVE <filename> ascii

Without the `ascii` option, the image is saved in binary format.

### EXIT

Frees allocated memory and exits the program.

    EXIT

## Memory Management

The project uses dynamic memory allocation for storing image data.
Separate data structures are used for grayscale and RGB images.

Memory is explicitly allocated when loading or processing images
and released when images are replaced, cropped, rotated, or
when the program exits.

## Implementation

The program is implemented in C and uses standard C libraries
for file handling, memory management, string processing, and
mathematical operations.

The image data is represented using a custom `Image` structure
containing image dimensions, format, pixel data, and selection
coordinates.

Image processing operations are implemented as separate functions,
while the main function parses and executes user commands.

## Technologies

- C
- Dynamic Memory Allocation
- File I/O
- Image Processing
- Histogram Processing
- Convolution Filters
- Netpbm Image Formats
