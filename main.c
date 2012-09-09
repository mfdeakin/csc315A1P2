
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <GL/gl.h>
#include <GL/glut.h>

#define VIEWHEIGHT 500.0
#define VIEWWIDTH 500.0
#define OFFHEIGHT 100
#define OFFWIDTH 100
#define CENTERX 350
#define CENTERY 350

struct pt {
	GLint x;
	GLint y;
} linestart, lineend, wnddim;

enum Region {
	CENTER = 0,
	LEFT = 1,
	RIGHT = 2,
	TOP = 8,
	BOTTOM = 4
};

bool ptCompare(struct pt lhs, struct pt rhs)
{
	if(lhs.x != rhs.x || lhs.y != rhs.y)
		return false;
	return true;
}

enum Region pointRegion(struct pt point);
void drawView(void);
void drawCircle(float radius);
void drawCircle2(float radius);
void drawLine(void);
bool clipLine(struct pt *p1, struct pt *p2);
void display(void);
void resize(GLsizei width, GLsizei height);
void mpress(int btn, int state, int x, int y);

int interpolateX(struct pt p1, struct pt p2, int p1x);
int interpolateY(struct pt p1, struct pt p2, int p1y);

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	drawView();
	drawLine();
	glFlush();
 	glutSwapBuffers();
}

void drawLine(void)
{
	struct pt start, end;
	start = linestart;
	end = lineend;
	if(!ptCompare(start, end) &&
		 clipLine(&start, &end)) {
		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_POINTS);

		GLint deltax = end.x - start.x,
			deltay = end.y - start.y,
			x = 0,
			y = 0;
	
		GLfloat m = (float)deltay / deltax;

		if(m == INFINITY) {
			while(y < deltay) {
				glVertex2i(start.x, start.y + y);
				y++;
			}
		}
		else if(m == -INFINITY) {
			while(y > deltay) {
				glVertex2i(start.x, start.y + y);
				y--;
			}
		}
		else {
			float residual = 0;
			while(x > deltax) {
				x--;
				int cnt = 0;
				do {
					glVertex2i(start.x + x, start.y + y);
					cnt++;
					y--;
				} while(cnt < m + residual);
				do {
					glVertex2i(start.x + x, start.y + y);
					cnt--;
					y++;
				} while(cnt > m + residual);
				residual = m + residual - cnt;
			}
			while(x < deltax) {
				x++;
				int cnt = 0;
				do {
					glVertex2i(start.x + x, start.y + y);
					cnt++;
					y++;
				} while(cnt < m + residual);
				do {
					glVertex2i(start.x + x, start.y + y);
					cnt--;
					y--;
				} while(cnt > m + residual);
				residual = m + residual - cnt;
			}
		}
		glEnd();
	}
}

enum Region pointRegion(struct pt point)
{
	enum Region reg = CENTER;
	if(point.x < OFFWIDTH)
		reg = LEFT;
	else if(point.x > OFFWIDTH + VIEWWIDTH)
		reg = RIGHT;
	if(point.y < OFFHEIGHT)
		reg |= BOTTOM;
	else if(point.y > OFFHEIGHT + VIEWHEIGHT)
		reg |= TOP;
	return reg;
}

bool clipLine(struct pt *p1, struct pt *p2)
{
	/* Because we are in 2D space, the line is
	 * gauranteed to be clipped after two passes.
	 * With the loop like this, the compiler can
	 * unroll it more easily*/
	int i;
	printf("point 1: %d,   %d\n"
				 "point 2: %d,   %d\n",
				 p1->x, p1->y,
				 p2->x, p2->y);
	for(i = 0; i < 2; i++) {
		enum Region reg1 = pointRegion(*p1),
			reg2 = pointRegion(*p2);
		printf("reg1: %d,  reg2: %d\n", reg1, reg2);
		if(reg1 & reg2) {
			printf("exiting false\npoint 1: %d,   %d\n"
						 "point 2: %d,   %d\n\n",
						 p1->x, p1->y,
						 p2->x, p2->y);
			return false;
		}
		if(!reg1 && !reg2) {
			printf("exiting true\npoint 1: %d,   %d\n"
						 "point 2: %d,   %d\n\n",
						 p1->x, p1->y,
						 p2->x, p2->y);
			return true;
		}
		if(reg1 & LEFT) {
			p1->y = interpolateX(*p1, *p2, OFFWIDTH);
			p1->x = OFFWIDTH;
		}
		else if(reg1 & RIGHT) {
			p1->y = interpolateX(*p1, *p2, OFFWIDTH + VIEWWIDTH);
			p1->x = OFFWIDTH + VIEWWIDTH;
		}
		else if(reg1 & TOP) {
			p1->x = interpolateY(*p1, *p2, OFFHEIGHT + VIEWHEIGHT);
			p1->y = OFFHEIGHT + VIEWHEIGHT;
		}
		else if(reg1 & BOTTOM) {
			p1->x = interpolateY(*p1, *p2, OFFHEIGHT);
			p1->y = OFFHEIGHT;
		}

		if(reg2 & LEFT) {
			printf("Setting y in LEFT\n");
			p2->y = interpolateX(*p1, *p2, OFFWIDTH);
			p2->x = OFFWIDTH;
		}
		else if(reg2 & RIGHT) {
			printf("Setting y in RIGHT\n");
			p2->y = interpolateX(*p1, *p2, OFFWIDTH + VIEWWIDTH);
			p2->x = OFFWIDTH + VIEWWIDTH;
		}
		else if(reg2 & TOP) {
			printf("Setting x in TOP\n");
			p2->x = interpolateY(*p1, *p2, OFFHEIGHT + VIEWHEIGHT);
			p2->y = OFFHEIGHT + VIEWHEIGHT;
		}
		else if(reg2 & BOTTOM) {
			printf("Setting x in BOTTOM\n");
			p2->x = interpolateY(*p1, *p2, OFFHEIGHT);
			p2->y = OFFHEIGHT;
		}
		printf("point 1: %d,   %d\n"
					 "point 2: %d,   %d\n",
					 p1->x, p1->y,
					 p2->x, p2->y);
	}
	puts("");
	return true;
}

