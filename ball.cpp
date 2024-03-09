#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define BLOCKS 10
#define BLOCKS_SIZE 40
#define CHAR_BUFFER 10

void initBlocks(float, float);
void output(float, float, float, const char *);
void handleKeys();
void specialKeyPressed(int, int, int);
void specialKeyUp(int, int, int);

float blockColor[BLOCKS];

float blocks[BLOCKS_SIZE];
float blocks2[BLOCKS_SIZE];

char str[CHAR_BUFFER];

bool* keys;

float paddle_height = 0.01,
		paddle_width = 0.2;
float paddle_x1 = 0-paddle_width/2.0,
		paddle_y1 = -0.8;
float paddle_x2 = 0+paddle_width/2.0,
		paddle_y2 = paddle_y1 + paddle_height;
float paddle_alpha = 1.0;

float ball_x = 0, ball_y = 0, ball_z = 0;
float ball_radius = 0.015;
float ball_offset_y = 0.009, ball_offset_x = 0.001;

float x = 0;
float epsilon = 0.001;

int count = 0;

//flags
int bounced = 0, flag = 0, xflag = 0, paddle = 0, block = 0;

const float pi = 355.0/113.0; // well-known decent estimate
const float radians = 2.0*pi; // full circle is 2pi
// Divide into 360 points along the circumference
const float degreeInterval = radians/360.0;
const float max_ball_offset_x = 0.01, max_ball_offset_y = 0.01;

float paddleBounceRads = 1234321132421;

float  height = 0.05, width = 0.15;

void init() {
	glEnable(GL_DEPTH_TEST);
    initBlocks(-0.8, 0.9);
    keys = new bool[256];
    printf("GLUT key codes: l:%d r:%d u:%d d:%d", GLUT_KEY_LEFT, GLUT_KEY_RIGHT, GLUT_KEY_UP, GLUT_KEY_DOWN);
}

void reshaped(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 0, 1, 200);
}

void drawBox(float x, int i) {
    glColor3f(x - 0.05, x, x + 0.05);
    glRectf(blocks[i], blocks[i+1], blocks[i+2], blocks[i+3]);
}

int checkBall(int blockIndex) {

    float i;

    for(i = 0.0; i < radians; i += degreeInterval) {

        float circX = cos(i)*ball_radius + ball_x;
        float circY = sin(i)*ball_radius + ball_y;
		//printf("Looking at %f\n", i);
        if( circX >= blocks[blockIndex]   &&
            circX <= blocks[blockIndex+2] &&
            circY >= blocks[blockIndex+1] &&
            circY <= blocks[blockIndex+3]   )
        {
        	block = 1;
			printf("Bounced: %f", i);
            return true;
        }
		else if(circX >= paddle_x1 &&
				circX <= paddle_x2 &&
				circY >= paddle_y1 &&
				circY <= paddle_y2   ) {
			paddle = 1;
			paddleBounceRads = i;
			printf("Bounced: %f", paddleBounceRads);
			return true;
		}
    }

    return false;
}

void checkCollision(int i) {

    x = 0.65f;
    paddle = block = 0;

    if(checkBall(i)) {

        if(block && blockColor[i/4] != x) {
            blockColor[i/4] = x;
            count++;
        }
        else if(block) {
            blockColor[i/4] = .5f;
            count++;
        }
        else if(paddle) {
        	if( paddleBounceRads < (0.5*pi - epsilon) ||
        		paddleBounceRads > (1.5*pi + epsilon)   ) {
				xflag = !xflag;
        	}
        	else if(paddleBounceRads > (0.5*pi + epsilon) &&
        			paddleBounceRads < (1.5*pi - epsilon)   ) {
				xflag = !xflag;
        	}
        	else if(paddleBounceRads > (0.5*pi - epsilon) &&
        			paddleBounceRads < (0.5*pi + epsilon)   ) {
        		//don't really give a shit
        	}
        	else if(paddleBounceRads < (1.5*pi - epsilon) &&
        			paddleBounceRads > (1.5*pi + epsilon)   ) {
        		float halfPaddle = paddle_width/2.0;
        		float paddleCenter = paddle_x1 + halfPaddle;
        		float ballRelativeToCenter = ball_x - paddleCenter;
        		float absBallRelative = fabs(ballRelativeToCenter);
        		ball_offset_x = (absBallRelative/halfPaddle)*max_ball_offset_x;
        	}
        }

        drawBox(blockColor[i/4], i);

        if(!bounced) {

            bounced = 1;

            if(flag) {
                flag = 0;
            }
            else {
                flag = 1;
            }
        }
    }
    else {
        drawBox(blockColor[i/4], i);
    }
}

