#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#define  PLAYER_ONE 1
#define  PLAYER_TWO 2

#define N 13
#define M 6

#define  EMPTY     0
#define  SCISSOR   1
#define  PAPER     2
#define  ROCK      3
#define  BOMB      4
#define  RED       5
#define  BLUE      6
#define  TODELETE  7
#define  ITEMS_NUM 6
#define  RED_MARKED 8
#define  BLUE_MARKED 9

void place(int matrix[][M], int current, int player_id);
void send_to_opponent(int matrix[][M], int from, int current);
void setItem(int choise);

int matrix_player1[N][M];
int matrix_player2[N][M];
int x_player1,y_player1 = 0;
int x_player2, y_player2 = 0;
int item_player1;
int item_player2;
int score_player1 = 0;
int score_player2 = 0;
int refreshMills = 4;   //CHANGE TO CHANGE SPEED
int start = 0;
int game_over = 0;
int one_player = 0;
GLuint Scissor_tex;
GLuint Paper_tex;
GLuint Rock_tex;
GLuint Bomb_tex;

unsigned char *Rock;
unsigned char *Scissor;
unsigned char *Paper;
unsigned char *Bomb;

typedef struct neighbors_list{
      int i;
      int j;
      struct neighbors_list *next;
}neighbors_list;

void add_to_list(int i, int j ,neighbors_list **x){
      neighbors_list *temp;
      temp = (neighbors_list *)malloc(sizeof(neighbors_list));
      temp->i = i;
      temp->j = j;
      temp->next = *x;
      *x = temp;
}

void drawText(char*string,int x,int y, float w, float h){
      char *c;
      glPushMatrix();
      glTranslatef(x, y,0);
      glScalef(w,h,0);
      for (c=string; *c != '\0'; c++)
            glutStrokeCharacter(GLUT_STROKE_ROMAN , *c);

      glPopMatrix();
}

void load_items(){
      FILE *Rfile;
      FILE *Sfile;
      FILE *Pfile;
      FILE *Bfile;
      int i;
      char buffer[64];
      Rfile = fopen("rock.pgm", "r");
      Sfile = fopen("scissors.pgm", "r");
      Pfile = fopen("paper.pgm", "r");
      Bfile = fopen("bomb.pgm", "r");

      Rock = (unsigned char*)malloc(1600*sizeof(unsigned char));
      Scissor = (unsigned char*)malloc(1600*sizeof(unsigned char));
      Paper = (unsigned char*)malloc(1600*sizeof(unsigned char));
      Bomb = (unsigned char*)malloc(1600*sizeof(unsigned char));

      //Skip 4 lines
      for(i = 0; i<8; i++){
            fscanf(Rfile, "%s", buffer);
            fscanf(Sfile, "%s", buffer);
            fscanf(Pfile, "%s", buffer);
            fscanf(Bfile, "%s", buffer);
      }

      for(i = 0; i < 1600; i++){
            fscanf(Rfile, "%d", &Rock[i]);
            fscanf(Sfile, "%d", &Scissor[i]);
            fscanf(Pfile, "%d", &Paper[i]);
            fscanf(Bfile, "%d", &Bomb[i]);
      }
}

void setItem(int choise){
      glColor3f(1, 1, 1);
      if(choise == ROCK)
             glBindTexture(GL_TEXTURE_2D, Rock_tex);
      else if(choise == PAPER)
            glBindTexture(GL_TEXTURE_2D, Paper_tex);
      else if(choise == SCISSOR)
             glBindTexture(GL_TEXTURE_2D, Scissor_tex);
      else if(choise == BOMB)
            glBindTexture(GL_TEXTURE_2D, Bomb_tex);
      else if(choise == RED || choise == RED_MARKED){
            glColor3f(0.9, 0, 0);
            glDisable(GL_TEXTURE_2D);
            return;
      }
      else if(choise == BLUE || choise == RED_MARKED){
            glColor3f(0, 0, 0.9);
            glDisable(GL_TEXTURE_2D);
            return;
      }
      else
            return;
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glEnable(GL_TEXTURE_2D);
}

