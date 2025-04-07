//	Author : Noah Strekow
//	References : 3DSage @ YouTube, w3schools, Microsoft Learn, ChatGPT
/*	
 *	Description : Raycaster game utilizing non-euclidian geometry,
 *	in this case hyperbolic geometry, 
 *	to create illusory puzzles the player must solve to save the princess.
 */
// Controls : Forward = W, Back = S, Turn Left = A, Turn Right = D	

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <Windows.h>
#include <GL/glut.h>

#include "texture/allTextures.ppm"	// Include collision textures
#include "texture/sky.ppm"			// Include sky texture
#include "texture/title.ppm"		// Include title screen texture
#include "texture/win.ppm"			// Include win screen texture

float degToRad(float a) {return a*M_PI/180.0;}
float fixAngle(float a) {if (a > 359) {a -= 360;} if (a < 0) {a += 360;} return a;}
float distance(ax, ay, bx, by, ang) {return (cos(degToRad(ang))*(bx-ax) - sin(degToRad(ang))*(by-ay));}
float px, py, pdx, pdy, pa;		// Player position
float frame1, frame2, fps;		// Frames per second
int gameState = 0, timer = 0;	// Game state -> Title screen, game, lose
float fade = 0;					// Fade screens up from black
#define SCREEN_W 960
#define SCREEN_H 640
#define PI 3.14159274


// <---------------------- MAP ---------------------->
#define mapX 8	// Initalize map width
#define mapY 8	// Initalize map height
#define mapS 64	// Initalize map size
int mapW[] = 	// Initilize map array matrix for walls
{
	1,1,1,1,1,1,1,1,
	1,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,1,
	1,0,0,0,0,0,3,1,
	1,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1
};
int mapF[] = 	// Initilize map array matrix for floor
{
	0,0,0,0,0,0,0,0,
	0,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,0,
	0,0,0,0,0,0,0,0
};
int mapC[] =	// Initilize map array matrix for ceiling
{
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,1,0,
	0,0,0,0,0,0,1,0,
	0,0,0,0,0,0,1,0,
	0,0,0,0,0,0,1,0,
	0,0,0,0,0,0,1,0,
	0,0,0,0,0,0,1,0,
	0,0,0,0,0,0,0,0
};

// <---------------------- SPRITES ---------------------->
typedef struct
{
	int type;		// Static, key, enemy
	int state;		// On / Off
	int map;		// Texture to show
	float x,y,z;	// Position
} sprite; sprite sp[4];
int depth[120];      // Hold wall line depth to compare for sprite depth

void drawSprite()
{
	int x, y, s;
	if ((px < sp[0].x+30) && (px > sp[0].x-30) && (px < sp[0].y+30) && (px > sp[0].y-30)) {sp[0].state = 0;}	// Pop sprite
	
	for (s = 0; s < 4; s++)
	{
		float sx = sp[s].x-px;
		float sy = sp[s].y-py;
		float sz = sp[s].z;
		
		float CS = cos(degToRad(pa)), SN = sin(degToRad(pa)); // Rotate around origin (player)
		float a = sy*CS + sx*SN;
		float b = sx*CS + sy*SN;
		sx = a; sy = b;
		
		// Convert to screen x & y
		sx = (sx*108.0/sy) + (120/2);
		sy = (sz*108.0/sy) + (80/2);
		
		// Scale sprite based on distance away
		int scale = 32*80/b;
		if (scale < 0) {scale = 0;} if (scale > 120) {scale = 120;}
		
		float tx = 0, ty = 31, txs = 31.5/(float)scale, tys = 32.0/(float)scale;
		for (x = sx-scale/2; x < sx+scale/2; x++)
		{
			ty = 31;
			for (y = 0; y < scale; y++)
			{
				if ((sp[0].state == 1) && (x > 0) && (x < 120) && (b < depth[x]))
				{
					int pixel = ((int)(ty)*32 + (int)(tx))*3 + (sp[s].map*32*32*3);
					int red		= allTextures[pixel+0];
					int green	= allTextures[pixel+1];
					int blue	= allTextures[pixel+2];
					if (red != 255, green != 0, blue != 255)
					{
						glPointSize(8); glColor3ub(red, green, blue); glBegin(GL_POINTS); glVertex2i(x*8, sy*8-y*8); glEnd();
					}
					ty -= tys; if (ty < 0){ty = 0;}
				}
			}
			tx += txs;
		}
	}
}

