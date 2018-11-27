//
// Created by Rob on 28/07/2018.
//

#include <locale.h>
#include <stdlib.h>
#include "screen.h"

static void write_pixel(bool b) {
    if (b) attron(COLOR_PAIR(1));
    printw(" ");
    attroff(COLOR_PAIR(1));
}

static void setup_color(void) {
    if (has_colors() == false) {
        fprintf(stderr, "Your terminal does not support colour. What year is this?");
        exit(EXIT_FAILURE);
    }

    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
}

int do_screen()
{
    setlocale(LC_ALL, "");
    initscr();			/* Start curses mode 		  */
//    printw("Hello World !!!");	/* Print Hello World		  */
//    printw("foo %c    ", 219);

    setup_color();

    write_pixel(true);
    write_pixel(false);
    write_pixel(true);
    write_pixel(false);
    write_pixel(true);
    write_pixel(false);
    write_pixel(true);
    write_pixel(false);
    write_pixel(true);
    write_pixel(false);

    refresh();			/* Print it on to the real screen */
    getch();			/* Wait for user input */
    endwin();			/* End curses mode		  */

    return 0;
}