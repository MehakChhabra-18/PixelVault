CC = gcc

CFLAGS = -Wall -Wextra -Iinclude

TARGET = pixelvault.exe

SRC = src/main.c \
      src/bmp.c \
      src/pixel.c \
      src/encoder.c \
      src/extractor.c \
      src/payload.c \
      src/compare.c \
      src/detect.c \
      src/forensic.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	del /Q $(TARGET) 2>nul