// <---------------------- RAYS to 3D ---------------------->
void drawRays3D()	// Cast player vision on matrix (raycast)
{
	int r, mx, my, mp, dof, side; float vx, vy, rx, ry, ra, xo, yo, disV, disH;
	ra = fixAngle(pa+30);
	
	for (r = 0; r < 120; r++)
	{
		int vmt = 0, hmt = 0;	// Vertical & Horizontal texture num
		// Check vertical lines
		dof = 0; side = 0; disV = 100000;
		float Tan = tan(degToRad(ra));
		if		(cos(degToRad(ra)) > 0.001)		{rx = (((int)px>>6)<<6)+64;		ry = (px-rx)*Tan+py; xo = 64;	yo = -xo*Tan;}	// Looking left
		else if (cos(degToRad(ra)) < -0.001)	{rx = (((int)px>>6)<<6)-0.0001;	ry = (px-rx)*Tan+py; xo = -64;	yo = -xo*Tan;}	// Looking right
		else	{rx = px; ry = py; dof = 8;}	// Looking up or down
		while (dof < 8)
		{
			mx = (int)(rx)>>6; my = (int)(ry)>>6; mp = my*mapX+mx;
			if ((mp > 0) && (mp < mapX*mapY) && (mapW[mp] > 0)) {vmt = mapW[mp]-1; dof = 8; disV = cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}	// Hit wall
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
			if ((mp > 0) && (mp < mapX*mapY) && (mapW[mp] > 0)) {hmt = mapW[mp]-1; dof = 8; disH = cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}	// Hit wall
			else {rx += xo; ry += yo; dof += 1;}	// If no wall yet, add offset
		}
		
		float shade = 1; int shadeF = 32;	// Shade true or false + shade factor
		glColor3f(0, 0.5, 0);	// Vertical or Horizontal wall hit
		if (disV < disH) {hmt = vmt; shade = 0.5; rx = vx; ry = vy; disH = disV; glColor3f(0, 0.3, 0);}
		//glLineWidth(1); glBegin(GL_LINES); glVertex2i(px, py); glVertex2i(rx, ry); glEnd();	// Draw rays
		
		depth[r] = disH;
		// Draw 3D walls
		int ca = fixAngle(pa-ra); disH = disH*cos(degToRad(ca));			// Fix line warping (fisheye)
		int lineH = (mapS*640)/(disH);
		float tyS = 32.0/(float)lineH, tyO = 0;
		if (lineH > 640) {tyO = (lineH-640)/2.0; lineH = 640;}	// Line height
		int lineO = 320-(lineH>>1);										// Line offset
		
		int y;
		float ty = tyO*tyS; //+ hmt*32;
		float tx;
		if (shade == 1) {tx = (int)(rx/2.0)%32; if (ra > 180) {tx = 31-tx;}}
		else 			{tx = (int)(ry/2.0)%32; if (ra > 90 && ra < 270) {tx = 31-tx;}}
		for (y = 0; y < lineH; y++)	// Pointilate texture drawing
		{
			int pixel = ((int)(ty)*32 + (int)(tx))*3 + (hmt*32*32*3);
			int red		= allTextures[pixel+0]*shade+shadeF;
			int green	= allTextures[pixel+1]*shade+shadeF;
			int blue	= allTextures[pixel+2]*shade+shadeF;
			glPointSize(8); glColor3ub(red, green, blue); glBegin(GL_POINTS); glVertex2i(r*8, y+lineO); glEnd();
			ty += tyS;
		}
		
		// Draw floor and ceiling
		for (y = lineO+lineH; y < 640; y++)
		{
			float dy = y-(640/2.0), deg = degToRad(ra), raFix = cos(degToRad(fixAngle(pa-ra)));
			tx = px/2 + cos(deg)*158*2*32/dy/raFix;
			ty = py/2 - sin(deg)*158*2*32/dy/raFix;
			
			// Floor
			int mp = mapF[(int)(ty/32.0)*mapX + (int)(tx/32.0)]*32*32;
			int pixel = (((int)(ty)&31)*32 + ((int)(tx)&31))*3 + (mp*3);
			int red		= allTextures[pixel+0]*0.6;
			int green	= allTextures[pixel+1]*0.6;
			int blue	= allTextures[pixel+2]*0.6;
			glPointSize(8); glColor3ub(red, green, blue); glBegin(GL_POINTS); glVertex2i(r*8, y); glEnd();
			
			// Ceiling
			mp = mapC[(int)(ty/32.0)*mapX + (int)(tx/32.0)]*32*32;
			pixel = (((int)(ty)&31)*32 + ((int)(tx)&31))*3 + (mp*3);
			red		= allTextures[pixel+0];
			green	= allTextures[pixel+1];
			blue	= allTextures[pixel+2];
			if (mp > 0) {glPointSize(8); glColor3ub(red, green, blue); glBegin(GL_POINTS); glVertex2i(r*8, 640-y); glEnd();}
		}
		
		ra = fixAngle(ra-0.5);
	}
}
void drawSky()
{
	int x, y;
	for (y = 0; y < 40; y++)
	{
		for (x = 0; x < 120; x++)
		{
			int xo = (int)pa-x; if (xo < 0) {xo += 120;} xo = xo%120;
			int pixel = (y*120+xo)*3;
			int red		= sky[pixel+0];
			int green	= sky[pixel+1];
			int blue	= sky[pixel+2];
			glPointSize(8); glColor3ub(red, green, blue); glBegin(GL_POINTS); glVertex2i(x*8, y*8); glEnd();
		}
	}
}

