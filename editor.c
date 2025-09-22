#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include "gapbuffer.h"


struct editorconfig {
    size_t cur_x;
    size_t cur_y;
    size_t scroll_offset;
    char *FileName;

    bool redraw_linenos; 
    WINDOW *TitleBar;
    WINDOW *TextScreen;
    WINDOW *lineno;
    // WINDOW *option;
} E;



void configEditor(Text *text,char *filename){

    E.cur_x = E.cur_y = 0;
    E.scroll_offset = 0;
    E.FileName = filename;
    E.redraw_linenos = TRUE;
    text-> capacity = 5;
    text-> size = 0;
    text-> lines = calloc(text->capacity,sizeof(Gapbuffer*));
    
    
    if(text->lines == NULL){ // Added a check for calloc failure
        free(text);
        perror("Failed to allocate memory for lines");
        exit(1);
    }

    //Title bar because i only need to draw it once

    int lineNo_width = 3;
    E.TitleBar = newwin(1, COLS, 0, 0);
    E.lineno = newwin(LINES-1, lineNo_width, 1, 0);
    E.TextScreen = newwin(LINES-1, COLS-lineNo_width, 1, lineNo_width);
    int col = getmaxx(stdscr);
    // title bar
    int center = (col-strlen(E.FileName)) / 2;

    wattron(E.TitleBar,A_BOLD);
	 if(!has_colors()) { // without white

        mvwaddnstr(E.TitleBar,0, center,E.FileName,strlen(E.FileName));
        wrefresh(E.TitleBar);

    }else{//with white color

        start_color();
        init_pair(1,COLOR_BLACK,COLOR_WHITE);
        wbkgd(E.TitleBar,COLOR_PAIR(1));
        mvwaddnstr(E.TitleBar,0, center,E.FileName,strlen(E.FileName));
        wrefresh(E.TitleBar);
        
    }
    wattroff(E.TitleBar,A_BOLD);

    DrawScreen(NULL);

    insertLine(text,0);
    FILE *fp = fopen(filename,"r");
    if (fp != NULL) {
        int ch;
        int line =0;
        int index = 0;
        while((ch = fgetc(fp)) != EOF){
            if(ch == '\n'){
                line +=1;
                E.cur_y+=1;
                E.cur_x=0;
                insertLine(text,line);
                index = 0;
                continue;
            }else{
                insert(text->lines[line],(char)ch,index);
                E.cur_x+=1;
                index+=1;
            }
        }
        fclose(fp);
    }
}
void DrawTilde() {
    werase(E.lineno);
    for (int row = 0; row < LINES; row++) {
        mvwaddch(E.lineno,row,0,'~');
    }

}
void DrawScreen(Text *text){
    if (E.redraw_linenos) {
        DrawTilde();
        E.redraw_linenos = false;
    }
    if(text == NULL) return; 
    else{
        // Draw text
        int row = getmaxy(E.TextScreen);
        werase(E.TextScreen);
        wmove(E.TextScreen,0,0);
        size_t i;
        Gapbuffer *buff;
        for(size_t j = E.scroll_offset; j < text->size && j < row + E.scroll_offset; j++){
            buff = text->lines[j];
    
            for (i = 0; i < buff->gap_start; i++){
                waddch(E.TextScreen,buff->buffer[i]);
            }
            for (i = buff->gap_end; i < buff->buffSize; i++){
                waddch(E.TextScreen,buff->buffer[i]);  
            }
            if(j < text -> size - 1) waddch(E.TextScreen,'\n');
        }
        if(E.scroll_offset >0 ){
            wmove(E.TextScreen,E.cur_y,E.cur_x);
        }else{
            wmove(E.TextScreen,E.cur_y-E.scroll_offset,E.cur_x);
        }
        
        wnoutrefresh(E.lineno);
        wnoutrefresh(E.TextScreen);
        doupdate();
    }


}


// text maniulation

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


    return gbuf;
}

