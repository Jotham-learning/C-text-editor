#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <ncurses.h>

#include "gapbuffer.h"

struct editorconfig {
    size_t cur_x;
    size_t cur_y;
} E;



void configEditor(Text *text){
    E.cur_x = E.cur_y = 0;
    text-> capacity = 2;
    text-> size = 0;
    text-> lines = calloc(text->capacity,sizeof(Gapbuffer*));
    insertline(text,E.cur_y);
}

Gapbuffer* createLine(){
    Gapbuffer *gbuf = (Gapbuffer*) malloc(sizeof(Gapbuffer));
    gbuf->buffSize =5;
    gbuf->buffer = calloc(gbuf->buffSize,sizeof(char));
    if (gbuf->buffer == NULL) {
        perror("Failed to allocate buffer for the line");
        exit(1);
    }
    gbuf->gap_start = 0;
    gbuf->gap_end = gbuf->buffSize;
    gbuf->gap_size = gbuf->gap_end - gbuf->gap_start;
    gbuf->size = gbuf->buffSize - gbuf->gap_size;
    memset(gbuf->buffer,'_',gbuf->buffSize);

    return gbuf;
}
int insertline(Text *text,size_t index){
    if(index > text->size) return 1;
    if(text->size == text->capacity){
        resizeText(text);
    }
    for(size_t i = text->size ;i > index; i--){// shifting
        text->lines[i] = text->lines[i-1];
    }
    text->lines[index] = createLine();
    text->size +=1;
    return 1;
}
int resizeText(Text *text){
    text->capacity *= 2;
    Gapbuffer **temp = realloc(text->lines,(text->capacity)*sizeof(Gapbuffer*));
    if(temp == NULL){
        return 1;
    }
    text -> lines = temp;
    return 0;
}
void destroyText(Text *text){
    for (size_t i = 0; i < text->size; i++){
        if(text->lines[i] != NULL){
            free(text->lines[i]->buffer);
            free(text->lines[i]);
        }
    }
    free(text->lines);
    
}
// gapbuffer manipluation
int move_left(Gapbuffer *buff,size_t delta){
    if( buff->gap_start >= delta ){
        
        
        memmove(&buff -> buffer[ ((buff -> gap_end) - delta)], 
        &buff -> buffer[ ((buff -> gap_start) - delta)], 
        sizeof(char)*delta);
        
        
        
        buff->gap_start -= delta;
        buff->gap_end = buff->gap_start + buff->gap_size;
        return 0;
    }else{
        return 1;
    }
}

int move_right(Gapbuffer *buff,size_t delta){
    if( ((buff->gap_end + delta ) <= buff->buffSize)){
        
        
        memmove(&buff -> buffer[ ((buff -> gap_start))], 
        &buff -> buffer[ ((buff -> gap_end))], 
        sizeof(char)*delta);
        
        
        
        buff->gap_start += delta;
        buff->gap_end = buff->gap_start + buff->gap_size;
        return 0;
    }else{
        return 1;
    }
}

void resizeBuffer(Gapbuffer *buff, size_t extraSpace){
    
    char *temp;
    int tail = buff->buffSize - buff->gap_end;
    
    temp = realloc(buff->buffer,buff->buffSize + extraSpace);
    if(temp == NULL){
        endwin();
        fprintf(stderr, "Realloc failed\n");
        free(buff->buffer); // Free old memory if realloc failed
        exit(1);
    }
    buff->buffer = temp;
    memmove(
        &buff->buffer[buff->gap_end+extraSpace],
        &buff->buffer[buff->gap_end],
        tail
    );
    buff->gap_end += extraSpace;
    buff->gap_size += extraSpace;
    buff->buffSize += extraSpace;
    
    
    
}

int insert(Gapbuffer *buff,char ch, size_t index){
    
    if (index < buff->buffSize){
        if(buff->gap_size <= 1){
            resizeBuffer(buff,10);
        }
        int delta = index - buff->gap_start;
        // to move gapbuffer
        if (delta > 0){
            move_right(buff, delta);
        }
        else{
            move_left(buff, abs(delta));  
        }
        buff->buffer[buff->gap_start] = ch;
        buff->gap_start += 1;
        buff->gap_size -= 1;
        buff->size +=1;
        return 0;
    }else{
        return 1;
    }
}

