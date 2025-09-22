#ifndef GAPBUFFER_H
#define GAPBUFFER_H


#include <stddef.h>   // for size_t

/* -----------------------------
   Data Structures
------------------------------ */

// Gap buffer for each line
typedef struct {
    char *buffer;       // character buffer
    size_t gap_start;   // start of gap
    size_t gap_end;     // end of gap
    size_t gap_size;    // current gap size
    size_t buffSize;    // total buffer size
    size_t size;        // actual text size (characters in use)
} Gapbuffer;

// Text structure: dynamic array of line pointers
typedef struct {
    Gapbuffer **lines;  // array of line pointers
    size_t size;        // number of lines
    size_t capacity;    // allocated capacity
} Text;




// === Editor config ===
void configEditor(Text *text, char *filename);
void DrawTilde(void);
void DrawScreen(Text *text);

// === Text manipulation ===
Gapbuffer* createLine(void);
int insertLine(Text *text, size_t index);
int deleteLine(Text *text, size_t index);
int resizeText(Text *text);
void destroyText(Text *text);

// === Gap buffer manipulation ===
int move_left(Gapbuffer *buff, size_t delta);
int move_right(Gapbuffer *buff, size_t delta);
void resizeBuffer(Gapbuffer *buff, size_t extraSpace);
int insert(Gapbuffer *buff, char ch, size_t index);
int delete(Gapbuffer *buff, size_t index);

// === Input handling ===
void processKeyPress(int ch, Text *text);

// === File I/O ===
int putTofile(Text *text, char *FileName);

// === Main entry point ===
int main(int argc, char *argv[]);


#endif