void init(){
	glClearColor(0.2f, 0.1f, 0.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D (0.0, 520.0, -80.0, 520.0);

      glDrawPixels(40, 40, GL_LUMINANCE, GL_UNSIGNED_BYTE, Bomb);
      glGenTextures(1, &Bomb_tex);
      glBindTexture(GL_TEXTURE_2D, Bomb_tex);
      glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 0,0,40,40,0);

      glDrawPixels(40, 40, GL_LUMINANCE, GL_UNSIGNED_BYTE, Scissor);
      glGenTextures(1, &Scissor_tex);
      glBindTexture(GL_TEXTURE_2D, Scissor_tex);
      glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 0, 0, 40, 40, 0);

      glDrawPixels(40, 40, GL_LUMINANCE, GL_UNSIGNED_BYTE, Rock);
      glGenTextures(1, &Rock_tex);
      glBindTexture(GL_TEXTURE_2D, Rock_tex);
      glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 0, 0, 40, 40, 0);

      glDrawPixels(40, 40, GL_LUMINANCE, GL_UNSIGNED_BYTE, Paper);
      glGenTextures(1, &Paper_tex);
      glBindTexture(GL_TEXTURE_2D, Paper_tex);
      glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 0, 0, 40, 40, 0);
}

void find_neighbours(int i, int j, int matrix[][M], neighbors_list **list){
      int color = matrix[i][j];
      if(matrix[i][j] == RED)
            matrix[i][j] = RED_MARKED;
      if(matrix[i][j] == BLUE)
            matrix[i][j] = BLUE_MARKED;

      add_to_list(i,j,list);
      if(i != 0)
            if(matrix[i-1][j] == color)
                  find_neighbours(i-1, j, matrix, list);
      if(i != N - 1)
            if(matrix[i+1][j] == color)
                  find_neighbours(i+1, j, matrix, list);
      if(j != M - 1)
            if(matrix[i][j+1] == color)
                  find_neighbours(i, j+1, matrix, list);
      if(j != 0)
            if(matrix[i][j-1] == color)
                  find_neighbours(i, j-1, matrix, list);

}

void replace(int matrix[][M]){
      int i, j;
      for(i = 0; i < N; i++)
            for(j = 0; j < M; j++){
                  if(matrix[i][j] == RED_MARKED)
                        matrix[i][j] = RED;
                  if(matrix[i][j] == BLUE_MARKED)
                        matrix[i][j] = BLUE;
            }
}

void rearrange_colored(int i, int j, int matrix[][M]){
      int k, min = 13;
      int color = matrix[i][j];
      neighbors_list *list = NULL, *temp;
      find_neighbours(i, j, matrix, &list);
      replace(matrix);

      for(temp = list; temp != NULL; temp = temp->next){
            int count = 0;
            k = temp->i;
            if(matrix[k-1][temp->j] != color){
                  while(matrix[k-1][temp->j] == EMPTY && k != 0){
                        k--;
                        count++;
                  }
                  if(count < min)
                        min = count;
            }
      }
      if(min == 13) min = 0;
      if(min != 0){
            for(temp = list; temp != NULL; temp = temp->next){
                  if(matrix[temp->i - min][temp->j] == EMPTY)
                        matrix[temp->i - min][temp->j] = matrix[temp->i][temp->j];
                  else
                        matrix[temp->i - min][temp->j] += 3;
            }

            for(temp = list; temp != NULL; temp = temp->next)
                  if(matrix[temp->i][temp->j] != color + 3)
                        matrix[temp->i][temp->j] = EMPTY;
            replace(matrix);
      }
}

void rearrange(int matrix[][M]){
      int i, j, k;
      for(i = 1; i < 13; i++)
            for(j = 0; j < 6; j++)
                  if(matrix[i][j] != 0){
                        if(matrix[i][j] == RED || matrix[i][j] == BLUE)
                              rearrange_colored(i,j, matrix);
                        else{
                              k = i;
                              while(matrix[k-1][j] == EMPTY && k != 0)
                                    k--;
                              if(k != i){
                                    matrix[k][j] = matrix[i][j];
                                    matrix[i][j] = EMPTY;
                              }
                        }
                  }

      for(i = 0; i < 6; i++)
            if(matrix[13][i] != EMPTY && matrix[12][i] != EMPTY){
                  game_over = 1;
                  for(j = 0; j < N; j++)
                        for(k = 0; k < M; k++)
                              matrix_player1[j][k]= matrix_player2[j][k] = EMPTY;
            }
}

