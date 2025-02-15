//	Author : Noah Strekow
//	References : 3DSage @ YouTube
/*	
 *	Description : Raycaster game utilizing non-euclidian geometry,
 *	in this case hyperbolic geometry, 
 *	to create illusory puzzles the player must solve to save the princess.
 */
// Controls : Forward = W, Back = S, Turn Left = A, Turn Right = D	
 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GL/glut.h>
#include <Windows.h>
#include <MMsystem.h>
#include <math.h>

#define PI 3.1415926535
#define UP PI/2
#define DOWN 3*PI/2
#define LEFT PI
#define RIGHT 0
#define DR 0.0174533	// 1 degree -> radians

/*
 * TODO : 
 *	Create menu screen
 *	Multiple levels
 *	Alter raycast to project player vision hyperbolically
 *	load textures
 */


// <---------------------- MAP ---------------------->
#define mapX 8	// Initalize map width
#define mapY 8	// Initalize map height
#define mapS 64	// Initalize map size
int map[] = 	// Initilize map array matrix
{
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 1, 0, 0, 0, 0, 1,
	1, 0, 1, 0, 0, 0, 0, 1,
	1, 0, 1, 1, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 1, 0, 1,
	1, 0, 0, 1, 1, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1
};
void drawMap2D()	// Draw a 2D version of the matrix map
{
	int x, y, xo, yo;
	for (y = 0; y < mapY; y++)
	{
		for (x = 0; x < mapX; x++)
		{
			if (map[y*mapX+x] == 1) {glColor3f(1, 1, 1);} else {glColor3f(0, 0, 0);}	// Check status of current block
			xo = x*mapS; yo = y*mapS;
			glBegin(GL_QUADS);
			glVertex2i(xo+1, yo+1);
			glVertex2i(xo+1, yo+mapS-1);
			glVertex2i(xo+mapS-1, yo+mapS-1);
			glVertex2i(xo+mapS-1, yo+1);
			glEnd();
		}
	}
}

// <---------------------- PLAYER & INPUT ---------------------->
float degToRad(float a) {return a*M_PI/180.0;}
float fixAngle(float a) {if (a > 359) {a -= 360;} if (a < 0) {a += 360;} return a;}

float px, py, pdx, pdy, pa;	// Player position
void drawPlayer2D()	// Draw player onto matrix
{
	glColor3f(0, 1, 1);	glPointSize(20);	glLineWidth(4);
	glBegin(GL_POINTS);	glVertex2i(px, py);	glEnd();
	glBegin(GL_LINES);	glVertex2i(px, py);	glVertex2i(px+pdx, py+pdy); glEnd();
}

void move(int dir)
{
	int xo = 0; if (pdx < 0) {xo = -64;} else {xo = 64;}
	int yo = 0; if (pdy < 0) {yo = -64;} else {yo = 64;}
	int ipx = px/64.0, ipx_add_xo = (px+xo)/64.0, ipx_sub_xo = (px-xo)/64.0;
	int ipy = py/64.0, ipy_add_yo = (py+yo)/64.0, ipy_sub_yo = (py-yo)/64.0;
	int i;
	switch (dir)
	{
		case 1:	// Move forwards smoothly
			if (map[ipy*mapX + ipx_add_xo] == 0) {for (i = 0; i < 64; i++) {px += pdx;}}
			if (map[ipy_add_yo*mapX + ipx] == 0) {for (i = 0; i < 64; i++) {py += pdy;}}
			break;
		case 0:	// Move backwards smoothly
			if (map[ipy*mapX + ipx_sub_xo] == 0) {for (i = 0; i < 64; i++) {px -= pdx;}}
			if (map[ipy_sub_yo*mapX + ipx] == 0) {for (i = 0; i < 64; i++) {py -= pdy;}}
			break;
		default: break;
	}
}
void turn(int dir)
{
	int i;
	switch (dir)
	{
		case 1:	// Turn right smoothly
			for (i = 0; i < 90; i++) {pa -= 1; pa = fixAngle(pa); pdx = cos(degToRad(pa)); pdy = -sin(degToRad(pa));}
			break;
		case 0:	// Turn left smoothly
			for (i = 0; i < 90; i++) {pa += 1; pa = fixAngle(pa); pdx = cos(degToRad(pa)); pdy = -sin(degToRad(pa));}
			break;
		default: break;
	}
}
void input(unsigned char key, int x, int y)	// Move player on the matrix grid
{
	if (key == 'w') {move(1);}	// Forward
	if (key == 'a') {turn(0);}	// Left
	if (key == 's') {move(0);}	// Back
	if (key == 'd') {turn(1);}	// Right
	glutPostRedisplay();
}

// <---------------------- RAYS & WALLS ---------------------->
float distance(ax, ay, bx, by, ang) {return (cos(degToRad(ang))*(bx-ax) - sin(degToRad(ang))*(by-ay));}

