#include <stdio.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>
#include <time.h>
#define N 15
#define EMPTY 0
#define RED   1
#define GREEN 2
#define BLUE  3
#define SELECTED_RED 4
#define SELECTED_GREEN 5
#define SELECTED_BLUE  6

#define DRAW_LINE(From_x,From_y,From_z, To_x, To_y, To_z){\
      glPushMatrix();\
      glBegin(GL_LINES);\
      glVertex3d(From_x,From_y,From_z);\
      glVertex3d(To_x, To_y, To_z);\
      glEnd();\
      glPopMatrix();\
      }

#define DRAW_QUAD(x1,y1,z1, x2,y2,z2, x3,y3,z3, x4,y4,z4)\
      glPushMatrix();\
      glBegin(GL_QUADS);\
      glVertex3f(x1,y1,z1);\
      glVertex3f(x2,y2,z2);\
      glVertex3f(x3,y3,z3);\
      glVertex3f(x4,y4,z4);\
      glEnd();\
      glPopMatrix();

#define KEY_ESCAPE 27

GLuint grid;
int refreshMills = 18;
float cameraX = 20, cameraY = 20, cameraZ = 10;
float fi = 1.2, theta = 0.7, R = 30;
int eyeX = 0, eyeY = 0, eyeZ = 0;
int color = 0, cur_x, cur_y;  //
float cur_z = 14;            //The color and the starting position of the cubes;
int matrix[N][N][N];
int start = 0;
int color_flag = 0;
int game_over = 0;

typedef struct {
      int width;
      int height;
	char* title;
	float field_of_view_angle;
	float z_near;
	float z_far;
} glutWindow;

glutWindow win;

void draw_cube(int color){
      if(color == RED)
            glColor3f(1,0,0);
      else if(color == GREEN)
            glColor3f(0,1,0);
      else if(color == BLUE)
            glColor3f(0,0,1);
      else if(color == SELECTED_RED)
            glColor3f(0.3,0,0);
      else if(color == SELECTED_GREEN)
            glColor3f(0,0.3,0);
      else if(color == SELECTED_BLUE)
            glColor3f(0,0,0.3);
      else
            return;
      glNormal3d(1,0,0);
      DRAW_QUAD(0,0,0, 0,0,1, 0,1,1, 0,1,0);
      glNormal3d(0,0,1);
      DRAW_QUAD(0,0,0, 1,0,0, 1,1,0, 0,1,0);
      glNormal3d(0,1,0);
      DRAW_QUAD(0,0,0, 0,0,1, 1,0,1, 1,0,0);
      glNormal3d(0,0,-1);
      DRAW_QUAD(0,0,1, 0,1,1, 1,1,1, 1,0,1);
      glNormal3d(-1,0,0);
      DRAW_QUAD(1,0,1, 1,1,1, 1,1,0, 1,0,0);
      glNormal3d(0,-1,0);
      DRAW_QUAD(1,1,1, 0,1,1, 0,1,0, 1,1,0);
}

typedef struct group{
      int x;
      int y;
      int z;
      int color;
      group *next;
}group;

group *selected_group = NULL;
group *rearrange_group = NULL;

void add_to_list(int i, int j, int k,int color, group **list){
      group *temp;
      temp = (group *)malloc(sizeof(group));
      temp->x = i;
      temp->y = j;
      temp->z = k;
      temp->color = color;
      temp->next = *list;
      *list = temp;
}

void remove_from_list(int i, int j, int k){
      group *temp;
      group *prev = selected_group;
      if(prev->x == i && prev->y == j && prev->z == k){
            selected_group = prev->next;
            return;
      }
      for(temp = selected_group; temp != NULL; prev = temp, temp = temp->next){
            if(temp->x == i && temp->y == j && temp->z == k){
                  prev->next = temp->next;
                  return;
            }
      }
}

void free_list(group **list){
      group *temp, *temp1;
      temp = *list;
      while(temp != NULL){
            temp1 = temp->next;
            free(temp);
            temp = temp1;
      }
      *list = NULL;
}

void randomize(){
      color = 1 + rand() % 3;
      cur_x = rand() % 15;
      cur_y = rand() % 15;
      cur_z = 15;
      color_flag = 1;
}

void drawText(char*string,int x,int y, float w, float h){
      char *c;
      glPushMatrix();
      glTranslatef(x, y,1);
      glScalef(w,h,0);
      for (c=string; *c != '\0'; c++)
            glutStrokeCharacter(GLUT_STROKE_ROMAN , *c);
      glPopMatrix();
}