void randomize(int player_id){
      if(player_id == PLAYER_ONE){
            x_player1 = rand() % M;
            x_player1 = 40 * x_player1 + 20;
            y_player1 = 520;
            item_player1 = 1 + rand()% ITEMS_NUM;
      }
      if(player_id == PLAYER_TWO){
            x_player2 = rand() % M;
            x_player2 = 40 * x_player2 + 300;
            y_player2 = 520;
            item_player2 = 1 + rand() % ITEMS_NUM;
      }
}

void to_be_cleaned_colored(int i, int j, int matrix[][M]){
      int current = matrix[i][j];
      matrix[i][j] = TODELETE;
      if(i != 0)
            if(matrix[i-1][j] == current)
                  to_be_cleaned_colored(i-1, j, matrix);
      if(i != N - 1)
            if(matrix[i+1][j] == current)
                  to_be_cleaned_colored(i+1, j, matrix);
      if(j != M - 1)
            if(matrix[i][j+1] == current)
                  to_be_cleaned_colored(i, j+1, matrix);
      if(j != 0)
            if(matrix[i][j-1] == current)
                  to_be_cleaned_colored(i, j-1, matrix);
}

void to_be_cleaned(int i, int j, int current, int matrix[][M]){
      if(current == BOMB){
            matrix[i][j] = TODELETE;
            if(i != 0)
                  if(matrix[i-1][j] != EMPTY){
                        if(matrix[i-1][j] == BLUE || matrix[i-1][j] == RED)
                              to_be_cleaned_colored(i-1, j, matrix);
                        else
                              matrix[i-1][j] = TODELETE;
                  }
            if(i != N - 1)
                  if(matrix[i+1][j] != EMPTY){
                        if(matrix[i+1][j] == BLUE || matrix[i+1][j] == RED)
                              to_be_cleaned_colored(i+1, j, matrix);
                        else
                              matrix[i+1][j] = TODELETE;
                  }
            if(j != 0)
                  if(matrix[i][j-1] != EMPTY){
                        if(matrix[i][j-1] == BLUE || matrix[i][j-1] == RED)
                              to_be_cleaned_colored(i, j-1, matrix);
                        else
                              matrix[i][j-1] = TODELETE;
                  }
            if(j != M - 1)
                  if(matrix[i][j+1] != EMPTY){
                        if(matrix[i][j+1] == BLUE || matrix[i][j+1] == RED)
                              to_be_cleaned_colored(i, j+1, matrix);
                        else
                              matrix[i][j+1] = TODELETE;
                  }
            return;
      }
      if(current == ROCK){
            if(i != 0)
                  if(matrix[i-1][j] == SCISSOR){
                        matrix[i-1][j] = TODELETE;
                        to_be_cleaned(i-1, j, current, matrix);
                  }
                  if(matrix[i+1][j] == SCISSOR){
                        matrix[i+1][j] = TODELETE;
                        to_be_cleaned(i+1, j, current, matrix);
                  }
            if(j != 0)
                  if(matrix[i][j-1] == SCISSOR){
                        matrix[i][j-1] = TODELETE;
                        to_be_cleaned(i, j-1, current, matrix);
                  }
            if(j != M-1)
                  if(matrix[i][j+1] == SCISSOR){
                        matrix[i][j+1] = TODELETE;
                        to_be_cleaned(i, j+1, current, matrix);
                  }
            return;
      }
      else if(current == PAPER){
            if(i!=0)
                  if(matrix[i-1][j] == ROCK){
                        matrix[i-1][j] = TODELETE;
                        to_be_cleaned(i-1, j, current, matrix);
                  }
                  if(matrix[i+1][j] == ROCK){
                        matrix[i+1][j] = TODELETE;
                        to_be_cleaned(i+1, j, current, matrix);
                  }
            if(j != 0)
                  if(matrix[i][j-1] == ROCK){
                        matrix[i][j-1] = TODELETE;
                        to_be_cleaned(i, j-1, current, matrix);
                  }
            if(j != M-1)
                  if(matrix[i][j+1] == ROCK){
                        matrix[i][j+1] = TODELETE;
                        to_be_cleaned(i, j+1, current, matrix);
                  }
            return;
      }
      else if(current == SCISSOR){
            if(i != 0)
                  if(matrix[i-1][j] == PAPER){
                        matrix[i-1][j] = TODELETE;
                        to_be_cleaned(i-1, j, current, matrix);
                  }
                  if(matrix[i+1][j] == PAPER){
                        matrix[i+1][j] = TODELETE;
                        to_be_cleaned(i+1, j, current, matrix);
                  }
	    if(j != 0)
                  if(matrix[i][j-1] == PAPER){
                        matrix[i][j-1] = TODELETE;
                        to_be_cleaned(i, j-1, current, matrix);
                  }
	    if(j != M - 1)
                  if(matrix[i][j+1] == PAPER){
                        matrix[i][j+1] = TODELETE;
                        to_be_cleaned(i, j+1, current, matrix);
                  }
	    return;
      }

}

