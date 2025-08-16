#ifndef GAPBUFFER_H
#define GAPBUFFER_H

typedef struct
{   
    char *buffer;
    size_t gap_start;
    size_t gap_end;
    size_t gap_size;
    size_t buffSize;
    size_t size;
    
}Gapbuffer;
typedef struct Text
{
    Gapbuffer **lines;
    size_t size;
    size_t capacity;
    
}Text;


void configEditor(Text *text);
Gapbuffer* createLine(void);
int insertline(Text *text,size_t index);
void initEditor(void);
void printText(Text *text);
int resizeText(Text *text);
int move_left(Gapbuffer *buff, size_t delta);
int move_right(Gapbuffer *buff, size_t delta);
void resizeBuffer(Gapbuffer *buff, size_t extraSpace);
int insert(Gapbuffer *buff, char ch, size_t index);
int delete(Gapbuffer *buff, size_t index);
void processKeyPress(int ch, Text *);
int putTofile(Text *text,char *FileName);
int main(int argc, char *argv[]);

#endif