void stabilize(){
      int z = (int)(cur_z + 0.9) - 1;
      if(cur_z <= 0 && color_flag)
                  matrix[cur_x][cur_y][(int)cur_z] = color;

      else if(matrix[cur_x][cur_y][z] != EMPTY){
            if((color == RED && matrix[cur_x][cur_y][z] == BLUE) || (color == BLUE && matrix[cur_x][cur_y][z] == RED)){
                  matrix[cur_x][cur_y][(int)(cur_z)] = EMPTY;
                  matrix[cur_x][cur_y][z] = EMPTY;
            }
            else if(color_flag)
                  matrix[cur_x][cur_y][(int)cur_z] = color;
            randomize();
      }
      for(int i = 0; i < N; i++)
            for(int j = 0; j < N; j++)
                  if(matrix[i][j][N-1] != EMPTY)
                        game_over = 1;
}

void create_hole(int color){
      if(matrix[cur_x][cur_y][(int)(cur_z + 0.9) - 1] == color)
            matrix[cur_x][cur_y][(int)(cur_z + 0.9) - 1] = EMPTY;
}

void print_matrix(){
      int i, j, k;
      for(i = 0; i < N; i++)
            for(j = 0; j < N; j++)
                  for(k = 0; k < N; k++){
                        if(matrix[i][j][k] != EMPTY){
                              int color = matrix[i][j][k];
                              glPushMatrix();
                              glTranslatef(i,j,k);

                                    draw_cube(color);
                                    glColor3f(0.8,0.8,0.8);
                                    glLineWidth(2);
                                    if(matrix[i][j][k+1] != color && matrix[i+1][j][k] != color)
                                          DRAW_LINE(1,1,1, 1,0,1);
                                    if(matrix[i+1][j][k] != color && matrix[i][j+1][k] != color)
                                          DRAW_LINE(1,1,0, 1,1,1);
                                    if(matrix[i][j+1][k] != color && matrix[i][j][k+1] != color)
                                          DRAW_LINE(1,1,1, 0,1,1);
                                    if(matrix[i][j][k-1] !=color && matrix[i][j+1][k] != color)
                                          DRAW_LINE(1,1,0, 0,1,0);
                                    if(matrix[i-1][j][k] != color && matrix[i][j][k+1] != color)
                                          DRAW_LINE(0,0,1, 0,1,1);
                                    if(matrix[i][j-1][k] != color && matrix[i][j][k+1] != color)
                                          DRAW_LINE(0,0,1, 1,0,1);
                                    if(matrix[i][j-1][k] != color && matrix[i+1][j][k] != color)
                                          DRAW_LINE(1,0,0, 1,0,1);
                                    if(matrix[i][j][k-1] != color && matrix[i+1][j][k] != color)
                                          DRAW_LINE(1,0,0, 1,1,0);
                                    if(matrix[i-1][j][k] != color && matrix[i][j-1][k] != color)
                                          DRAW_LINE(0,0,0, 0,0,1);
                                    if(matrix[i-1][j][k] != color && matrix[i][j+1][k] != color)
                                          DRAW_LINE(0,1,0, 0,1,1);
                                    if(matrix[i][j][k-1] != color && matrix[i][j-1][k] != color)
                                          DRAW_LINE(0,0,0, 1,0,0);
                                    if(matrix[i-1][j][k] != color && matrix[i][j][k-1] != color)
                                          DRAW_LINE(0,0,0, 0,1,0);
                                    glLineWidth(1);
                              glPopMatrix();
                        }
                  }
}

void restore_matrix(group *list){
      group *temp;
      for(temp = list; temp != NULL; temp = temp->next)
            matrix[temp->x][temp->y][temp->z] = temp->color;
}

void remove_from_matrix(group *list){
      group *temp;
      for(temp = list; temp != NULL; temp = temp->next)
            matrix[temp->x][temp->y][temp->z] = EMPTY;
}

void print_list(){
      group *temp;
      for(temp = selected_group; temp != NULL; temp = temp->next){
            glPushMatrix();
            glTranslatef(temp->x, temp->y, temp->z);
            draw_cube(temp->color + 3);
            glPopMatrix();
      }
}

