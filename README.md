# PixelVault

### C-Based Image Steganography & Steganalysis Toolkit

PixelVault is a command-line toolkit written entirely in C for
hiding and extracting data inside BMP images using
Least Significant Bit (LSB) steganography.

The project also provides image analysis, comparison,
basic steganalysis, and forensic analysis capabilities.

---

## 1. Overview

Digital images contain a large number of pixel bytes.

A small change in the Least Significant Bit (LSB) of a pixel
usually causes only a very small change in the visual appearance
of the image.

PixelVault uses this property to embed data inside images.

For example:

Original byte:

10110110

After embedding a bit:

10110111

Only the last bit has changed.

PixelVault uses this technique to hide:

- Text files
- Images
- WAV audio files

inside uncompressed 24-bit BMP images.

---

## 2. Objectives

The main objectives of PixelVault are:

1. Understand how image data is represented at the byte level.
2. Implement LSB-based steganography in C.
3. Hide and extract binary files from BMP images.
4. Analyze LSB distributions.
5. Compare original and modified images.
6. Generate basic forensic evidence about pixel modifications.
7. Demonstrate practical applications of C file handling,
   structures, bitwise operations, and binary data processing.

---

## 3. Key Features

### Steganography

- Hide text files inside BMP images
- Hide JPG/PNG images inside BMP images
- Hide WAV audio files inside BMP images
- Extract hidden files
- Preserve original payload filename
- Store payload size
- Store payload type
- Verify extracted data using checksum

### Image Analysis

- BMP validation
- Image dimensions
- Color depth
- Pixel data size
- LSB distribution
- LSB balance
- LSB transition analysis

### Comparison

- Compare two BMP images
- Count changed pixel bytes
- Calculate percentage of changed bytes
- Calculate maximum difference
- Calculate average difference
- Detect LSB changes

### Forensic Analysis

- Compare original and suspect images
- Identify pixel-level modifications
- Measure LSB modification rate
- Determine whether changes are consistent with
  LSB-level modification

---

## 4. Supported Format

### Carrier Image

Currently supported:

- BMP
- 24-bit
- Uncompressed

### Payload Types

Supported payloads:

| Type | Supported Format |
|------|------------------|
| Text | `.txt` |
| Image | `.jpg`, `.jpeg`, `.png` |
| Audio | `.wav` |

The payload itself is treated as binary data.

PixelVault does not need to decode the payload contents.

---

## 5. How PixelVault Works

The basic embedding process is:

```text
Payload File
     |
     v
Read binary bytes
     |
     v
Create PixelVault metadata
     |
     v
Convert bytes into bits
     |
     v
Modify BMP pixel LSBs
     |
     v
Stego BMP