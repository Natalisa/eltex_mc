#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <curses.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

struct files_inf{
  char name[250];
  int type;
}mass[250];
//char mass[250][250];
int i = 0, w = 0, j;

void sig_winch(int signo){
  struct winsize size;
  ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
  resizeterm(size.ws_row, size.ws_col);
}

void dir(char *path){//Считывание дериктории
  FILE *f = fopen("test.txt","w");
  i = 0;
  DIR *fd = opendir(path);
  struct dirent *q;
  for(j = 0;j < 250; j++){
    strcpy(mass[j].name, " ");
  }
  while((q = readdir(fd)) != NULL){
    if(strcmp(q->d_name,".") != 0){
      strcpy(mass[i].name, q->d_name);
      mass[i].type = q->d_type;
      fprintf(f,"%s %d\n", mass[i].name, mass[i].type );
      i++;
    }
  }
  closedir(fd);
  fclose(f);
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
  for(j = i - 1; j >= 0; j--){
    wprintw(leftwnd," %s\n",mass[j].name);
  }

  //Рабочее поле правое
  rightwnd = derwin(wnd, size.ws_row, (int)size.ws_col/2, 0, (int)size.ws_col/2);
  // box(rightwnd,'|','-');
  scrollok(rightwnd,true);
  for(j = i - 1; j >= 0; j--){
    wprintw(rightwnd," %s\n",mass[j].name);
  }

  wrefresh(wnd);
  wrefresh(leftwnd);
  wrefresh(rightwnd);

  WINDOW *activwnd=leftwnd;
  int col = 0, row = 0;
  int temp, activ = 1, nstr = i - 1;

  char path[512],pathleft[512] = "./",pathright[512] = "./";
  strcpy(path,pathleft);
//СЧИТЫВАНИЕ
  while(true){
    wclear(activwnd);
    for(j = i-1;j >=0 ; j--){
      if(j == nstr){
        wattroff(activwnd,COLOR_PAIR(0));
        wattron(activwnd,COLOR_PAIR(1));
      }
      wprintw(activwnd," %s\n",mass[j].name);
      if(j == nstr){
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
        if(mass[nstr].type == 4) { //если папка
          sprintf(path,"%s%s/",path,mass[nstr].name);
          dir(path);
          nstr = i - 1;
        } else {  //если файл
          int fd[2];
          pipe(fd);
          switch(fork()){
            case 0: {
              dup2(fd[1],1);
              close(fd[0]);
              execlp("ls","ls","-l",mass[nstr].name,NULL);
            } break;
            default:{
              close(fd[1]);
              char buf[255];
              read(fd[0],buf,255);
              if(buf[3] == 'x'){ //исполняемый
                switch(fork()){
                  case 0:
                    close(fd[0]);
                    execlp(mass[nstr].name,mass[nstr].name,NULL);
                  break;
                  default:
                    close(fd[0]);
                  break;
                }
              } else { //все остальные(редактор)
                switch(fork()){
                  case 0:
                    execlp("../editor/editor","editor",mass[nstr].name,NULL);
                  break;
                  default:
                    wait(NULL);
                  break;
                }
              }
            } break;
          }
        }
      }; break;

      case 9:{//Смена рабочего экрана
        if(activ == 1){
          activwnd=rightwnd;
          strcpy(pathleft,path);
          strcpy(path,pathright);
          activ = 0;
        } else {
          activwnd=leftwnd;
          strcpy(pathright,path);
          strcpy(path,pathleft);
          activ = 1;
        }
        dir(path);
        nstr = i - 1;
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