void update_list(int x, int y, int z, group **list){
      group *temp;
      for(temp = *list; temp != NULL; temp = temp->next){
            temp->x += x;
            temp->y += y;
            temp->z += z;
      }
      for(temp = *list; temp != NULL; temp = temp->next){
            if(temp->x > 14 || temp->x < 0)
                  remove_from_list(temp->x, temp->y, temp->z);
            if(temp->y > 14 || temp->y < 0)
                  remove_from_list(temp->x, temp->y, temp->z);
      }
}

void find_neighbors(int i, int j, int k, int color, group **list){
      add_to_list(i,j,k, color, list);

      if(matrix[i][j][k] <= 3)
            matrix[i][j][k] += 3;

      if(i != 0)
            if(matrix[i-1][j][k] == color)
                  find_neighbors(i-1, j, k, color, list);
      if(i != N - 1)
            if(matrix[i+1][j][k] == color)
                  find_neighbors(i+1, j,k, color, list);

      if(j != 0)
            if(matrix[i][j-1][k] == color)
                  find_neighbors(i, j-1, k, color, list);
      if(j != N - 1)
            if(matrix[i][j+1][k] == color)
                  find_neighbors(i, j+1,k, color,list);

      if(k != 0)
            if(matrix[i][j][k-1] == color)
                  find_neighbors(i, j, k-1, color, list);
      if(k != N - 1)
            if(matrix[i][j][k+1] == color)
                  find_neighbors(i, j,k + 1, color, list);

}

bool can_move(int i, int j, int k, group **list){
      group *temp;
      for(temp = *list; temp != NULL; temp = temp->next){
            if(temp->z == 0 && k == -1)
                  return false;
            if(temp->x + i >= 0 && temp->x + i < N && temp->y + j >= 0 && temp->y + j < N && temp->z + k >= 0 && temp->z +k < N){
                  if(matrix[temp->x + i][temp->y + j][temp->z] == temp->color){
                        find_neighbors(temp->x + i, temp->y + j, temp->z, temp->color, list);
                        remove_from_matrix(*list);
                        continue;
                  }
                  if(matrix[temp->x + i][temp->y + j][temp->z + k] != EMPTY)
                        return false;
            }
      }
      return true;
}

void rearrange(){
      for(int i = 0; i < N; i++)
            for(int j = 0; j < N; j++)
                  for(int k = 1; k < N; k++){
                        if(matrix[i][j][k] != EMPTY && matrix[i][j][k-1] == EMPTY &&  start){
                              free_list(&rearrange_group);
                              find_neighbors(i, j, k, matrix[i][j][k], &rearrange_group);
                              remove_from_matrix(rearrange_group);
                              while(can_move(0,0,-1,&rearrange_group)){
                                    update_list(0,0,-1, &rearrange_group);
                              }
                              restore_matrix(rearrange_group);
                        }
                  }
}

void draw_perigram(){
      glLineWidth(2);
      glColor3f(0.8,0.8,0.8);
      DRAW_LINE(0,0,0, 0,0,1);
      DRAW_LINE(0,0,0, 1,0,0);
      DRAW_LINE(0,0,0, 0,1,0);
      DRAW_LINE(1,1,1, 1,1,0);
      DRAW_LINE(1,1,1, 1,0,1);
      DRAW_LINE(1,1,1, 0,1,1);
      DRAW_LINE(1,1,0, 1,0,0);
      DRAW_LINE(1,1,0, 0,1,0);
      DRAW_LINE(1,0,0, 1,0,1);
      DRAW_LINE(0,1,0, 0,1,1);
      DRAW_LINE(0,0,1, 0,1,1);
      DRAW_LINE(0,0,1, 1,0,1);
      glLineWidth(1);
}

int calculate_score(){
      int i,j,k, score = 0;
      for(i = 0; i < N; i++)
            for(j = 0; j < N; j++)
                  for(k = 0; k < N; k++)
                        if(matrix[i][j][k] != EMPTY){
                              int count = 0;
                              free_list(&rearrange_group);
                              find_neighbors(i, j, k, matrix[i][j][k], &rearrange_group);
                              for(group *temp = rearrange_group; temp != NULL; temp = temp->next)
                                    count++;
			      score += count*count;
                              remove_from_matrix(rearrange_group);
                        }
      return score;
}