void updateBall()
{
    int i;

    for(i = 0; i < BLOCKS_SIZE; i += 4) {
        checkCollision(i);
    }

	if(!flag)
	{
		ball_y += ball_offset_y;
		if (ball_y>0.95) {
			flag = 1;
		}
	}
	else if(flag) {

		ball_y -= ball_offset_y;
		if (ball_y<-0.95) {
			flag = 0;
		}
	}

	if(!xflag) {
		ball_x += ball_offset_x;
		if(ball_x > 0.95) {
			xflag = 1;
		}
	}
	else if(xflag) {
		ball_x -= ball_offset_x;
		if(ball_x < -0.95) {
			xflag = 0;
		}
	}

    bounced = 0;
	handleKeys();
	output(-0.8,-0.8,-0.1,"test");
}

void drawPaddle() {
    glColor3f(0.0, 0.7, 0.0);
    glRectf(paddle_x1, paddle_y1, paddle_x2, paddle_y2);
}

void specialKeyPressed(int key, int x, int y) {
	keys[key] = true;
}

void specialKeyUp(int key, int x, int y) {
	keys[key] = false;
}

void handleKeys() {

	float delta = 0.008;
	int i;

	for(i = 0; i < 256; i++) {

		unsigned char key;

		if(keys[i]) {
			key = i;
		}
		else {
			continue;
		}

		switch (key) {

		case GLUT_KEY_LEFT:
        	if(paddle_x1 > -0.9) {
            	paddle_x1 -= delta;
            	paddle_x2 -= delta;
        	}
			break;

		case GLUT_KEY_RIGHT:
			if(paddle_x2 < 0.9) {
            	paddle_x1 += delta;
            	paddle_x2 += delta;
        	}
			break;

		case GLUT_KEY_UP:
			if(paddle_y2 < 0.2) {
            	paddle_y1 += delta;
            	paddle_y2 += delta;
        	}
			break;

		case GLUT_KEY_DOWN:
			if(paddle_y2 > -0.9) {
            	paddle_y1 -= delta;
            	paddle_y2 -= delta;
        	}
			break;
		}
	}
}

void initBlocks(float x_block, float y_block) {

   int i;

   for(i = 0; i < 40; i += 4) {
      blockColor[i/4] = 0.5f;
      blocks[i] = x_block;
      blocks[i+1] = y_block - height;
      blocks[i+2] = x_block + width;
      blocks[i+3] = y_block;
      x_block += width + 0.01;
   }

   printf("Blocks initialized!\n");
}

void output(float x, float y, float z, const char * string) {

    glPushMatrix();

	glColor3f(0.05, 0.65, 0.24);

	int len, i;

	glRasterPos3f(x, y, z);

	len = (int) strlen(string);

	for (i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
		printf("%c", str[i]);
	}

	sprintf(str, "%d", count);

    glPopMatrix() ;
}

void display() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glPushMatrix();
	glColor3f(0, 1, 1);
	glTranslatef(ball_x, ball_y, ball_z);
	glutSolidSphere(ball_radius, 23, 23);
	glPopMatrix();

	glPushMatrix();
	drawPaddle();
	glPopMatrix();

	updateBall();

    glutSwapBuffers();
}

int main(int argv, char** argc) {

    glutInit(&argv, argc);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH);

	printf("Calculated pi as: %f\n", pi);
	printf("Calculated 2pi as: %f\n", radians);
	printf("Calculated 1 degree, in radians, as: %f\n", degreeInterval);

    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("breakout clone");
	glutSpecialFunc(specialKeyPressed);
	glutSpecialUpFunc(specialKeyUp);

    init ();

    glutDisplayFunc(display);
    glutIdleFunc(display);
	glutReshapeFunc(reshaped);
	glutMainLoop();

    return 0;
}