bool clipLine2(struct pt *p1, struct pt *p2)
{
	/* Better (faster) algorithm: No looping. */
	enum Region reg1 = pointRegion(*p1),
		reg2 = pointRegion(*p2);
	if(reg1 & reg2)
		return false;
	if(!reg1 && !reg2)
		return true;
	if(p1->x < OFFWIDTH) {
		p1->y = interpolateX(*p1, *p2, OFFWIDTH);
		p1->x = OFFWIDTH;
	}
	else if(p1->x > OFFWIDTH + VIEWWIDTH) {
		p1->y = interpolateX(*p1, *p2, OFFWIDTH + VIEWWIDTH);
		p1->x = OFFWIDTH + VIEWWIDTH;
	}
	if(p1->y < OFFHEIGHT) {
		p1->x = interpolateY(*p1, *p2, OFFHEIGHT + VIEWHEIGHT);
		p1->y = OFFHEIGHT + VIEWHEIGHT;
	}
	else if(p1->y > OFFHEIGHT + VIEWHEIGHT) {
		p1->x = interpolateY(*p1, *p2, OFFHEIGHT);
		p1->y = OFFHEIGHT;
	}

	if(p2->x < OFFWIDTH) {
		p2->y = interpolateX(*p1, *p2, OFFWIDTH);
		p2->x = OFFWIDTH;
	}
	else if(p2->x > OFFWIDTH + VIEWWIDTH) {
		p2->y = interpolateX(*p1, *p2, OFFWIDTH + VIEWWIDTH);
		p2->x = OFFWIDTH + VIEWWIDTH;
	}
	if(p2->y < OFFHEIGHT) {
		p2->x = interpolateY(*p1, *p2, OFFHEIGHT + VIEWHEIGHT);
		p2->y = OFFHEIGHT + VIEWHEIGHT;
	}
	else if(p2->y > OFFHEIGHT + VIEWHEIGHT) {
		p2->x = interpolateY(*p1, *p2, OFFHEIGHT);
		p2->y = OFFHEIGHT;
	}
	return true;
}

int interpolateX(struct pt p1, struct pt p2, int newX)
{
	float dx = p1.x - p2.x,
		dy = p1.y - p2.y;
	return (int)dy / dx * (newX - p2.x) + p2.y;
}

int interpolateY(struct pt p1, struct pt p2, int newY)
{
	float dx = p1.x - p2.x,
		dy = p1.y - p2.y;
	return (int)dx / dy * (newY - p2.y) + p2.x;
}

void mpress(int btn, int state, int x, int y)
{
	y = wnddim.y - y;
	if(btn == GLUT_LEFT_BUTTON) {
		linestart.x = x;
		linestart.y = y;
		glutPostRedisplay();
	}
	else if(btn == GLUT_RIGHT_BUTTON) {
		lineend.x = x;
		lineend.y = y;
		glutPostRedisplay();
	}
}

void drawView(void)
{
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_POINTS);
	struct pt pos;
	for(pos.x = 100; pos.x < 600; pos.x++)
		for(pos.y = 100; pos.y < 600; pos.y++) {
			glVertex2i(pos.x, pos.y);
		}
	glEnd();
}

void resize(GLsizei width, GLsizei height)
{
	wnddim.x = width;
	wnddim.y = height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, 0.0,
					height, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void keypress(unsigned char key, int x, int y)
{
	switch(key) {
	case 'q':
	case 'Q':
		exit(0);
	}
}

int main(int argc, char **argv)
{
	memset(&linestart, 0, sizeof(linestart));
	memset(&lineend, 0, sizeof(lineend));
	memset(&wnddim, 0, sizeof(wnddim));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 10);
	glutInitWindowSize(700, 700);
	glutCreateWindow("Program 1");
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutMouseFunc(mpress);
	glutKeyboardFunc(keypress);
	glutMainLoop();
  return 0;
}