void game_over_fun(){
      int static score = calculate_score();
      char s[32];
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      GLint viewport [4];
	glGetIntegerv (GL_VIEWPORT, viewport);
      gluOrtho2D (0,viewport[2], 0, viewport[3]);

      glColor3f(0.7,0.3,0.7);
      glPushMatrix();
      drawText("Game Over", 60,500, 0.8, 0.8);
      sprintf(s, "Score: %d", score);
      drawText(s, 60, 300, 0.5, 0.5);
      glDepthFunc (GL_LESS);

      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);

      glutSwapBuffers();
}

void display(){
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(win.field_of_view_angle, (GLfloat) win.width / win.height, win.z_near, win.z_far);

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glLoadIdentity();


      if(game_over){
            game_over_fun();
            return;
      }

      cameraX= R*sin(fi)*cos(theta);
      cameraY= R*sin(fi)*sin(theta);
      cameraZ= R*cos(fi);
      gluLookAt( cameraX,cameraY,cameraZ, eyeX, eyeY, eyeZ, 0,0,1);

      glCallList(grid);
      print_matrix();

      if(start){
            static int loc_color;
            if(cur_z <= 0)
                  randomize();
            glPushMatrix();
            glTranslatef(cur_x, cur_y, cur_z-=0.1);
            draw_cube(color);
            draw_perigram();

            if(color == GREEN
                        && matrix[cur_x][cur_y][(int)(cur_z + 0.9) - 1] != EMPTY
                        && color_flag
                        && matrix[cur_x][cur_y][(int)(cur_z + 0.9) - 1] != GREEN){
                  loc_color = matrix[cur_x][cur_y][(int)(cur_z + 0.9) - 1];
                  color_flag = !color_flag;
            }
            if(color == GREEN)
                  create_hole(loc_color);

            stabilize();
            glPopMatrix();
      }
      else{
            glPushMatrix();
            glTranslatef(cur_x, cur_y, cur_z);
            draw_cube(color);
            if(color != 0)
                  draw_perigram();
            glPopMatrix();
            if(selected_group != NULL)
                  print_list();
      }
      rearrange();
      glutSwapBuffers();
}

void grid_init(){
      int i;
      grid = glGenLists(1);
      glNewList(grid,GL_COMPILE);
      glColor3f(0,0,0);
      for( i = 0; i <= N; i++){
            glPushMatrix();
            DRAW_LINE(i, 0, 0, i, N, 0);
            DRAW_LINE(i, 0, 0, i, 0, N);
            DRAW_LINE(0, i, 0, N, i, 0);
            DRAW_LINE(0, i, 0, 0,  i, N);
            DRAW_LINE(0, 0, i, 0, N, i);
            DRAW_LINE(0, 0, i, N, 0, i);
            glPopMatrix();
      }
      glPushMatrix();
      glLineWidth(2);
      DRAW_LINE(0,0,0,0,0,15);
      DRAW_LINE(0,0,0,15,0,0);
      DRAW_LINE(0,0,0,0,15,0);
      glLineWidth(2);
      glPopMatrix();
      glLineWidth(1);
      glEndList();
}

void initialize (){
      glMatrixMode(GL_PROJECTION);

      glViewport(0, 0, win.width, win.height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      gluPerspective(win.field_of_view_angle, (GLfloat) win.width / win.height, win.z_near, win.z_far);

      glMatrixMode(GL_MODELVIEW);
      glShadeModel( GL_SMOOTH );

      glClearDepth( 1.0f );
      glEnable( GL_DEPTH_TEST );
      glDepthFunc( GL_LEQUAL );


      GLfloat lightpos[] = {15,15,15,1.0};
      GLfloat amb_light[] = { 0.15, 0.15, 0.15, 1.0 };
      GLfloat diffuse[] = { 0.8, 0.8, 0.8, 1 };
      GLfloat specular[] = {0.5, 0.5, 0.5, 1 };
      glLightModelfv( GL_LIGHT_MODEL_AMBIENT, amb_light );
      glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
      glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
      glLightfv( GL_LIGHT0, GL_SPECULAR, specular );
      glEnable( GL_COLOR_MATERIAL);
      glShadeModel( GL_SMOOTH);
      glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
      glDepthFunc( GL_LEQUAL );
      glEnable( GL_DEPTH_TEST );
      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);
      glClearColor(0.5, 0.7, 0.6, 1.0);

      grid_init();
}