void clean(int posi, int posj, int current, int matrix[][M], int player_id){
      int i, j, n = 0;
      int newPosi = 13, newPosj = 0;

      for(i = 0; i < N; i++)
            for(j = 0; j < M; j++)
                  if(matrix[i][j] == TODELETE){
                        if(i < newPosi)
                              newPosi = i;
                        if(j > newPosj)
                              newPosj = j;
                        matrix[i][j] = EMPTY;
                        n++;
                  }

      if(n != 0 && current != BOMB){
            matrix[posi][posj] = EMPTY;
            matrix[newPosi][newPosj] = current;
      }
      if(player_id == PLAYER_ONE)
            score_player1 += 10*n + n*n;
      else
            score_player2 += 10*n + n*n;

}

void place(int matrix[][M], int current, int player_id){
      int r,i;
      r = rand() % M;
      for(i = 0; i < N; i++)
            if(matrix[i][r] == EMPTY){
                  matrix[i][r] = current;
                  to_be_cleaned(i,r,current, matrix);
                  clean(i, r, current, matrix, player_id);
                  return;
            }
}

void send_to_opponent(int matrix[][M], int from, int current){
      int i, j;
      int count = 0;
      for(i = 0; i < N; i++)
            for(j = 0; j < M; j++)
                  if(matrix[i][j] == TODELETE)
                        count++;
      if(current == BOMB)     //BOMB DOESN'T SEND ITEMS TO THE OPPENENT
            return;

      if(current == ROCK)
            current = SCISSOR;
      else if(current == PAPER)
            current = ROCK;
      else
            current = PAPER;

      for(i = 0; i < count; i++){
            if(from == PLAYER_ONE)
                  place(matrix_player2, current, PLAYER_TWO);
            else
                  place(matrix_player1, current, PLAYER_ONE);
      }
}

void print_matrix(int matrix[][M], int player_id){
      int i, j;

      for(i = 0; i<N; i++)
            for(j = 0; j < M; j++)
                  if(matrix[i][j] != EMPTY){
                        setItem(matrix[i][j]);
                        glPushMatrix();
                        if(player_id == PLAYER_ONE)
                              glTranslatef(40*j+20, 40*i+20, 0);
                        else
                              glTranslatef(40*j + 300, 40*i+20, 0);
                              glBegin(GL_QUADS);
                              glTexCoord2i(0.0f, 0.0f);
                              glVertex2i(-20, 20);
                              glTexCoord2i(1.0f, 0.0f);
                              glVertex2i(20, 20);
                              glTexCoord2i(1.0f, 1.0f);
                              glVertex2i(20, -20);
                              glTexCoord2i(0.0f, 1.0f);
                              glVertex2i(-20,-20);
                        glEnd();
                        glPopMatrix();
                  }
}

