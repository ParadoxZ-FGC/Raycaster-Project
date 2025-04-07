// SAVExTHExPRINCESS Header
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>

#include <Windows.h>
#include <GL/glut.h>

#include "texture/allTextures.ppm"	// Include collision textures
#include "texture/sky.ppm"			// Include sky texture
#include "texture/title.ppm"		// Include title screen texture
#include "texture/win.ppm"			// Include win screen texture

#define SCREEN_W 960
#define SCREEN_H 640
#define PI 3.14159274

#define mapX 8	// Initalize map width
#define mapY 8	// Initalize map height
#define mapS 64	// Initalize map size

typedef struct
{
	int type;		// Static, key, enemy
	int state;		// On / Off
	int map;		// Texture to show
	float x,y,z;	// Position
} sprite; sprite sp[4];