void keyboard ( unsigned char key, int X, int Y ){
      if(key == KEY_ESCAPE)
            exit(0);

      if(key == 'q')
            game_over = 1;

      if(key == 's'){
            static int flag = 0;

            if(!flag)
                  randomize();
            start = !start;
            restore_matrix(selected_group);
            free_list(&selected_group);
            flag = 1;
        }

      if(key == '-')
            win.field_of_view_angle++;

      if(key == '+')
            win.field_of_view_angle--;

      if(key ==  'A')
            theta-=0.1;
      if(key  == 'D')
            theta+=0.1;
      if(key == 'W')
            fi-=0.1;

      if(key == 'S')
            fi+=0.1;
      if(key == '+')
            R-=0.1;
      if(key == '-')
            R+=0.1;

      if(key == 'x')
            eyeX++;
      if(key == 'X')
            eyeX--;
      if(key == 'z')
            eyeZ++;
      if(key == 'Z')
            eyeZ--;
      if(key == 'y')
            eyeX++;
      if(key == 'Y')
            eyeZ--;
}

void Timer(int value) {
      glutPostRedisplay();
      glutTimerFunc(refreshMills, Timer, 0);
}

void special_Keys(int key, int X, int Y){
      if(start){
            if(key == GLUT_KEY_LEFT)
                  if(cur_y > 0 && matrix[cur_x][cur_y - 1][(int)cur_z] == EMPTY)
                        cur_y--;

            if(key == GLUT_KEY_RIGHT)
                  if(cur_y < N - 1 && matrix[cur_x][cur_y + 1][(int)(cur_z)] == EMPTY)
                        cur_y++;

            if(key == GLUT_KEY_UP)
                  if(cur_x > 0 && matrix[cur_x - 1][cur_y][(int)cur_z] == EMPTY)
                        cur_x--;
            if(key == GLUT_KEY_DOWN)
                  if(cur_x < N - 1 && matrix[cur_x + 1][cur_y][(int)cur_z] == EMPTY)
                        cur_x++;
      }
      else{
            if(key == GLUT_KEY_LEFT && selected_group != NULL)
                  if(can_move(0,-1, 0 ,&selected_group))
                        update_list(0,-1, 0, &selected_group);
            if(key == GLUT_KEY_RIGHT && selected_group != NULL )
                  if(can_move(0,1,0, &selected_group))
                        update_list(0,1,0, &selected_group );
            if(key == GLUT_KEY_DOWN && selected_group != NULL)
                  if(can_move(1,0,0, &selected_group))
                        update_list(1, 0, 0, &selected_group);
            if(key == GLUT_KEY_UP && selected_group != NULL)
                  if(can_move(-1,0,0, &selected_group))
                        update_list(-1,0,0, &selected_group);
      }
}

void mouse(int button,int state, int x, int y){
      GLint viewport[4];
      GLdouble modelview[16];
      GLdouble projection[16];
      GLfloat winX, winY, winZ;
      double posX, posY, posZ;

      glGetIntegerv(GL_VIEWPORT,viewport);
      glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
      glGetDoublev(GL_PROJECTION_MATRIX, projection);

      winX = (float)x;
      winY = (float)viewport[3] - (float)y;

      glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
      gluUnProject(winX, winY, winZ, modelview, projection, viewport,&posX, &posY, &posZ);

      int i,j,k;
      i = posX;
      j = posY;
      k = posZ;

      if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && i >= 0 && i < 15 && j >= 0 && j < 15 && k >= 0 && k < 15 && matrix[i][j][k] != EMPTY && !start){
            restore_matrix(selected_group);
            free_list(&selected_group);
            find_neighbors(i,j,k,matrix[i][j][k], &selected_group);
            remove_from_matrix(selected_group);
      }

}

void reshape(GLsizei w, GLsizei h){
      glViewport(0,0,800, 600);
}

int main(int argc, char **argv){
      int i,j,k;
      for(i = 0; i < N; i++)
            for(j = 0; j < N; j++)
                  for(k = 0; k < N; k++)
                        matrix[i][j][k] = EMPTY;

        srand(time(NULL));
	win.width = 800;
	win.height = 600;
	win.title = "3d Design";
	win.field_of_view_angle = 70;
	win.z_near = 1.0f;
	win.z_far = 500.0f;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize(win.width,win.height);
	glutCreateWindow(win.title);
	glutDisplayFunc(display);
        glutTimerFunc(0, Timer, 0);
        glutKeyboardFunc( keyboard );
        glutSpecialFunc(special_Keys);
        glutMouseFunc(mouse);
        glutReshapeFunc(reshape);
	initialize();
	glutMainLoop();
	return 0;
}