int delete(Gapbuffer *buff,size_t index){
    if ( index <= buff->size){
        int delta = index - buff->gap_start;
        // to move gapbuffer
        if (delta > 0){
            move_right(buff, delta);
        }
        else{
            move_left(buff, abs(delta));  
        }
        buff->gap_start -=1;
        buff->gap_size +=1;
        buff->size -=1;
        return 0;
    }else{
        return 1;
    }
}


// editor stuff
void printText(Text *text){

    clear();
    move(0,0);
    size_t i;
    Gapbuffer *buff;
    for(size_t j = 0;j < text->size;j++){
        buff = text->lines[j];

        for (i = 0; i < buff->gap_start; i++){
            addch(buff->buffer[i]);
        }
        for (i = buff->gap_end; i < buff->buffSize; i++){
            addch(buff->buffer[i]);  
        }
        addch('\n');
    }
    refresh();

}

void processKeyPress(int ch,Text *text) {
    getyx(stdscr,E.cur_y,E.cur_x);
    Gapbuffer *prevline = text->lines[E.cur_y-1];
    Gapbuffer *currentLine = text->lines[E.cur_y];// get the current line
    Gapbuffer  *nextline = text->lines[E.cur_y+1];;
    
    if (32 <= ch && ch <= 126) {
        insert(currentLine,(char)ch,E.cur_x);
        move(E.cur_y, ++E.cur_x); 
    }else{
        switch(ch)
        {
            case KEY_RIGHT:
                if(E.cur_x == currentLine->size) break;
                move(E.cur_y,++(E.cur_x));
                break;
            case KEY_LEFT:
                if(E.cur_x == 0) break;
                move(E.cur_y,--E.cur_x);
                break;
            case KEY_UP:
                if(E.cur_y == 0) break;
                if(E.cur_x > prevline->size){
                    E.cur_x = prevline->size;
                    move(--E.cur_y,E.cur_x);
                    break;
                }
                move(--E.cur_y,E.cur_x);
                break;
            case KEY_DOWN:
                if(E.cur_y + 1 >= text -> size) break;
                if(E.cur_x > nextline->size){
                    E.cur_x = nextline->size;
                    move(++E.cur_y,E.cur_x);
                    break;
                }
                break;
            case KEY_BACKSPACE:
                if(E.cur_x == 0) break;
                delete(currentLine,E.cur_x);
                move(E.cur_y,--E.cur_x);
                break;

            case 10:  // ASCII newline '\n'
            case 13:  // Carriage return '\r'
            case KEY_ENTER:
                insertline(text,++E.cur_y);
                E.cur_x = 0;
                move(E.cur_y,E.cur_x);
                break;
            default:
                break;

        }
    }
}

int putTofile(Text *text,char *FileName){
    FILE *fp;
    fp = fopen(FileName,"w");
    if (fp == NULL){
        perror("error oprning file");
        return 1;
    }
    for(size_t i = 0;i< text->size;i++ ){
        Gapbuffer *buff = text ->lines[i];
        fwrite(buff->buffer,sizeof(char), buff->gap_start ,fp);
        fwrite(&buff->buffer[buff->gap_end], sizeof(char),buff->buffSize-buff->gap_end,fp);
        fputc('\n',fp);
    }
    fclose(fp);
    return 0;
}

int main(int argc, char *argv[]) {
    if(argc < 2 ){
        printf("Usage: %s filename\n", argv[0]);
        return 1;
    }
    Text *text = (Text*) malloc(sizeof(Text));
    if (text == NULL) {
        perror("Text malloc failed");
        exit(1);
    }   
    configEditor(text);
    int ch = 0;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE); // for arrow keys

    while (1) {
        
        ch = getch();
        if(ch == 'q') break;
        processKeyPress(ch, text);
        move(0, 0);
        
        for(size_t i = 0;i<text->size;i++){
            printText(text);
        }
        move(E.cur_y,E.cur_x);
        refresh();
        

    }
    putTofile(text,argv[1]);
    destroyText(text);
    endwin();
}