int insertLine(Text *text,size_t index){
    if(index > text->size) return 1;
    if(text->size == text->capacity){
        resizeText(text);
    }
    for(size_t i = text->size ;i > index; i--){// shifting
        text->lines[i] = text->lines[i-1];
    }
    text->lines[index] = createLine();
    text->size +=1;
    return 0;
}
int deleteLine(Text *text,size_t index){
    if(index> text->size) return 1;
    free(text->lines[index]->buffer);
    free(text->lines[index]);
    for(size_t i = index;i < text->size - 1;i++){
        text->lines[i] = text->lines[i+1];
    }

    text->size -=1;
    return 0;
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
    free(text);
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
        fprintf(stderr, "Resizing buffer : Realloc failed\n");
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
    
    if (index <= buff->size){
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
    if (index == 0 || index > buff->size) return 1;
    int delta = index - buff->gap_start;
    
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
}

void processKeyPress(int ch,Text *text) {
    size_t row = (size_t)getmaxy(E.TextScreen) - 1;
    getyx(E.TextScreen,E.cur_y,E.cur_x);
    //size_t row=getmaxy(E.TextScreen);
    Gapbuffer *prevline;
    Gapbuffer *currentLine = text->lines[E.cur_y+E.scroll_offset];// get the current line
    Gapbuffer  *nextline;
    
    if (32 <= ch && ch <= 126) {
        insert(currentLine,(char)ch,E.cur_x);
        E.cur_x++;
    }else{
        switch(ch)
        {
            case KEY_RIGHT:
                if(E.cur_x == currentLine->size) break;
                E.cur_x++;
                break;
            case KEY_LEFT:
                if(E.cur_x == 0) break;
                E.cur_x--;
                break;
            case KEY_UP:
                if(E.cur_y == 0 && E.scroll_offset ==0) break;
                prevline = text->lines[(E.cur_y+E.scroll_offset-1)];
                if(E.cur_y == 0){
                    E.scroll_offset--;
                } 
                if(E.cur_x > prevline->size){
                    E.cur_x = prevline->size;
                    E.cur_y--;
                    break;
                }
                if(E.cur_y != 0)E.cur_y--;
                break;
            case KEY_DOWN:
                if(E.cur_y + E.scroll_offset + 1 >= text -> size) break;
                nextline = text->lines[E.cur_y+E.scroll_offset+1];
                if(E.cur_y == row &&  E.cur_y + E.scroll_offset < text->size){
                    E.scroll_offset++; 
                }
                if(E.cur_x > nextline->size){
                    E.cur_x = nextline->size;
                    E.cur_y++;
                    break;
                }
                if(E.cur_x != row) E.cur_y++;
                break;
            case KEY_BACKSPACE:{
                if(E.cur_x == 0 && E.cur_y + E.scroll_offset != 0){
                    prevline = text->lines[(E.cur_y+E.scroll_offset)-1];

                    int old_size = prevline->size;

                    if(currentLine->size != 0){


                        move_right(prevline,(prevline->size - prevline->gap_start));
                        

                        size_t beforeGap = currentLine->gap_start;
                        size_t afterGap  = currentLine->buffSize - currentLine->gap_end;
                        size_t totalToMove = beforeGap + afterGap;


                        if (prevline->gap_size < totalToMove) {
                            resizeBuffer(prevline, totalToMove - prevline->gap_size);
                        }


                        memmove(&prevline->buffer[prevline->gap_start],
                                currentLine->buffer,
                                beforeGap);

                        memmove(&prevline->buffer[prevline->gap_start + beforeGap],
                                &currentLine->buffer[currentLine->gap_end],
                                afterGap);

                        prevline->gap_start += totalToMove;
                        prevline->gap_size  -= totalToMove;
                        prevline->size      += totalToMove;
                    }
                    deleteLine(text,E.cur_y+E.scroll_offset);
                    if(E.cur_y == 0){
                        E.scroll_offset--;
                    }else{
                        E.cur_y--;
                    }
                    E.cur_x = old_size;


                }
                else if(E.cur_x != 0){
                    delete(currentLine,E.cur_x--);
                    break;
                }
                break;
            }
            case '\t':
                for (int i = 0; i < 4; i++) {
                    insert(currentLine, ' ', E.cur_x);
                    E.cur_x++;
                }
                break;
            case 10:  // ASCII newline '\n'
            case 13:  // Carriage return '\r'
            case KEY_ENTER:{
                if(E.cur_x <= currentLine->size ){
                    int delta = E.cur_x - currentLine->gap_start;
                    if (delta > 0) {
                        move_right(currentLine, delta);
                    } else {
                        move_left(currentLine, abs(delta));
                    }
    
                    insertLine(text, E.cur_y + E.scroll_offset + 1);
                    nextline = text->lines[E.cur_y + E.scroll_offset + 1];
    
                    size_t amountToMove = currentLine->buffSize - currentLine->gap_end;
    
                    if (nextline->gap_size < amountToMove) {
                        resizeBuffer(nextline, amountToMove - nextline->gap_size);
                    }
    
    
                    memmove(&nextline->buffer[nextline->gap_start],
                            &currentLine->buffer[currentLine->gap_end],
                            amountToMove);
    
    
                    nextline->gap_start += amountToMove;
                    nextline->gap_size  -= amountToMove;
                    nextline->size      += amountToMove;
    
    
                    currentLine->gap_end = currentLine->buffSize;
                    currentLine->gap_size = currentLine->buffSize - currentLine->gap_start;
                    currentLine->size = currentLine->gap_start;
    
                    E.cur_x = 0;
                    if(E.cur_y != row) E.cur_y++;
                    else {
                        E.scroll_offset++;
                    }
                    break;
                }else{
                    insertLine(text, E.cur_y + E.scroll_offset + 1);
                    E.cur_x = 0;
                    if(E.cur_y != row) E.cur_y++;
                    else {
                        E.scroll_offset++;
                    }                
                }
                break;
            }


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
        if(i < text->size-1)fputc('\n',fp);
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
    initscr();
    configEditor(text,argv[1]);
    
    cbreak();
    noecho();
    keypad(E.TextScreen, TRUE); // for arrow keys
    
    
    DrawScreen(text);
    
    int ch = 0;
    while (1) {
        
        ch = wgetch(E.TextScreen);
        if(ch == 24) break;// cntrl-X
        processKeyPress(ch, text);
        DrawScreen(text);


    }
    putTofile(text,argv[1]);
    destroyText(text);
    delwin(E.TextScreen);
    endwin();
}// i wrote this in my own text editor :)