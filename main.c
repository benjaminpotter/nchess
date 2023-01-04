// Written by Ben.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

// colors
#define C_WHITE 1
#define C_BLACK 2
#define C_WHITE_PIECE 3
#define C_BLACK_PIECE 4

// game states
#define START 0
#define PLAYING 1
#define PAUSED 2

// all data that needs to be serialized
typedef struct {
    
   int turn; // 0 for p1, 1 for p2 
   int cursorx;
   int cursory;

   int isselected;
   int selectedx;
   int selectedy;

   // game board
   char* pieces;

} game_data; 

WINDOW* w_start;
WINDOW* w_game;
WINDOW* w_exit;
int game_state = PLAYING;

// drawing routines
void draw_start(WINDOW* w) {
    mvwprintw(w, 10, 15, "nChess"); 
    mvwprintw(w, 11, 15, "New Game");
    mvwprintw(w, 12, 15, "Load Game");
    wrefresh(w);
}

void draw_board(WINDOW* w, game_data* gd) {
    int offsety = LINES/2 - 4;
    int offsetx = COLS/2 - 8;

    for(int i = 0; i < 8; ++i) {
        wmove(w, i + offsety, offsetx);
        for(int j = 0; j < 8; ++j) {
            if((j+i) % 2 == gd->turn) {
                wattron(w, COLOR_PAIR(C_WHITE));
                wprintw(w, "@@"); 
                wattroff(w, COLOR_PAIR(C_WHITE));
            } else {
                wattron(w, COLOR_PAIR(C_BLACK));
                wprintw(w, "  ");
                wattroff(w, COLOR_PAIR(C_BLACK));
            }
        }
    }     

    // draw rank and file
    for(int i = 0; i < 8; ++i) {
        wmove(w, i + offsety, offsetx - 1);
        if(gd->turn == 0) {
            wprintw(w, "%d", 8 - i);
        } else {
            wprintw(w, "%d", 1 + i); // a b c d e f g h
        }
    }     

    wmove(w, offsety + 8, offsetx);
    for(int i = 0; i < 8; ++i) {
        if(gd->turn == 0) {
            wprintw(w, "%c ", 97 + i);
        } else {
            wprintw(w, "%c ", 104 - i); // a b c d e f g h
        }
    }     

    mvwprintw(w, offsety-1, offsetx, "%d %d", gd->cursorx, gd->cursory);
    mvwprintw(w, 0, 0, "%s to play", gd->turn == 0 ? "White" : "Black");
    mvwprintw(w, 1, 0, "%s", gd->isselected == 1 ? "Selected" : "");
}

void draw_pieces(WINDOW* w, game_data* gd) {
    int offsety = LINES/2 - 4;
    int offsetx = COLS/2 - 8;

    for(int i = 0; i < 8*8; ++i) {
        if(gd->pieces[i*2] == 'e')
            continue;

        // color
        int c = gd->pieces[i*2] == 'b' ? C_BLACK_PIECE : C_WHITE_PIECE;
        
        // piece
        char p = gd->pieces[i*2 +1];

        wattron(w, COLOR_PAIR(c));
        int y = i/8;
        mvwaddch(w, offsety + y, offsetx + i%8*2, p);  
        wattroff(w, COLOR_PAIR(c));
    }
}

void draw_cursor(WINDOW* w, game_data* gd) {
    int offsety = LINES/2 - 4;
    int offsetx = COLS/2 - 8;
    
    mvwprintw(w, offsety+gd->cursory, offsetx+(gd->cursorx*2)+1, "X");
}

void draw_exit(WINDOW* w) {} 

// game logic routines
void move_piece(game_data* gd) {

    int i,j,di,dj;
    i = (gd->selectedy * 8 + gd->selectedx) * 2;
    j = i + 1;
    di = (gd->cursory * 8 + gd->cursorx) * 2;
    dj = di + 1; 
    
    char piece[2];
    piece[0] = gd->pieces[i];
    piece[1] = gd->pieces[j];

    // replace with empty space
    gd->pieces[i] = 'e';
    gd->pieces[j] = 'e';

    // move to destination
    gd->pieces[di] = piece[0];
    gd->pieces[dj] = piece[1];
}

int main(void) {
    
    initscr();
    start_color();
    use_default_colors();

    noecho();
    cbreak(); 
    curs_set(0);

    // colors
    init_color(COLOR_BLACK, 0, 0, 0);
    init_color(COLOR_CYAN, 500, 500, 500);
    init_pair(C_WHITE, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(C_BLACK, COLOR_GREEN, COLOR_GREEN);
    init_pair(C_BLACK_PIECE, COLOR_WHITE, COLOR_CYAN);
    init_pair(C_WHITE_PIECE, COLOR_CYAN, COLOR_WHITE);

    // initialize windows
    w_start = newwin(LINES, COLS, 0, 0);
    w_game = newwin(LINES, COLS, 0, 0);

    int exitw, exith, exitx, exity;
    exitw = COLS/2;
    exith = LINES/2;
    exitx = COLS/2 - exitw/2;
    exity = LINES/2 - exith/2;
    w_exit = newwin(exith, exitw, exity, exitx); 
    refresh();
    
    game_data* gd = (game_data*) malloc(sizeof(game_data));
    gd->turn = 0;
    gd->cursorx = 0;
    gd->cursory = 0;
    gd->pieces = (char*) malloc(sizeof(char) * 128);
    strcpy(gd->pieces, "brbnbbbKbQbbbnbrbpbpbpbpbpbpbpbpeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeewpwpwpwpwpwpwpwpwrwnwbwQwKwbwnwr"); // this might not work becuase of the null terminator

    int ch;
    while(1) {

        switch(game_state) {
            case START:
                draw_start(w_start);
                break;

            case PLAYING:
                if(ch == 'd')
                    gd->cursorx += 1;
                if(ch == 'a')
                    gd->cursorx += -1;
                if(ch == 'w')
                    gd->cursory += -1;
                if(ch == 's')
                    gd->cursory += 1;
                if(ch == ' ') {
                    if(gd->isselected == 1) {
                        move_piece(gd);
                        //gd->turn = (gd->turn + 1)%2;
                        gd->isselected = 0;
                    } else {
                        if(gd->pieces[(gd->cursory * 8 + gd->cursorx) * 2] != 'e') {
                            gd->selectedx = gd->cursorx;
                            gd->selectedy = gd->cursory;
                            gd->isselected = 1;
                        } 
                    }
                }

                wclear(w_game);
                draw_board(w_game, gd);
                draw_pieces(w_game, gd);
                draw_cursor(w_game, gd);
                wrefresh(w_game);
                break;

            case PAUSED:
                break;

        }

        ch = getch();
    }

    // clean up
    endwin();
    return 0;
}