void stabilize(int matrix[][M], int player_id){
      int i, current;
      if(player_id == PLAYER_ONE){
            i = (19+y_player1)/40;
            if(y_player1 == 20){
                  matrix[0][x_player1/40] = current = item_player1;
                  to_be_cleaned(0, x_player1/40, current, matrix);
                  send_to_opponent(matrix, PLAYER_ONE, current);
                  clean(0, x_player1/40, current, matrix, PLAYER_ONE);
                  rearrange(matrix);
            }
            else if(matrix[i-1][x_player1/40] != EMPTY){
                  matrix[i][x_player1/40] = current = item_player1;
                  to_be_cleaned(i, x_player1/40, current, matrix);
                  send_to_opponent(matrix, PLAYER_ONE, current);
                  clean(i, x_player1/40,current, matrix, PLAYER_ONE);
                  rearrange(matrix);
                  randomize(PLAYER_ONE);
            }
      }
      else{
            i = (19 + y_player2)/40;
            if(y_player2 == 20){
                  matrix[0][(x_player2-300)/40] = current = item_player2;
                  to_be_cleaned(0, (x_player2-300)/40, current, matrix);
                  send_to_opponent(matrix, PLAYER_TWO, current);
                  clean(0, (x_player2-300)/40, current, matrix, PLAYER_TWO);
                  rearrange(matrix);

            }
            else if(matrix[i-1][(x_player2-300)/40] != EMPTY){
                  matrix[i][(x_player2-300)/40] =current = item_player2;
                  to_be_cleaned(i, (x_player2-300)/40, current, matrix);
                  send_to_opponent(matrix, PLAYER_TWO, current);
                  clean(i, (x_player2-300)/40, current, matrix, PLAYER_TWO);
                  rearrange(matrix);
                  randomize(PLAYER_TWO);
            }
      }
}

void start_menu(){
      glutReshapeWindow(520,600);
      glColor3f(255,0, 0);
	drawText("VRAXAPSA", 180, 400, 0.2, 0.2);
      glColor3f(1,1,1);
      drawText("Press: ", 20, 280, 0.16, 0.16);
      drawText("<1> for 1 player", 20, 230, 0.16, 0.16);
      drawText("<2> for 2 players", 20, 210, 0.16, 0.16);
      drawText("Player 1: ", 0, -40, 0.14, 0.14);
      drawText("Score : 0", 0, -80, 0.14, 0.14);
      glutSwapBuffers();
}

void game_over_fun(){
      char text[32];
      glColor3f(1,1,1);
      if(one_player){
            drawText("Game Over", 50, 300, 0.2, 0.2);
            drawText("Press <b> to restart ", 55, 250, 0.1,0.1);
            drawText("or <esc> to exit", 68, 230, 0.1, 0.1);
            sprintf(text, "%d", score_player1);
            drawText("Final Score :", 40, 190, 0.2, 0.2);
            drawText(text, 100, 150, 0.2, 0.2);
            glutSwapBuffers();
      }
      else{
            if(score_player1 > score_player2)
                  drawText("Player 1 wins!", 50, 300, 0.2, 0.2);
            else if(score_player2 > score_player1)
                  drawText("Player 2 wins!", 50, 300, 0.2, 0.2);
            else
                  drawText("Tie!", 50, 300, 0.2, 0.2);

            drawText("Press <b> to restart ", 55, 250, 0.1,0.1);
            drawText("or <esc> to exit", 68, 230, 0.1, 0.1);
            drawText("Final Score :", 50, 190, 0.2, 0.2);
            sprintf(text, "Player 1: %d", score_player1);
            drawText(text, 100, 150, 0.2, 0.2);
            sprintf(text, "Player 2: %d", score_player2);
            drawText(text, 100, 120, 0.2, 0.2);
            glutSwapBuffers();
      }

}

