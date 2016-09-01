#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <curses.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

char mass[250][250];
int i=0,w=0,j;

void sig_winch(int signo){
  struct winsize size;
  ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
  resizeterm(size.ws_row, size.ws_col);
}

void dir(char *path){//Считывание дериктории
  i=0;
  DIR *fd = opendir(path);
  struct dirent *q;
  for(j=0;j<250;j++){
    strcpy(mass[j], " ");
  }
  while((q = readdir(fd)) != NULL){
    if(strcmp(q->d_name,".")!=0){
      strcpy(mass[i], q->d_name);
      i++;
    }
  }
  closedir(fd);
}

int main(int argc, char **argv){
  WINDOW *wnd;
  WINDOW *leftwnd;
  WINDOW *rightwnd;
  initscr();
  signal(SIGWINCH, sig_winch);
  cbreak();
  curs_set(0);
  keypad(stdscr,1);
  refresh();
  struct winsize size;
  ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
  wnd = newwin( size.ws_row, size.ws_col, 0, 0);

  start_color();
  init_pair(1,COLOR_BLACK,COLOR_WHITE);

  //Рабочее поле левое
  leftwnd = derwin(wnd, size.ws_row, (int)size.ws_col/2, 0, 0);
  // box(leftwnd,'|','-');
  scrollok(leftwnd,true);
  dir("./");
  for(j=i-1;j>=0;j--){
    wprintw(leftwnd," %s\n",mass[j]);
  }

  //Рабочее поле правое
  rightwnd = derwin(wnd, size.ws_row, (int)size.ws_col/2, 0, (int)size.ws_col/2);
  // box(rightwnd,'|','-');
  scrollok(rightwnd,true);
  for(j=i-1;j>=0;j--){
    wprintw(rightwnd," %s\n",mass[j]);
  }

  wrefresh(wnd);
  wrefresh(leftwnd);
  wrefresh(rightwnd);

  WINDOW *activwnd=leftwnd;
  int col = 0, row = 0;
  int temp,activ=1,nstr=i-1;

  char path[512],pathleft[512] = "./",pathright[512] = "./";
  strcpy(path,pathleft);
//СЧИТЫВАНИЕ
  while(true){
    wclear(activwnd);
    for(j=i-1;j>=0;j--){
      if(j==nstr){
        wattroff(activwnd,COLOR_PAIR(0));
        wattron(activwnd,COLOR_PAIR(1));
      }
      wprintw(activwnd," %s\n",mass[j]);
      if(j==nstr){
        wattroff(activwnd,COLOR_PAIR(1));
        wattron(activwnd,COLOR_PAIR(0));
      }
    }
    // применение изменения окна
    wrefresh(activwnd);

    temp = (int)getch();
    //printf("%d ",temp);
    switch (temp){
      case 10: {//ENTER
        sprintf(path,"%s%s/",path,mass[nstr]);
        dir(path);
        nstr=i-1;
      }; break;

      case 9:{//Смена рабочего экрана
        if(activ==1){
          activwnd=rightwnd;
          strcpy(pathleft,path);
          strcpy(path,pathright);
          activ=0;
        } else {
          activwnd=leftwnd;
          strcpy(pathright,path);
          strcpy(path,pathleft);
          activ=1;
        }
        dir(path);
        nstr=i-1;
       }; break;

      case KEY_UP: {
        if(nstr < i-1)
          nstr++;
        //wrefresh(activwnd);
      }; break;

      case KEY_DOWN: {
        if(nstr > 0)
          nstr--;
        //wrefresh(activwnd);
      }; break;

      case 24: {//CTRL+X
        delwin(leftwnd);
        delwin(rightwnd);
        delwin(activwnd);
        delwin(wnd);
        move(9, 0);
        refresh();
        endwin();
        exit(EXIT_SUCCESS);
      }; break;
    }
  }
}