// <---------------------- PLAYER & INPUT ---------------------->
void move(int dir)
{
	int xo = 0; if (pdx < 0) {xo = -64;} else {xo = 64;}
	int yo = 0; if (pdy < 0) {yo = -64;} else {yo = 64;}
	int ipx = px/64.0, ipx_add_xo = (px+xo)/64.0, ipx_sub_xo = (px-xo)/64.0;
	int ipy = py/64.0, ipy_add_yo = (py+yo)/64.0, ipy_sub_yo = (py-yo)/64.0;
	int i;
	
	int a, b;
	float moveSpeed = 1, sinTime = 0, t;
	
	switch (dir)
	{
		case 1:	// Move forwards smoothly
			if (mapW[ipy*mapX + ipx_add_xo] == 0) { // Check horizontal space 
				for (i = 0; i < 64; i++) {px += pdx;}
				/*int target = px + 64;
				while (px < target)
				{
					sinTime += fps * moveSpeed;
					if (sinTime < 0) {sinTime = 0;} if (sinTime > PI) {sinTime = PI;}
					t = 0.5 * sin(sinTime - PI/2) + 0.5;
					px += (target - px)*t;
				}*/
			}
			if (mapW[ipy_add_yo*mapX + ipx] == 0) {for (i = 0; i < 64; i++) {py += pdy;}} // Check vertical space
			break;
		case 0:	// Move backwards smoothly
			if (mapW[ipy*mapX + ipx_sub_xo] == 0) {for (i = 0; i < 64; i++) {px -= pdx;}}
			if (mapW[ipy_sub_yo*mapX + ipx] == 0) {for (i = 0; i < 64; i++) {py -= pdy;}}
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
	if (key == 'e')				// Open door - NOT WORKING make into move object
	{
		
		int xo = 0; if (pdx < 0) {xo = -64;} else {xo = 64;}
		int yo = 0; if (pdy < 0) {yo = -64;} else {yo = 64;}
		int ipx = px/64.0, ipx_add_xo = (px+xo)/64.0;
		int ipy = py/64.0, ipy_add_yo = (py+yo)/64.0;
		if (mapW[ipy_add_yo*mapX + ipx_add_xo] == 3) {mapW[ipy_add_yo*mapX + ipx_add_xo] = 0;}
	}
	glutPostRedisplay();
}
// <-------------------------------------------->


void screen(int v)
{
	int x, y;
	int *T;
	if (v == 1) {T = title;}
	if (v == 2) {T = win;}
	for (y = 0; y < 80; y++)
	{
		for (x = 0; x < 120; x++)
		{
			int pixel = (y*120+x)*3;
			int red		= T[pixel+0]*fade;
			int green	= T[pixel+1]*fade;
			int blue	= T[pixel+2]*fade;
			glPointSize(8); glColor3ub(red, green, blue); glBegin(GL_POINTS); glVertex2i(x*8, y*8); glEnd();
		}
	}
	if (fade < 1) {fade += 0.001*fps;}
	if (fade > 1) {fade = 1;}
}

void init()	// Initialize application
{
	glClearColor(0.2, 0.2, 0.2, 0);
	pa = 0;	// Set initial angle 
	px = 1.5*64;	pdx = cos(degToRad(pa));	// Set initial player x
	py = 4.5*64;	pdy = -sin(degToRad(pa));	// Set initial player y
	mapW[49];	// Close doors
	
	sp[0].type = 1; sp[0].state = 1; sp[0].map = 3; sp[0].x = 6.5*64; sp[0].y = 3.5*64; sp[0].z = 20;	// THE Princess
	//sp[1].type = 1; sp[1].state = 1; sp[1].map = 3; sp[1].x = 4.5*64; sp[1].y = 4.5*64; sp[1].z = 0;	// Light
	//sp[2].type = 1; sp[3].state = 1; sp[2].map = 3; sp[2].x = 1.5*64; sp[3].y = 1.5*64; sp[3].z = 20;	//
	
	char *track;	// Initialize track name
	srand(time(NULL));	// Set rand() seed
	switch ((rand()%10)+1)	// Generate random number 1-10, select track based on number
	{
		case 1: track = "audio/1.wav"; break;	// Navia
		case 2: track = "audio/2.wav"; break;	// Collei
		case 3: track = "audio/3.wav"; break;	// Dehya
		case 4: track = "audio/4.wav"; break;	// Eula
		case 5: track = "audio/5.wav"; break;	// Kuki Shinobu
		case 6: track = "audio/6.wav"; break;	// Wanderer
		case 7: track = "audio/7.wav"; break;	// Nervilette
		case 8: track = "audio/8.wav"; break;	// Ororon
		case 9: track = "audio/9.wav"; break;
		case 10: track = "audio/10.wav"; break;	// Charlotte
		default: break;
	}
	PlaySound(track, NULL, SND_FILENAME | SND_ASYNC);	// Play random soundtrack
}
void display()	// Display window
{
	frame2 = glutGet(GLUT_ELAPSED_TIME); fps = (frame2-frame1); frame1 = glutGet(GLUT_ELAPSED_TIME);	// FPS
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (gameState == 0) {init(); fade = 0; timer = 0; gameState = 1;}	// Init game
	if (gameState == 1) {screen(1); timer += 1*fps; if (timer > 3000) {fade = 0; timer = 0; gameState=2;}}	// Title screen
	if (gameState == 2)	// Game
	{
		drawSky();
		drawRays3D();
		drawSprite();
		if ((((int)px>>6) == 6) && (((int)py>>6) == 3)) {fade = 0; timer = 0; gameState = 3;}
	}
	if (gameState == 3) {screen(2); timer += 1*fps; if (timer > 3000) {exit(0);}}	// Win
	glutPostRedisplay();
	glutSwapBuffers();
}
void resize(int w, int h) {glutReshapeWindow(960, 640);}	// Force window to keep correct dimensions

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);							// Glut init
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);	// Initilize display mode
	glutInitWindowSize(960, 640);					// Initilize window size
	glutInitWindowPosition(glutGet(GLUT_SCREEN_WIDTH)/2-960/2, glutGet(GLUT_SCREEN_HEIGHT)/2-640/2);	// Initilize window position
	glutCreateWindow("SAVExTHExPRINCESS");			// Creates window and names it
	gluOrtho2D(0, 960, 640, 0);
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(input);
	glutMainLoop();
}

/*void drawMap2D()	// Draw a 2D version of the matrix map
{
	int x, y, xo, yo;
	for (y = 0; y < mapY; y++)
	{
		for (x = 0; x < mapX; x++)
		{
			if (mapW[y*mapX+x] > 0) {glColor3f(1, 1, 1);} else {glColor3f(0, 0, 0);}	// Check status of current block
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
void drawPlayer2D()	// Draw player onto matrix
{
	glColor3f(0, 1, 1);	glPointSize(20);	glLineWidth(4);
	glBegin(GL_POINTS);	glVertex2i(px, py);	glEnd();
	glBegin(GL_LINES);	glVertex2i(px, py);	glVertex2i(px+pdx, py+pdy); glEnd();
}*/