void display(void){
      char text[32];
	glClear(GL_COLOR_BUFFER_BIT);

	if(!start){
	      start_menu();
            return;
	}
	if(game_over){
            game_over_fun();
            return;
	}

      if(!one_player){
            glColor3f(0.6, 0.1, 0.3);
            glPushMatrix();
            glTranslatef(400,120,0);
            glBegin(GL_QUADS);
            glVertex2i(-120, 400);
            glVertex2i(120,  400);
            glVertex2i(120, -120);
            glVertex2i(-120, -120);
            glEnd();
            glPopMatrix();

            sprintf(text, "Score : %d", score_player2);
            glColor3f(1,1,1);
            drawText("Player 2: ", 280, -40, 0.14, 0.14);
            drawText(text, 280, -80, 0.14, 0.14);


            if(y_player2 == 20)
                  randomize(PLAYER_TWO);

            print_matrix(matrix_player2, PLAYER_TWO);

            setItem(item_player2);
            glPushMatrix();
            glTranslatef(x_player2, y_player2--, 0);
            glBegin(GL_QUADS);
            glTexCoord2i(0.0f, 0.0f);
            glVertex2i(-20, 20);
            glTexCoord2i(1.0f, 0.0f);
            glVertex2i(20, 20);
            glTexCoord2i(1.0f, 1.0f);
            glVertex2i(20, -20);
            glTexCoord2i(0.0f, 1.0f);
            glVertex2i(-20,-20);
            glEnd();
            glPopMatrix();
            stabilize(matrix_player2, PLAYER_TWO);

      }

      
      glPushMatrix();
      glColor3f(0.1, 0.7, 0.2);
      glTranslatef(120,120,0);
      glBegin(GL_QUADS);
      glVertex2i(-120, 400);
      glVertex2i(120,  400);
      glVertex2i(120, -120);
      glVertex2i(-120, -120);
      glEnd();
      glPopMatrix();


      sprintf(text, "Score : %d", score_player1);
      glColor3f(1,1,1);
      drawText("Player 1: ", 0, -40, 0.14, 0.14);
      drawText(text, 0, -80, 0.14, 0.14);

      print_matrix(matrix_player1, PLAYER_ONE);


      if(y_player1 == 20)
            randomize(PLAYER_ONE);

      setItem(item_player1);
      glPushMatrix();
      glTranslatef(x_player1, y_player1--, 0);
      glBegin(GL_QUADS);
            glTexCoord2i(0.0f, 0.0f);
            glVertex2i(-20, 20);
            glTexCoord2i(1.0f, 0.0f);
            glVertex2i(20, 20);
            glTexCoord2i(1.0f, 1.0f);
            glVertex2i(20, -20);
            glTexCoord2i(0.0f, 1.0f);
            glVertex2i(-20,-20);
      glEnd();
      glPopMatrix();
      stabilize(matrix_player1, PLAYER_ONE);
      glutSwapBuffers();
      glFlush();
}

void Timer(int value) {
      glutPostRedisplay();
      glutTimerFunc(refreshMills, Timer, 0);
}

void KeysHandler(GLubyte Key, int w, int z){
      if(Key == 27)
            exit(0);
      if(Key == 'd')
            if(x_player1 != 220 && matrix_player1[y_player1/40][(x_player1/40)+1] == EMPTY)
                  x_player1+=40;
      if(Key == 's')
            if(x_player1 != 20 && matrix_player1[y_player1/40][(x_player1/40)-1] == EMPTY)
                  x_player1 -=40;

      if(Key == 'k' )
            if(x_player2 != 300 && matrix_player2[y_player2/40][(x_player2 - 300)/40 - 1 ] == EMPTY)
                  x_player2 -=40;
      if(Key == 'l')
            if(x_player2 != 500  && matrix_player2[y_player2/40][(x_player2 - 300)/40 + 1] == EMPTY)
                  x_player2 += 40;

      if(Key == '1'){
            one_player = 1;
            glutReshapeWindow(240, 600);
            start = 1;
      }
      if(Key == '2'){
            start = 1;
            one_player = 0;
      }
      if(Key == 'b' && game_over){
            start = 0;
            score_player1 = score_player2 = 0;
            game_over = 0;
      }

}

void reshape(GLsizei w, GLsizei h){
      glViewport(0,0,520, 600);
}

int main(int argc, char** argv) {
      int i,j;
      srand(time(NULL));
      for(i = 0; i < N; i++)
            for(j = 0; j < M; j++)
                  matrix_player1[i][j] = matrix_player2[i][j] = EMPTY;
      load_items();
      randomize(PLAYER_ONE);
      randomize(PLAYER_TWO);

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);
	glutInitWindowSize(520,600);
	glutCreateWindow("VraXaPsa");
	glutTimerFunc(0, Timer, 0);
	glutKeyboardFunc(KeysHandler);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	init();
	glutMainLoop();
}