void drawRays3D()	// Cast player vision on matrix (raycast)
{
	glColor3f(0,1,1); glBegin(GL_QUADS); glVertex2i(526,  0); glVertex2i(1006,  0); glVertex2i(1006,160); glVertex2i(526,160); glEnd();	
 	glColor3f(0,0,1); glBegin(GL_QUADS); glVertex2i(526,160); glVertex2i(1006,160); glVertex2i(1006,320); glVertex2i(526,320); glEnd();	
	
	int r, mx, my, mp, dof, side; float vx, vy, rx, ry, ra, xo, yo, disV, disH;
	ra = fixAngle(pa+30);
	
	for (r = 0; r < 60; r++)
	{
		// Check vertical lines
		dof = 0; side = 0; disV = 100000;
		float Tan = tan(degToRad(ra));
		if		(cos(degToRad(ra)) > 0.001)		{rx = (((int)px>>6)<<6)+64;		ry = (px-rx)*Tan+py; xo = 64;	yo = -xo*Tan;}	// Looking left
		else if (cos(degToRad(ra)) < -0.001)	{rx = (((int)px>>6)<<6)-0.0001;	ry = (px-rx)*Tan+py; xo = -64;	yo = -xo*Tan;}	// Looking right
		else	{rx = px; ry = py; dof = 8;}	// Looking up or down
		while (dof < 8)
		{
			mx = (int)(rx)>>6; my = (int)(ry)>>6; mp = my*mapX+mx;
			if ((mp > 0) && (mp < mapX*mapY) && (map[mp] == 1)) {dof = 8; disV = cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}	// Hit wall
			else {rx += xo; ry += yo; dof += 1;}	// If no wall yet, add offset
		}
		vx = rx; vy = ry;
		
		// Check horizontal lines
		dof = 0; disH = 100000;
		Tan = 1.0/Tan;
		if		(sin(degToRad(ra)) > 0.001)		{ry = (((int)py>>6)<<6)-0.0001;	rx = (py-ry)*Tan+px; yo = -64;	xo = -yo*Tan;}	// Looking up
		else if (sin(degToRad(ra)) < -0.001)	{ry = (((int)py>>6)<<6)+64;		rx = (py-ry)*Tan+px; yo = 64; 	xo = -yo*Tan;}	// Looking down
		else 	{rx = px; ry = py; dof = 8;}	// Looking left or right
		while (dof < 8)
		{
			mx = (int)(rx)>>6; my = (int)(ry)>>6; mp = my*mapX+mx;
			if ((mp > 0) && (mp < mapX*mapY) && (map[mp] == 1)) {dof = 8; disH = cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}	// Hit wall
			else {rx += xo; ry += yo; dof += 1;}	// If no wall yet, add offset
		}
		
		glColor3f(0, 0.5, 0);	// Vertical or Horizontal wall hit
		if (disV < disH) {rx = vx; ry = vy; disH = disV; glColor3f(0, 0.3, 0);}
		glLineWidth(1); glBegin(GL_LINES); glVertex2i(px, py); glVertex2i(rx, ry); glEnd();	// Draw rays
		
		// Draw 3D walls
		float ca = fixAngle(pa-ra); disH = disH*cos(degToRad(ca));			// Fix line warping (fisheye)
		float lineH = (mapS*320)/(disH); if (lineH > 320) {lineH = 320;}	// Line height
		float lineO = 160-(lineH/2);									// Line offset
		glLineWidth(8); glBegin(GL_LINES); glVertex2i(r*8+530, lineO); glVertex2i(r*8+530, lineH+lineO); glEnd();	// Draw walls
		
		ra = fixAngle(ra-1);
	}
}
// <-------------------------------------------->


void init()	// Initialize application
{
	glClearColor(0.2, 0.2, 0.2, 0);
	gluOrtho2D(0, 1024, 512, 0);
	pa = 90;	// Set initial angle 
	px = (64*5)-32;	pdx = cos(degToRad(pa));	// Set initial player x
	py = (64*5)-32;	pdy = sin(degToRad(pa));	// Set initial player y
	
	char *track;	// Initialize track name
	srand(time(NULL));	// Set rand() seed
	switch ((rand()%3)+1)	// Generate random number 1-10, select track based on number
	{
		case 1: track = "audio/1.wav"; break;
		case 2: track = "audio/2.wav"; break;
		case 3: track = "audio/3.wav"; break;
		default: break;
	}
	PlaySound(track, NULL, SND_FILENAME | SND_ASYNC);	// Play random soundtrack
	//system("pause");
}
void display()	// Display window
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawMap2D();
	drawRays3D();
	drawPlayer2D();
	glutSwapBuffers();
}
void resize(int w, int h)	// Force window to keep correct dimensions
{
	glutReshapeWindow(1024, 512);
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);							// Glut init
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);	// Initilize display mode
	glutInitWindowSize(1024, 512);					// Initilize window size
	glutInitWindowPosition(400, 400);				// Initilize window position
	glutCreateWindow("SAVExTHExPRINCESS");			// Creates window and names it
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(input);
	glutMainLoop();
}

