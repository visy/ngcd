#include <stdio.h>
#include <stdlib.h>
#include <input.h>
#include <DATlib.h>
#include <math.h>
#include <string.h>
#include "externs.h"

#define FRONT_START_X 0
#define FRONT_START_Y 0
#define FRONT_MIN_X 8
#define FRONT_MAX_X 307
#define FRONT_MIN_Y 16
#define FRONT_MAX_Y 24

#define BACK_MIN_X 8
#define BACK_MAX_X 149
#define BACK_MIN_Y 5
#define BACK_MAX_Y 8

#define SHIFT_AMOUNT 16 // 2^16 = 65536
#define SHIFT_MASK ((1 << SHIFT_AMOUNT) - 1) // 65535 (all LSB set, all MSB clear)

struct palette_t
{
	int original_color, new_color;
	int index, red, green, blue, index_temp, red_temp, green_temp, blue_temp;
	int RGB63_red, RGB63_green, RGB63_blue, RGB63_red_index, RGB63_green_index, RGB63_blue_index;
	int RGB63_red_temp, RGB63_green_temp, RGB63_blue_temp, RGB63_red_index_temp, RGB63_green_index_temp, RGB63_blue_index_temp;
};

struct palette_t color[256];
scroller kissScroll;

// FADE OUT //////////////////////////////////////////////////////////////////

void fade_out()
{
	int palette_no = 0; // first palette to start with
	int i;
	int steps=0;

	waitVBlank();

	for(i=0; i<128; i++)
	{
		// save original palettes and calculate hex values for index bits and each color //

		color[i].original_color  = volMEMWORD(4194304+(i*2)+(palette_no*32));  // 4194304 decimal = 0x400000 hex adress of color 0 of palette 0

		color[i].blue=color[i].original_color/16;	// quotient
		color[i].green=color[i].blue/16;		// quotient
		color[i].red=color[i].green/16;			// quotient
		color[i].index=color[i].red/16;			// quotient

		color[i].index=color[i].red%16;			// remainder
		color[i].red=color[i].green%16;			// remainder
		color[i].green=color[i].blue%16;		// remainder
		color[i].blue=color[i].original_color%16;	// remainder

		// convert to RGB63 ///////////////////////////////////////////////

		if(color[i].index==0) {color[i].RGB63_red_index=1; color[i].RGB63_green_index=1; color[i].RGB63_blue_index=1;}
		if(color[i].index==1) {color[i].RGB63_red_index=1; color[i].RGB63_green_index=1; color[i].RGB63_blue_index=3;}
		if(color[i].index==2) {color[i].RGB63_red_index=1; color[i].RGB63_green_index=3; color[i].RGB63_blue_index=1;}
		if(color[i].index==3) {color[i].RGB63_red_index=1; color[i].RGB63_green_index=3; color[i].RGB63_blue_index=3;}
		if(color[i].index==4) {color[i].RGB63_red_index=3; color[i].RGB63_green_index=1; color[i].RGB63_blue_index=1;}
		if(color[i].index==5) {color[i].RGB63_red_index=3; color[i].RGB63_green_index=1; color[i].RGB63_blue_index=3;}
		if(color[i].index==6) {color[i].RGB63_red_index=3; color[i].RGB63_green_index=3; color[i].RGB63_blue_index=1;}
		if(color[i].index==7) {color[i].RGB63_red_index=3; color[i].RGB63_green_index=3; color[i].RGB63_blue_index=3;}
		if(color[i].index==8) {color[i].RGB63_red_index=0; color[i].RGB63_green_index=0; color[i].RGB63_blue_index=0;}
		if(color[i].index==9) {color[i].RGB63_red_index=0; color[i].RGB63_green_index=0; color[i].RGB63_blue_index=2;}
		if(color[i].index==10){color[i].RGB63_red_index=0; color[i].RGB63_green_index=2; color[i].RGB63_blue_index=0;}
		if(color[i].index==11){color[i].RGB63_red_index=0; color[i].RGB63_green_index=2; color[i].RGB63_blue_index=2;}
		if(color[i].index==12){color[i].RGB63_red_index=2; color[i].RGB63_green_index=0; color[i].RGB63_blue_index=0;}
		if(color[i].index==13){color[i].RGB63_red_index=2; color[i].RGB63_green_index=0; color[i].RGB63_blue_index=2;}
		if(color[i].index==14){color[i].RGB63_red_index=2; color[i].RGB63_green_index=2; color[i].RGB63_blue_index=0;}
		if(color[i].index==15){color[i].RGB63_red_index=2; color[i].RGB63_green_index=2; color[i].RGB63_blue_index=2;}

		color[i].RGB63_red   = color[i].red   * 4 + color[i].RGB63_red_index;
		color[i].RGB63_green = color[i].green * 4 + color[i].RGB63_green_index;
		color[i].RGB63_blue  = color[i].blue  * 4 + color[i].RGB63_blue_index;

		// copy ASM Hex Color Code values to temporary variables ///////////

		color[i].index_temp = color[i].index;
		color[i].red_temp   = color[i].red;
		color[i].green_temp = color[i].green;
		color[i].blue_temp  = color[i].blue;
	}

	while(1)
	{
		waitVBlank();


		steps-=1;

		for(i=0; i<256; i++)
		{
			// add steps to RGB63 temporary values /////////////////////////

			color[i].RGB63_red_temp   = color[i].RGB63_red - steps;
			color[i].RGB63_green_temp = color[i].RGB63_green - steps;
			color[i].RGB63_blue_temp  = color[i].RGB63_blue - steps;

			// keep values inside the range of 0 to 63 /////////////////////

			if(color[i].RGB63_red_temp>63) color[i].RGB63_red_temp=63;
			if(color[i].RGB63_red_temp<0) color[i].RGB63_red_temp=0;
			if(color[i].RGB63_green_temp>63) color[i].RGB63_green_temp=63;
			if(color[i].RGB63_green_temp<0) color[i].RGB63_green_temp=0;
			if(color[i].RGB63_blue_temp>63) color[i].RGB63_blue_temp=63;
			if(color[i].RGB63_blue_temp<0) color[i].RGB63_blue_temp=0;

			// convert RGB63 back to ASM Hex Color Code ////////////////////

			color[i].RGB63_red_index_temp = color[i].RGB63_red_temp%4;
			color[i].red_temp = color[i].RGB63_red_temp/4;

			color[i].RGB63_green_index_temp = color[i].RGB63_green_temp%4;
			color[i].green_temp = color[i].RGB63_green_temp/4;

			color[i].RGB63_blue_index_temp = color[i].RGB63_blue_temp%4;
			color[i].blue_temp = color[i].RGB63_blue_temp/4;

			if(color[i].RGB63_red_index_temp==1 && color[i].RGB63_green_index_temp==1 && color[i].RGB63_blue_index_temp==1) color[i].index_temp=0;
			if(color[i].RGB63_red_index_temp==1 && color[i].RGB63_green_index_temp==1 && color[i].RGB63_blue_index_temp==3) color[i].index_temp=1;
			if(color[i].RGB63_red_index_temp==1 && color[i].RGB63_green_index_temp==3 && color[i].RGB63_blue_index_temp==1) color[i].index_temp=2;
			if(color[i].RGB63_red_index_temp==1 && color[i].RGB63_green_index_temp==3 && color[i].RGB63_blue_index_temp==3) color[i].index_temp=3;
			if(color[i].RGB63_red_index_temp==3 && color[i].RGB63_green_index_temp==1 && color[i].RGB63_blue_index_temp==1) color[i].index_temp=4;
			if(color[i].RGB63_red_index_temp==3 && color[i].RGB63_green_index_temp==1 && color[i].RGB63_blue_index_temp==3) color[i].index_temp=5;
			if(color[i].RGB63_red_index_temp==3 && color[i].RGB63_green_index_temp==3 && color[i].RGB63_blue_index_temp==1) color[i].index_temp=6;
			if(color[i].RGB63_red_index_temp==3 && color[i].RGB63_green_index_temp==3 && color[i].RGB63_blue_index_temp==3) color[i].index_temp=7;
			if(color[i].RGB63_red_index_temp==0 && color[i].RGB63_green_index_temp==0 && color[i].RGB63_blue_index_temp==0) color[i].index_temp=8;
			if(color[i].RGB63_red_index_temp==0 && color[i].RGB63_green_index_temp==0 && color[i].RGB63_blue_index_temp==2) color[i].index_temp=9;
			if(color[i].RGB63_red_index_temp==0 && color[i].RGB63_green_index_temp==2 && color[i].RGB63_blue_index_temp==0) color[i].index_temp=10;
			if(color[i].RGB63_red_index_temp==0 && color[i].RGB63_green_index_temp==2 && color[i].RGB63_blue_index_temp==2) color[i].index_temp=11;
			if(color[i].RGB63_red_index_temp==2 && color[i].RGB63_green_index_temp==0 && color[i].RGB63_blue_index_temp==0) color[i].index_temp=12;
			if(color[i].RGB63_red_index_temp==2 && color[i].RGB63_green_index_temp==0 && color[i].RGB63_blue_index_temp==2) color[i].index_temp=13;
			if(color[i].RGB63_red_index_temp==2 && color[i].RGB63_green_index_temp==2 && color[i].RGB63_blue_index_temp==0) color[i].index_temp=14;
			if(color[i].RGB63_red_index_temp==2 && color[i].RGB63_green_index_temp==2 && color[i].RGB63_blue_index_temp==2) color[i].index_temp=15;

			// write new ASM Hex Color Code to color adress ////////////////

			color[i].new_color=(color[i].index_temp)*4096+(color[i].red_temp)*256+(color[i].green_temp)*16+(color[i].blue_temp);
			volMEMWORD(4194304+(i*2)+(palette_no*32)) = color[i].new_color;
		}
		backgroundColor(color[2].new_color);
		SCClose();
		if(steps<=-64) break; // exit loop when done
	}


}

// FADE IN //////////////////////////////////////////////////////////////////

void prepare_fade_in()
{
	int palette_no = 5; // first palette to start with
	int i;
	int steps=-64;

	waitVBlank();

	for(i=0; i<128; i++)
	{
		// save original palettes and calculate hex values for index bits and each color //

		color[i].original_color  = volMEMWORD(4194304+(i*2)+(palette_no*32));  // 4194304 decimal = 0x400000 hex adress of color 0 of palette 0

		color[i].blue=color[i].original_color/16;	// quotient
		color[i].green=color[i].blue/16;		// quotient
		color[i].red=color[i].green/16;			// quotient
		color[i].index=color[i].red/16;			// quotient

		color[i].index=color[i].red%16;			// remainder
		color[i].red=color[i].green%16;			// remainder
		color[i].green=color[i].blue%16;		// remainder
		color[i].blue=color[i].original_color%16;	// remainder

		// convert to RGB 63 ////////////////////////////////////////////////////

		if(color[i].index==0) {color[i].RGB63_red_index=1; color[i].RGB63_green_index=1; color[i].RGB63_blue_index=1;}
		if(color[i].index==1) {color[i].RGB63_red_index=1; color[i].RGB63_green_index=1; color[i].RGB63_blue_index=3;}
		if(color[i].index==2) {color[i].RGB63_red_index=1; color[i].RGB63_green_index=3; color[i].RGB63_blue_index=1;}
		if(color[i].index==3) {color[i].RGB63_red_index=1; color[i].RGB63_green_index=3; color[i].RGB63_blue_index=3;}
		if(color[i].index==4) {color[i].RGB63_red_index=3; color[i].RGB63_green_index=1; color[i].RGB63_blue_index=1;}
		if(color[i].index==5) {color[i].RGB63_red_index=3; color[i].RGB63_green_index=1; color[i].RGB63_blue_index=3;}
		if(color[i].index==6) {color[i].RGB63_red_index=3; color[i].RGB63_green_index=3; color[i].RGB63_blue_index=1;}
		if(color[i].index==7) {color[i].RGB63_red_index=3; color[i].RGB63_green_index=3; color[i].RGB63_blue_index=3;}
		if(color[i].index==8) {color[i].RGB63_red_index=0; color[i].RGB63_green_index=0; color[i].RGB63_blue_index=0;}
		if(color[i].index==9) {color[i].RGB63_red_index=0; color[i].RGB63_green_index=0; color[i].RGB63_blue_index=2;}
		if(color[i].index==10){color[i].RGB63_red_index=0; color[i].RGB63_green_index=2; color[i].RGB63_blue_index=0;}
		if(color[i].index==11){color[i].RGB63_red_index=0; color[i].RGB63_green_index=2; color[i].RGB63_blue_index=2;}
		if(color[i].index==12){color[i].RGB63_red_index=2; color[i].RGB63_green_index=0; color[i].RGB63_blue_index=0;}
		if(color[i].index==13){color[i].RGB63_red_index=2; color[i].RGB63_green_index=0; color[i].RGB63_blue_index=2;}
		if(color[i].index==14){color[i].RGB63_red_index=2; color[i].RGB63_green_index=2; color[i].RGB63_blue_index=0;}
		if(color[i].index==15){color[i].RGB63_red_index=2; color[i].RGB63_green_index=2; color[i].RGB63_blue_index=2;}

		color[i].RGB63_red   = color[i].red   * 4 + color[i].RGB63_red_index;
		color[i].RGB63_green = color[i].green * 4 + color[i].RGB63_green_index;
		color[i].RGB63_blue  = color[i].blue  * 4 + color[i].RGB63_blue_index;

		// copy hex values to temporary variables ///////////////////////////////

		color[i].index_temp = color[i].index;
		color[i].red_temp   = color[i].red;
		color[i].green_temp = color[i].green;
		color[i].blue_temp  = color[i].blue;

		// set all color palettes to black //////////////////////////////////////

		volMEMWORD(4194304+(i*2)+(palette_no*32)) = 0x0000;
	}
}

int insteps=-64;

void fade_in()
{
	int palette_no;
	int i=0;

	palette_no = 5; // first palette to start with

	if (insteps >= 0) return;
	insteps+=2;

	for(i=0; i<128; i++)
	{
		// add steps to RGB63 temporary values ////////////////////

		color[i].RGB63_red_temp   = color[i].RGB63_red + insteps;
		color[i].RGB63_green_temp = color[i].RGB63_green + insteps;
		color[i].RGB63_blue_temp  = color[i].RGB63_blue + insteps;

		// keep values inside the range of 0 to 63 ////////////////

		if(color[i].RGB63_red_temp>63) color[i].RGB63_red_temp=63;
		if(color[i].RGB63_red_temp<0) color[i].RGB63_red_temp=0;
		if(color[i].RGB63_green_temp>63) color[i].RGB63_green_temp=63;
		if(color[i].RGB63_green_temp<0) color[i].RGB63_green_temp=0;
		if(color[i].RGB63_blue_temp>63) color[i].RGB63_blue_temp=63;
		if(color[i].RGB63_blue_temp<0) color[i].RGB63_blue_temp=0;

		// convert RGB63 back to ASM Hex Color Code ////////////////

		color[i].RGB63_red_index_temp = color[i].RGB63_red_temp%4;
		color[i].red_temp = color[i].RGB63_red_temp/4;

		color[i].RGB63_green_index_temp = color[i].RGB63_green_temp%4;
		color[i].green_temp = color[i].RGB63_green_temp/4;

		color[i].RGB63_blue_index_temp = color[i].RGB63_blue_temp%4;
		color[i].blue_temp = color[i].RGB63_blue_temp/4;

		if(color[i].RGB63_red_index_temp==1 && color[i].RGB63_green_index_temp==1 && color[i].RGB63_blue_index_temp==1) color[i].index_temp=0;
		if(color[i].RGB63_red_index_temp==1 && color[i].RGB63_green_index_temp==1 && color[i].RGB63_blue_index_temp==3) color[i].index_temp=1;
		if(color[i].RGB63_red_index_temp==1 && color[i].RGB63_green_index_temp==3 && color[i].RGB63_blue_index_temp==1) color[i].index_temp=2;
		if(color[i].RGB63_red_index_temp==1 && color[i].RGB63_green_index_temp==3 && color[i].RGB63_blue_index_temp==3) color[i].index_temp=3;
		if(color[i].RGB63_red_index_temp==3 && color[i].RGB63_green_index_temp==1 && color[i].RGB63_blue_index_temp==1) color[i].index_temp=4;
		if(color[i].RGB63_red_index_temp==3 && color[i].RGB63_green_index_temp==1 && color[i].RGB63_blue_index_temp==3) color[i].index_temp=5;
		if(color[i].RGB63_red_index_temp==3 && color[i].RGB63_green_index_temp==3 && color[i].RGB63_blue_index_temp==1) color[i].index_temp=6;
		if(color[i].RGB63_red_index_temp==3 && color[i].RGB63_green_index_temp==3 && color[i].RGB63_blue_index_temp==3) color[i].index_temp=7;
		if(color[i].RGB63_red_index_temp==0 && color[i].RGB63_green_index_temp==0 && color[i].RGB63_blue_index_temp==0) color[i].index_temp=8;
		if(color[i].RGB63_red_index_temp==0 && color[i].RGB63_green_index_temp==0 && color[i].RGB63_blue_index_temp==2) color[i].index_temp=9;
		if(color[i].RGB63_red_index_temp==0 && color[i].RGB63_green_index_temp==2 && color[i].RGB63_blue_index_temp==0) color[i].index_temp=10;
		if(color[i].RGB63_red_index_temp==0 && color[i].RGB63_green_index_temp==2 && color[i].RGB63_blue_index_temp==2) color[i].index_temp=11;
		if(color[i].RGB63_red_index_temp==2 && color[i].RGB63_green_index_temp==0 && color[i].RGB63_blue_index_temp==0) color[i].index_temp=12;
		if(color[i].RGB63_red_index_temp==2 && color[i].RGB63_green_index_temp==0 && color[i].RGB63_blue_index_temp==2) color[i].index_temp=13;
		if(color[i].RGB63_red_index_temp==2 && color[i].RGB63_green_index_temp==2 && color[i].RGB63_blue_index_temp==0) color[i].index_temp=14;
		if(color[i].RGB63_red_index_temp==2 && color[i].RGB63_green_index_temp==2 && color[i].RGB63_blue_index_temp==2) color[i].index_temp=15;

		// write new ASM Hex Color Code to color adress //////////////

		color[i].new_color=(color[i].index_temp)*4096+(color[i].red_temp)*256+(color[i].green_temp)*16+(color[i].blue_temp);
		volMEMWORD(4194304+(i*2)+(palette_no*32)) = color[i].new_color;
	}
}

long millis = 0;
long startframe = 0;
int texto = 0;

typedef struct bkp_ram_info {
	WORD debug_dips;
	BYTE stuff[254];
	//256 bytes
} bkp_ram_info;

bkp_ram_info bkp_data;


BYTE p1,p2,ps,p1e,p2e;

const char sinTable[] = {32,34,35,37,38,40,41,43,44,46,47,48,50,51,52,53,
								55,56,57,58,59,59,60,61,62,62,63,63,63,64,64,64,
								64,64,64,64,63,63,63,62,62,61,60,59,59,58,57,56,
								55,53,52,51,50,48,47,46,44,43,41,40,38,37,35,34,
								32,30,29,27,26,24,23,21,20,18,17,16,14,13,12,11,
								9,8,7,6,5,5,4,3,2,2,1,1,1,0,0,0,
								0,0,0,0,1,1,1,2,2,3,4,5,5,6,7,8,
								9,11,12,13,14,16,17,18,20,21,23,24,26,27,29,30};


const int sintab[1024] =
{
128,129,130,130,131,132,133,133,134,135,136,137,137,138,139,140,141,141,142,143,144,144,145,146,147,148,148,149,150,151,151,152,153,154,155,155,156,157,158,158,159,160,161,161,162,163,164,164,165,166,167,167,168,169,170,170,171,172,173,173,174,175,176,176,177,178,178,179,180,181,181,182,183,183,184,185,186,186,187,188,188,189,190,190,191,192,192,193,194,194,195,196,196,197,198,198,199,200,200,201,202,202,203,204,204,205,206,206,207,207,208,209,209,210,210,211,212,212,213,213,214,215,215,216,216,217,217,218,219,219,220,220,221,221,222,222,223,223,224,224,225,225,226,226,227,227,228,228,229,229,230,230,231,231,232,232,233,233,234,234,234,235,235,236,236,237,237,237,238,238,239,239,239,240,240,241,241,241,242,242,242,243,243,243,244,244,244,245,245,245,246,246,246,247,247,247,247,248,248,248,249,249,249,249,250,250,250,250,250,251,251,251,251,252,252,252,252,252,253,253,253,253,253,253,254,254,254,254,254,254,254,254,255,255,255,255,255,255,255,255,255,255,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,256,255,255,255,255,255,255,255,255,255,255,254,254,254,254,254,254,254,254,253,253,253,253,253,253,252,252,252,252,252,251,251,251,251,250,250,250,250,250,249,249,249,249,248,248,248,247,247,247,247,246,246,246,245,245,245,244,244,244,243,243,243,242,242,242,241,241,241,240,240,239,239,239,238,238,237,237,237,236,236,235,235,234,234,234,233,233,232,232,231,231,230,230,229,229,228,228,227,227,226,226,225,225,224,224,223,223,222,222,221,221,220,220,219,219,218,217,217,216,216,215,215,214,213,213,212,212,211,210,210,209,209,208,207,207,206,206,205,204,204,203,202,202,201,200,200,199,198,198,197,196,196,195,194,194,193,192,192,191,190,190,189,188,188,187,186,186,185,184,183,183,182,181,181,180,179,178,178,177,176,176,175,174,173,173,172,171,170,170,169,168,167,167,166,165,164,164,163,162,161,161,160,159,158,158,157,156,155,155,154,153,152,151,151,150,149,148,148,147,146,145,144,144,143,142,141,141,140,139,138,137,137,136,135,134,133,133,132,131,130,130,129,128,127,126,126,125,124,123,123,122,121,120,119,119,118,117,116,115,115,114,113,112,112,111,110,109,108,108,107,106,105,105,104,103,102,101,101,100,99,98,98,97,96,95,95,94,93,92,92,91,90,89,89,88,87,86,86,85,84,83,83,82,81,80,80,79,78,78,77,76,75,75,74,73,73,72,71,70,70,69,68,68,67,66,66,65,64,64,63,62,62,61,60,60,59,58,58,57,56,56,55,54,54,53,52,52,51,50,50,49,49,48,47,47,46,46,45,44,44,43,43,42,41,41,40,40,39,39,38,37,37,36,36,35,35,34,34,33,33,32,32,31,31,30,30,29,29,28,28,27,27,26,26,25,25,24,24,23,23,22,22,22,21,21,20,20,19,19,19,18,18,17,17,17,16,16,15,15,15,14,14,14,13,13,13,12,12,12,11,11,11,10,10,10,9,9,9,9,8,8,8,7,7,7,7,6,6,6,6,6,5,5,5,5,4,4,4,4,4,3,3,3,3,3,3,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,4,4,4,4,4,5,5,5,5,6,6,6,6,6,7,7,7,7,8,8,8,9,9,9,9,10,10,10,11,11,11,12,12,12,13,13,13,14,14,14,15,15,15,16,16,17,17,17,18,18,19,19,19,20,20,21,21,22,22,22,23,23,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31,32,32,33,33,34,34,35,35,36,36,37,37,38,39,39,40,40,41,41,42,43,43,44,44,45,46,46,47,47,48,49,49,50,50,51,52,52,53,54,54,55,56,56,57,58,58,59,60,60,61,62,62,63,64,64,65,66,66,67,68,68,69,70,70,71,72,73,73,74,75,75,76,77,78,78,79,80,80,81,82,83,83,84,85,86,86,87,88,89,89,90,91,92,92,93,94,95,95,96,97,98,98,99,100,101,101,102,103,104,105,105,106,107,108,108,109,110,111,112,112,113,114,115,115,116,117,118,119,119,120,121,122,123,123,124,125,126,126,127

};

int trastart = 0;
int traframe = 0;
int targetframe = 1705;
int firstframe = 1700;
int traend = 0;
int tradir = 0;

void draw_transition() {
	int t = DAT_frameCounter-trastart;
	int i = 0;
	int x;
	int y;
	int traframea;
	if (traend == 2) return;
	if (traend == 0) {
		if (trastart == 0) trastart = DAT_frameCounter;

		if (t >= firstframe && t <= targetframe && traframe == 0)  {
			traframe++;
			traframea = traframe;
			if (tradir == 1) traframea = 16-traframe;
			for (y = 0;y<30;y++) {
					fixPrintf(0,y,0,0,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea);
			}
			targetframe = t + 1;
		}
		else if (t >= targetframe && traframe > 0)  {
			traframe++; 
			traframea = traframe;
			if (tradir == 1) { traframea = 15-traframe; if (traframea == 0) traframea = 255;}
			for (y = 0;y<30;y++) {
					fixPrintf(0,y,0,0,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea,traframea);
			}
			targetframe = t + 1;
		}
		if (traframe == 15 && tradir == 0) traend = 1;
		if (traframe == 16 && tradir == 1) traend = 2;
	} else {
		traframe++;
		if (traframe > 60) {
			traend = 0;
			tradir = 1;
			traframe = 0;
			trastart = 0;
			targetframe = 5;
			firstframe = 1;
		}
	}

}

void demopart_letter() {
	int x=FRONT_START_X;
	int y=FRONT_START_Y;
	int lx=0;
	int ly=-190;
	int scrollx=100;
	int scrolly=30;
	int tox = 7;
	int toy = 12;
	int lt = 0;
	int lt2 = 0;
	int tl = 0;
	int ee = 0;
	int backX;
	int backY;
	int t;
	int t2;
	int t3=0;
	int fixed;
	int i,j;
	WORD raster=true;
	WORD tableShift=0;
	ushort rasterData0[512];
	ushort rasterData1[512];
	ushort rasterAddr;
	ushort *dataPtr;
	short displayedRasters;

//	pictureInfo* animList[12] = { &spin1, &spin2, &spin3, &spin4, &spin5, &spin6, &spin7, &spin8, &spin9, &spin10, &spin11, &spin12 };
//	paletteInfo* palList[12] = { &spin1_Palettes, &spin2_Palettes, &spin3_Palettes, &spin4_Palettes, &spin5_Palettes, &spin6_Palettes, &spin7_Palettes, &spin8_Palettes, &spin9_Palettes, &spin10_Palettes, &spin11_Palettes, &spin12_Palettes };

	scroller backScroll, frontScroll, letterScroll;
	picture scroll;

	pictureInfo* cur;
	paletteInfo* curpal;
	const char *text[8];

	if (texto == 0) {
	text[0] = "   Hey, it's me,0000000";
	text[1] = "I don't know why I'm";
	text[2] = " writing to you now,";
	text[3] = "after so many years.0000000";
	text[4] = " How have you been?00000";
	text[5] = "I wonder if you still";
	text[6] = " think about how it";
	text[7] = "used to be back then.";
}
else {
	text[0] = "   Anyway, I've      ";
	text[1] = "learned to accept it ";
	text[2] = " and the pain subsided.  ";
	text[3] = "Felt like forever... ";
	text[4] = " At least we'll have ";
	text[5] = "the memories. Those..";
	text[6] = " Those will never go ";
	text[7] = "away. Will never die.";

}

	LSPCmode=0x1c00;	//autoanim speed

	clearFixLayer();
	initGfx();
	jobMeterSetup(true);

	loadTIirq(TI_MODE_SINGLE_DATA);

//	rasterAddr=0x8400+scroll.baseSprite;

	scrollerInit(&backScroll, &ttbg_a, 256+1, 16, (((x-8)*141)/299)+BACK_MIN_X, (((y-16)*3)/8)+BACK_MIN_Y);
	palJobPut(16, ttbg_a_Palettes.palCount, ttbg_a_Palettes.data);

	scrollerInit(&frontScroll, &ttbg_b, 256+49, 16 + ttbg_a_Palettes.palCount, x, y);
	palJobPut(16 + ttbg_a_Palettes.palCount, ttbg_b_Palettes.palCount, ttbg_b_Palettes.data);

	scrollerInit(&letterScroll, &ttbg_c, 256+49+48, 16 + ttbg_a_Palettes.palCount + ttbg_b_Palettes.palCount, lx, ly);
	palJobPut(16 + ttbg_a_Palettes.palCount + ttbg_b_Palettes.palCount, ttbg_c_Palettes.palCount, ttbg_c_Palettes.data);


//	fixPrint(2,3,0,0,"1P \x12\x13\x10\x11: scroll");

	volMEMWORD(0x400000)=0x8000; // black
	volMEMWORD(0x400002)=0xcccc; // black
	volMEMWORD(0x400004)=0x9999; // black
	volMEMWORD(0x400006)=0x2222; // black

	SCClose();

	t = 0;
	t2 = 0;

		waitVBlank();

	while(1) {
		// BIOSF_SYSTEM_IO
		//cur = animList[(tableShift/4) % 11];
		//curpal = palList[(tableShift/4) % 11];
		waitVBlank();

		// init next frame of animation, and the palette for it
//		pictureInit(&scroll, cur, 43, 16 + ttbg_a_Palettes.palCount + ttbg_b_Palettes.palCount, scrollx, scrolly, FLIP_NONE);
//		palJobPut(16 + ttbg_a_Palettes.palCount + ttbg_b_Palettes.palCount, curpal->palCount, curpal->data);

		// BIOSF_SYSTEM_IO

		while((volMEMWORD(0x3c0006)>>7)!=0x120); //wait raster line 16
		jobMeterColor(JOB_PURPLE);

		p1=volMEMBYTE(P1_CURRENT);
		p2=volMEMBYTE(P2_CURRENT);
		ps=volMEMBYTE(PS_CURRENT);
		
		if((ps&P1_START)&&(ps&P2_START)) {
			clearSprites(1, 42+ffbg_c.tileWidth);
			SCClose();
			waitVBlank();
			return;
		}

		if(p1&JOY_UP)	y--;
		if(p1&JOY_DOWN)	y++;
//		if(p1&JOY_LEFT)	x--;
//		if(p1&JOY_RIGHT)	x++;

		t2++;
		if (t2>1) { millis = DAT_frameCounter/(2); t = millis; t2=0; }

//		fixPrintf(2,3,0,0,"%d", millis);

		if (millis > 880 && texto == 0) {
			clearSprites(0, 512);
			SCClose();
			waitVBlank();

			break;
		}

		if (millis > 800 && texto == 1) {
			clearSprites(0, 512);
			SCClose();
			waitVBlank();

			break;
		}

		x=((t*1)%400);

		if (t >= 50) t3 = t-50;

		if (t3 > 0) {

			if (ly >= -15) { 
				ly = -15;
				if (tl < 9) {
					if (text[tl][lt] != '0') fixPrintf(tox+5+lt,toy+tl*2+ee,0,0,"%c",text[tl][lt]);
					lt2++;
					if (lt2>6) {lt++; lt2=0;}
					if (lt >= strlen(text[tl])) { lt = 0; tl++; if (tl == 1 && texto == 0) ee++; if (tl == 4) ee++; }
					if (tl >= 8) tl = 9;
				}
			}
			else {
				lx = -105+sintab[t3*8 % 1023]>>5;
				ly = (-198+t3)+(sintab[t3*8 % 1023]>>4);
			}
		}
		if(x<FRONT_MIN_X) x=FRONT_MIN_X;
		else if(x>FRONT_MAX_X) x=FRONT_MAX_X;
		if(y<FRONT_MIN_Y) y=FRONT_MIN_Y;
		else if(y>FRONT_MAX_Y) y=FRONT_MAX_Y;

//		pictureSetPos(&scroll,320,scrolly);

		backX=(((x-8)*141)/299)+BACK_MIN_X;
		backY=(((y-16)*3)/8)+BACK_MIN_Y;

			
		jobMeterColor(JOB_BLUE);

		scrollerSetPos(&frontScroll, sintab[t*5 % 1023]>>3, -2-ly/7);
		scrollerSetPos(&backScroll, backX, backY);
		scrollerSetPos(&letterScroll, lx+8, ly);

		SCClose();

		draw_transition();
/*
		if(raster) {
			TInextTable=(TInextTable==rasterData0)?rasterData1:rasterData0;
			dataPtr=TInextTable;
			rasterAddr=0x8400+scroll.baseSprite;

			if(p2&JOY_B) for(i=0;i<50000;i++);	//induce frameskipping

			TIbase=TI_ZERO+(scrolly>0?((384*scrolly)):0); //timing to first line

			displayedRasters=(scroll.info->tileHeight<<4)-(scrolly>=0?0:0-scrolly);
			if(displayedRasters+scrolly>224) displayedRasters=224-scrolly;
			if(displayedRasters<0) displayedRasters=0;

			i=(scrolly>=0)?0:0-scrolly;
			for(j=0;j<displayedRasters;j++) {
				*dataPtr++=rasterAddr;
				if(!(j&0x1))
					*dataPtr++=(scrollx+(sinTable[(i+tableShift)&0x3f]+32))<<4;
				else	*dataPtr++=(scrollx+(sinTable[(i+1+tableShift)&0x3f]+31))<<4;
				i++;
			}
			SC234Put(rasterAddr,scrollx<<7); //restore pos
			*dataPtr++=0x0000;
			*dataPtr++=0x0000;	//end
		} else {
			SC234Put(rasterAddr,scrollx<<7); //restore position
			TInextTable=0;
		}

		tableShift++;
*/
		tableShift++;
		jobMeterColor(JOB_GREEN);

	}

}

void demopart_phone() {
	int lt = 0;
	int initf = 0;
	int blinkcnt = 0;
	int lt2 = 0;
	int clearflag = 1;
	int tl = 0;
	const char *text[8*2];
	int x=0;
	int x2=0;
	int y=70;
	int t;
	int t2;
	int t3=0;
	int fixed;
	int tox = 5;
	int toy = 9;
	int ee = 0;
	int lx=0;
	int ly=-190;
	int i,j;
	int lp2 = 0;
	int nexts = 0;
	int scrollx=0;
	int ml=0;
	int scrolly=0;
	int offer=0;
	WORD raster=true;
	WORD tableShift=0;
	ushort rasterData0[512];
	ushort rasterData1[512];
	ushort rasterAddr;
	ushort *dataPtr;
	short displayedRasters;

	scroller frontScroll;

	scroller layerbgScroll;
	scroller layerfrScroll;
	scroller layer2Scroll;
	scroller errorScroll;
	picture cursorPic;

	picture dovrot;
	pictureInfo* cur;
	paletteInfo* curpal;

	text[  0] = "    It's really  ";
	text[  1] = "    weird to sorta";
	text[  2] = "    being talking ";
	text[  3] = "    to ones past";
	text[  4] = "    self, almost  ";
	text[  5] = "    like the cur-";
	text[  6] = "    rent didn't ";
	text[  7] = "                                                                                                                                                                                                                                                  ";


	text[8+0] = "    even exist as";
	text[8+1] = "    a whole sepa- ";
	text[8+2] = "    rate person. ";
	text[8+3] = "    I can't even";
	text[8+4] = "    recognize how";
	text[8+5] = "    different I'm";
	text[8+6] = "    and u must be.";
	text[8+7] = "    ";

	LSPCmode=0x1c00;	//autoanim speed

	initGfx();
	jobMeterSetup(true);

	loadTIirq(TI_MODE_SINGLE_DATA);

	scrollerInit(&frontScroll, &map, 1, 1, x, y);
						palJobPut(1, map_Palettes.palCount, map_Palettes.data);

	scrollerInit(&errorScroll, &title, 100, 5, 0, 0);
	palJobPut(5, title_Palettes.palCount, title_Palettes.data);
	SCClose();

	prepare_fade_in();

	scrollerInit(&layerbgScroll, &dovbg, 150, 20, 0, 45);
	palJobPut(20, dovbg_Palettes.palCount, dovbg_Palettes.data);
	scrollerInit(&layerfrScroll, &dov, 200, 30, 0, 4);
	palJobPut(30, dov_Palettes.palCount, dov_Palettes.data);


	pictureInit(&cursorPic,&cursor,232,200,50,128,FLIP_NONE);
	palJobPut(200,cursor_Palettes.palCount,&cursor_Palettes.data);
	pictureHide(&cursorPic);

	SCClose();

	t = 0;
	t2 = 0;

		waitVBlank();

		startframe = DAT_frameCounter+1;

	while(1) {
		// BIOSF_SYSTEM_IO

		waitVBlank();
			blinkcnt++;

		if (t>800) break;

		while((volMEMWORD(0x3c0006)>>7)!=0x120); //wait raster line 16
		jobMeterColor(JOB_PURPLE);

		p1=volMEMBYTE(P1_CURRENT);
		p2=volMEMBYTE(P2_CURRENT);
		ps=volMEMBYTE(PS_CURRENT);
		
		if((ps&P1_START)&&(ps&P2_START)) {
			clearSprites(1, 42+ffbg_c.tileWidth);
			SCClose();
			waitVBlank();
			return;
		}

		if(p1&JOY_UP)	scrolly--;
		if(p1&JOY_DOWN)	scrolly++;
		if(p1&JOY_LEFT)	scrollx--;
		if(p1&JOY_RIGHT) scrollx++;

		t3 = DAT_frameCounter-startframe;

		//x=t3*2;
		x+=2;
		y=t3+(sintab[t3*3 % 1023]>>2);

		if (y<=0) y = 0;
		if (x>=2272) { x = 2272; x2=(t3*2)-x;  }
		if (y>=1644-64) { y = 1644-64; } 

		millis = t3/(2); 
		t = millis;
			
		jobMeterColor(JOB_BLUE);

			if (t >= 600 && t < 800 && ml < 15) { 
				backgroundColor(0xffff);

	volMEMWORD(0x400000)=0x8000; // black
	volMEMWORD(0x400002)=0xffff; // black
	volMEMWORD(0x400004)=0xffff; // black
	volMEMWORD(0x400006)=0xffff; // black
				ly = -15;
				if (tl < 9) {
					if (text[tl+offer][lt] == '1') { ml = 50; continue; }
					if (text[tl+offer][lt] != '0') fixPrintf(tox+5+lt,toy+tl*2+ee,0,0,"%c%c%c",text[tl+offer][lt],text[tl+offer][lt+1],text[tl+offer][lt+2]);
					lt+=2;
					if (lt >= strlen(text[tl+offer])) { 
						lt = 0; tl++;
						ml++;
						if (tl >= 8) { 
							offer += tl;
							tl = 0; 	
							//clearFixLayer();
 						} 
 					}
					if (tl >= 8) tl = 9;
				}
			}


			scrolly=y+(sintab[(t3*7)%1023]>>5);

		scrollerSetPos(&frontScroll, x-x2, scrolly);

		if (t2>3) scrollerSetPos(&layerbgScroll, 0, t2+42);
		if (t2>3) scrollerSetPos(&layerfrScroll, 0, -1+t2+(sintab[(t3*7)%1023]>>5));
		
		if (t3>3 && t3 < 550) scrollerSetPos(&errorScroll, 0, 310+t2 +(sintab[(t3*7)%1023]>>5)-4);
		else if (t3>=550 && t3 < 800) scrollerSetPos(&errorScroll, 0, (sintab[(t3*7)%1023]>>5)-4);
		else if (t3 >= 800 && t3 < 890) scrollerSetPos(&errorScroll, 0-(t3-800)*3, (sintab[(t3*7)%1023]>>5)-4);

		if (t3>=500 && t<800) {

			fade_in();

			if (t3 > 550) {
				if (blinkcnt == 32) pictureHide(&cursorPic); 

				if (blinkcnt > 64) blinkcnt = 0;
				if (blinkcnt == 0) pictureShow(&cursorPic);

				if (blinkcnt < 32) pictureSetPos(&cursorPic,37,128-(sintab[(t3*7)%1023]>>5)+3);
			}
		}
		if (t3 >= 800) {
			pictureHide(&cursorPic);
		}

		lp2 = -180+(t-600)+(sintab[(t3*4)%1023]>>5);
		if (lp2 >= -20) lp2 = -20;
		if (t > 599 && initf == 0) {

			scrollerInit(&layer2Scroll, &kaverit, 262+1, 16+16, 0, -224);
			palJobPut(16+16, kaverit_Palettes.palCount, kaverit_Palettes.data);
			initf = 1;

		}
		if (t > 600) scrollerSetPos(&layer2Scroll, 0, lp2);
		if (t > 850) scrollerSetPos(&layer2Scroll, 0, lp2+(850-t));

		t2=-40+millis;
		if (t2 >= 464-262) t2=464-262;



			SCClose();

			draw_transition();


		jobMeterColor(JOB_GREEN);

	}
	SCClose();

	clearSprites(1, 512);
	waitVBlank();
}


void demopart_sakura() {
	int x=0;
	int x2=0;
	int loaded = 0;
	int y=70;
	int t;
	int once = 0;
		int tox = 0;
	int toy = 23;
	int lt = 0;
	int xx=0;
	int yy=0;
	int lt2 = 0;
	int tl = 0;
	int ee = 0;
	int t2;
	int t3=0;
	int fixed;
	int i,j;
	int prevj=-1;
	int lp2 = 0;
	int nexts = 0;
	int scrollx=0;
	int scrolly=0;
	WORD raster=true;
	WORD tableShift=0;
	ushort rasterData0[512];
	ushort rasterData1[512];
	ushort rasterAddr;
	ushort *dataPtr;
	short displayedRasters;

	scroller layerbgScroll;
	scroller layerfg1Scroll;
	scroller layerfg2Scroll;
	scroller textboxScroll;
	scroller animScroll;

	picture dovrot;
	pictureInfo* cur;
	paletteInfo* curpal;
		const char *text[5];
	text[0] = " <NARRATOR>                ";
	text[1] = " ";
	text[2] = " Here's where I've always hidden away ";
	text[3] = " when my mind needs true tranquility, ";
	text[4] = " peace, and quiet... my Zen garden... ";

	backgroundColor(0xffff); //BG color
	LSPCmode=0x1c00;	//autoanim speed

	clearFixLayer();
	initGfx();
	jobMeterSetup(true);

	loadTIirq(TI_MODE_SINGLE_DATA);

	scrollerInit(&layerbgScroll, &cherries, 1, 16, 0, 0);
	palJobPut(16, cherries_Palettes.palCount, cherries_Palettes.data);

	scrollerInit(&layerfg1Scroll, &cherries_b, 1+32, 16+16, 0,0);
	palJobPut(16+16, cherries_b_Palettes.palCount, cherries_b_Palettes.data);

//	scrollerInit(&layerfg2Scroll, &cherries_a, 1+32+32, 16+16+16, 0,0);
//	palJobPut(16+16+16, cherries_a_Palettes.palCount, cherries_a_Palettes.data);

	volMEMWORD(0x400000)=0x8000; // black
	volMEMWORD(0x400006)=0xffff; // black
	volMEMWORD(0x400002)=0x3028; // black
	volMEMWORD(0x400004)=0x254b; // black

//	volMEMWORD(0x400002)=0xcccc; // black
//	volMEMWORD(0x400004)=0x9999; // black
//	volMEMWORD(0x400006)=0x2222; // black


		scrollerSetPos(&layerfg1Scroll,-32-scrollx,-40+(scrolly*2 >> (SHIFT_AMOUNT+2)));

	SCClose();

	t = 0;
	t2 = 0;
		waitVBlank();
		waitVBlank();

		startframe = DAT_frameCounter+1;

 	while(1) {
		// BIOSF_SYSTEM_IO

		waitVBlank();
		while((volMEMWORD(0x3c0006)>>7)!=0x120); //wait raster line 16

		jobMeterColor(JOB_BLUE);

		if (t > 900) break;

		t2 = (DAT_frameCounter-startframe-yy)/2;
		if (t2 < 0) t2 = 0;
		if (once !=2) once = 0;

		if (t >= 400-6 && loaded == 0) {
			loaded = 1;
			scrollerInit(&animScroll, &sakura, 1+32+32, 16+16+16, 0,0);
			palJobPut(16+16+16, sakura_Palettes.palCount, sakura_Palettes.data);
		}

		if (t > 400-6 && t < 559-6) {
			once = 1;
			yy++;
			if (yy > 5) {
			yy = 0;
			xx++;
			if (xx > 52) xx = 0;
			scrollerSetPos(&animScroll, 0, xx*224);
			}
		}

		if (once != 2 && t >= 559-6) {
			once = 2;
			clearSprites(1+32+32,100);
			scrollerInit(&textboxScroll, &textbox, 1+32+32+32, 16+16+16+16+16, 0,-224);
			palJobPut(16+16+16+16+16, textbox_Palettes.palCount, textbox_Palettes.data);

		}

		if (t > 0 && t < 420*2) { 
			y=sintab[((t*100) << SHIFT_AMOUNT) & 1023] << SHIFT_AMOUNT-6;
			if (scrolly > 2550 << SHIFT_AMOUNT) { 
				y-=(scrolly-(2550 << SHIFT_AMOUNT))*0.005;
			}
			if (y <= 0) y = 0;
			scrolly += y;


			scrollerSetPos(&layerbgScroll,0,t/2);
		}
		if (scrolly >= 200 << SHIFT_AMOUNT) scrollx = ((scrolly-(200 << SHIFT_AMOUNT)) >> SHIFT_AMOUNT+5);
		if (scrollx > 48) scrollx = 48;
//		scrollerSetPos(&layerfg1Scroll,-32-scrollx,-40+(scrolly*2 >> (SHIFT_AMOUNT+2)));
//		scrollerSetPos(&layerfg2Scroll,48+scrollx*0.5,50+(scrolly*3 >> (SHIFT_AMOUNT+2)));
		if (t < 840) scrollerSetPos(&layerfg1Scroll,-8-t/5,-40+(scrolly*2 >> (SHIFT_AMOUNT+2)));
	

//		if (t > 30 && t < 330*2) scrollerSetPos(&layerbgScroll,0,(t-30)/2);
//		if (t > 30 && t < 490) scrollerSetPos(&layerfg1Scroll,-32,(t-30));
//		if (t > 30 && t < 488) scrollerSetPos(&layerfg2Scroll,48,(t*2-30)+40);


		if (t > 600 && t < 668)
		scrollerSetPos(&textboxScroll,0,-224+((t-600)));

		if (t >= 680) {
				if (tl < 5) {
					if (text[tl][lt] != '0') fixPrintf(tox+lt+1,toy+tl*1+ee,0,0,"%c",text[tl][lt]);
					lt2++;
					if (lt2>1) {lt++; lt2=0;}
					if (tl<6) { 
						if (lt >= strlen(text[tl])) { lt = 0; tl++;  }
					}
				}

		}

		t3 = DAT_frameCounter-startframe;

		millis = t3/(2); 
		t = millis;
		jobMeterColor(JOB_GREEN);

	SCClose();

	}

	SCClose();

	clearSprites(1, 512);
	waitVBlank();

}

void demopart_meta() {
	int lt = 0;
	int lt2 = 0;
	int tl = 0;
	int x=0;
	int xx=0;
	int x2=0;
	int x5=0;
	int x6=0;
	int y=0;
	int y5=0;
	int t;
	int t2;
	int t3=0;
	int fixed;
	int tox = 4;
	int toy = 8;
	int ee = 0;
	int lx=0;
	int ly=-190;
	int i,j;
	int lp2 = 0;
	int nexts = 0;
	int scrollx=0;
	int scrolly=0;
	int offer=0;
	WORD raster=true;
	WORD tableShift=0;
	ushort rasterData0[512];
	ushort rasterData1[512];
	ushort rasterAddr;
	ushort *dataPtr;
	short displayedRasters;

	scroller frontScroll;
	scroller frontScroll2;


	picture dovrot;
	pictureInfo* cur;
	paletteInfo* curpal;

	backgroundColor(0x0000); //BG color
	LSPCmode=0x1c00;	//autoanim speed

	clearFixLayer();
	initGfx();
	jobMeterSetup(true);

	loadTIirq(TI_MODE_SINGLE_DATA);

	scrollerInit(&frontScroll, &meta, 1, 16, x, y);
	palJobPut(16, meta_Palettes.palCount, meta_Palettes.data);
	scrollerInit(&frontScroll2, &bs, 1+32, 16+16, 0, y);
	palJobPut(16+16, bs_Palettes.palCount, bs_Palettes.data);


	volMEMWORD(0x400004)=0xeeee; // white
	volMEMWORD(0x400002)=0x4444; // black

	SCClose();

	t = 0;
	t2 = 0;

		waitVBlank();

		startframe = DAT_frameCounter+1;

	while(1) {
		// BIOSF_SYSTEM_IO

		waitVBlank();
		if (t>1450) break;

		while((volMEMWORD(0x3c0006)>>7)!=0x120); //wait raster line 16
		jobMeterColor(JOB_PURPLE);

		p1=volMEMBYTE(P1_CURRENT);
		p2=volMEMBYTE(P2_CURRENT);
		ps=volMEMBYTE(PS_CURRENT);
		
		if((ps&P1_START)&&(ps&P2_START)) {
			clearSprites(1, 42+ffbg_c.tileWidth);
			SCClose();
			waitVBlank();
			return;
		}

		if(p1&JOY_UP)	scrolly--;
		if(p1&JOY_DOWN)	scrolly++;
		if(p1&JOY_LEFT)	scrollx--;
		if(p1&JOY_RIGHT) scrollx++;

		t3 = DAT_frameCounter-startframe;

		y=sintab[(t3 << SHIFT_AMOUNT) & 1023] << SHIFT_AMOUNT-2;
		x += (y+(x2*1000)) >> (SHIFT_AMOUNT+7);
		//x+=4;
		x2 += y >> (SHIFT_AMOUNT);

		if (x>=4864-320) { 	
			//xx+=4;	
			x = 4864-320;
			xx += (y+(x2*1000)) >> (SHIFT_AMOUNT+7);
		}

		millis = t3/(2); 
		t = millis;
			
		jobMeterColor(JOB_BLUE);

		scrollerSetPos(&frontScroll, x-xx, 0);

		if ( t > 1000) {

		y5=sintab[(x5 >> SHIFT_AMOUNT+8) & 1023] << SHIFT_AMOUNT-2;
		x5 += (x6*500) >> (SHIFT_AMOUNT+7);
		x6 += y >> (SHIFT_AMOUNT-2);

		if (x5>=4864-3050) { 		
			x5 = 4864-3050;
		}

		scrollerSetPos(&frontScroll2, x5, sintab[t3*8 & 1023]>>3);
		}

		SCClose();


		jobMeterColor(JOB_GREEN);

	}

	SCClose();

	clearSprites(1, 512);
	waitVBlank();

}

void demopart_kiss() {
	int lt = 0;
	int lt2 = 0;
	int tl = 0;
	int x=-0;
	int xx=0;
	int x2=0;
	int y=0;
	int reps=0;
	int t;
	int t2;
	int t3=0;
	int fixed;
	int tox = 4;
	int toy = 8;
	int ee = 0;
	int lx=0;
	int ly=-190;
	int i,j;
	int lp2 = 0;
	int nexts = 0;
	int scrollx=0;
	int scrolly=0;
	int offer=0;
	WORD raster=true;
	WORD tableShift=0;
	ushort rasterData0[512];
	ushort rasterData1[512];
	ushort rasterAddr;
	ushort *dataPtr;
	short displayedRasters;

	scroller c1;
	scroller c2;

	picture dovrot;
	pictureInfo* cur;
	paletteInfo* curpal;

	backgroundColor(0x77aa); //BG color
	LSPCmode=0x1c00;	//autoanim speed

	clearFixLayer();

	initGfx();
	jobMeterSetup(true);

	loadTIirq(TI_MODE_SINGLE_DATA);

	scrollerInit(&kissScroll, &kiss, 1, 1, x, y);
	palJobPut(1, kiss_Palettes.palCount, kiss_Palettes.data);

	scrollerInit(&c1, &credits1, 1+205, 3, 0, -256);
	palJobPut(3, credits1_Palettes.palCount, credits1_Palettes.data);

	scrollerInit(&c2, &credits2, 1+205+32, 5, 0, -256);
	palJobPut(5, credits2_Palettes.palCount, credits2_Palettes.data);

	volMEMWORD(0x400004)=0xeeee; // white
	volMEMWORD(0x400002)=0x4444; // black

		scrollerSetPos(&c1,-4,-240);
		scrollerSetPos(&c2,4,-240);

	SCClose();

	t = 0;
	t2 = 0;

		waitVBlank();

		startframe = DAT_frameCounter+1;

	while(1) {
		// BIOSF_SYSTEM_IO

		waitVBlank();

		if (t>1840) break;

		while((volMEMWORD(0x3c0006)>>7)!=0x120); //wait raster line 16
		jobMeterColor(JOB_PURPLE);

		p1=volMEMBYTE(P1_CURRENT);
		p2=volMEMBYTE(P2_CURRENT);
		ps=volMEMBYTE(PS_CURRENT);
		
		if((ps&P1_START)&&(ps&P2_START)) {
			clearSprites(1, 42+ffbg_c.tileWidth);
			SCClose();
			waitVBlank();
			return;
		}

		if(p1&JOY_UP)	scrolly--;
		if(p1&JOY_DOWN)	scrolly++;
		if(p1&JOY_LEFT)	scrollx--;
		if(p1&JOY_RIGHT) scrollx++;

		t3 = DAT_frameCounter-startframe;
		t2 = DAT_frameCounter-startframe-y;

		if (reps < 255){
		if (lp2 == 0) {
			if (t2/4 > 0 && x < 38*256) x = ((t2/4)%39) *256;
			if (t2/4 > 39*3) { x = ((t2/4)%39) *256; xx = 9984 - x; if (xx <=256) {y = t2+250; xx = 0; x=0; } reps++; }

		if (xx == 0) scrollerSetPos(&kissScroll, 0, x);
			else if (xx > 0) scrollerSetPos(&kissScroll, 0, xx);

		}
		}

		if (t3 < 680*2) 
		scrollerSetPos(&c1,-4,-240+t3/1.7);
		if (t3 < 770*2) 
		scrollerSetPos(&c2,4,-240+t3/1.62);

		if (t3 > 740*2) break;

		millis = t3/(2); 
		t = millis;
			
		jobMeterColor(JOB_BLUE);

		for (i = 0; i < 10; i++) {

		backgroundColor(0x77aa-(((t+i/2))>>5)); //BG color
		}




		SCClose();



		jobMeterColor(JOB_GREEN);

	}

	SCClose();
	fade_out();
	waitVBlank();
	SCClose();
	waitVBlank();

	clearSprites(1, 512);
	SCClose();
	waitVBlank();


}


/*void scrollerDemo() {
	int x=FRONT_START_X;
	int y=FRONT_START_Y;
	int carx=100;
	int cary=30;
	int backX;
	int backY;
	int t;
	int t2;
	int fixed;
	int i,j;
	WORD raster=true;
	WORD tableShift=0;
	ushort rasterData0[512];
	ushort rasterData1[512];
	ushort rasterAddr;
	ushort *dataPtr;
	short displayedRasters;

	pictureInfo* animList[12] = { &spin1, &spin2, &spin3, &spin4, &spin5, &spin6, &spin7, &spin8, &spin9, &spin10, &spin11, &spin12 };
	paletteInfo* palList[12] = { &spin1_Palettes, &spin2_Palettes, &spin3_Palettes, &spin4_Palettes, &spin5_Palettes, &spin6_Palettes, &spin7_Palettes, &spin8_Palettes, &spin9_Palettes, &spin10_Palettes, &spin11_Palettes, &spin12_Palettes };

	scroller backScroll, frontScroll;
	picture car;
		pictureInfo* cur;
		paletteInfo* curpal;


	backgroundColor(0x7bbb); //BG color
	LSPCmode=0x1c00;	//autoanim speed

	clearFixLayer();
	initGfx();
	jobMeterSetup(true);

	loadTIirq(TI_MODE_SINGLE_DATA);

	rasterAddr=0x8400+car.baseSprite;

	scrollerInit(&backScroll, &ffbg_a, 1, 16, (((x-8)*141)/299)+BACK_MIN_X, (((y-16)*3)/8)+BACK_MIN_Y);
	palJobPut(16, ffbg_a_Palettes.palCount, ffbg_a_Palettes.data);

	scrollerInit(&frontScroll, &ffbg_b, 22, 16 + ffbg_a_Palettes.palCount, x, y);
	palJobPut(16 + ffbg_a_Palettes.palCount, ffbg_b_Palettes.palCount, ffbg_b_Palettes.data);



//	fixPrint(2,3,0,0,"1P \x12\x13\x10\x11: scroll");
	fixPrintf(20,3,0,0,"GO Demo on Neo-Geo");

	SCClose();

	t = 0;
	t2 = 0;

		waitVBlank();
		// BIOSF_SYSTEM_IO
		asm("jsr 0xC0044A");


	while(1) {
		asm("jsr 0xC0044A");
		cur = animList[(tableShift/4) % 11];
		curpal = palList[(tableShift/4) % 11];
		waitVBlank();
		pictureInit(&car, cur, 43, 16 + ffbg_a_Palettes.palCount + ffbg_b_Palettes.palCount, carx, cary, FLIP_NONE);
		palJobPut(16 + ffbg_a_Palettes.palCount + ffbg_b_Palettes.palCount, curpal->palCount, curpal->data);

		// BIOSF_SYSTEM_IO

		while((volMEMWORD(0x3c0006)>>7)!=0x120); //wait raster line 16
		jobMeterColor(JOB_PURPLE);

		p1=volMEMBYTE(P1_CURRENT);
		p2=volMEMBYTE(P2_CURRENT);
		ps=volMEMBYTE(PS_CURRENT);
		
		if((ps&P1_START)&&(ps&P2_START)) {
			clearSprites(1, 42+ffbg_c.tileWidth);
			SCClose();
			waitVBlank();
			return;
		}

		if(p1&JOY_UP)	y--;
		if(p1&JOY_DOWN)	y++;
//		if(p1&JOY_LEFT)	x--;
//		if(p1&JOY_RIGHT)	x++;

		t2++;
		if (t2>1) { t++; t2=0; }

		x=(sintab[t&1023]);


		if(x<FRONT_MIN_X) x=FRONT_MIN_X;
		else if(x>FRONT_MAX_X) x=FRONT_MAX_X;
		if(y<FRONT_MIN_Y) y=FRONT_MIN_Y;
		else if(y>FRONT_MAX_Y) y=FRONT_MAX_Y;

		pictureSetPos(&car,320,cary);

		backX=(((x-8)*141)/299)+BACK_MIN_X;
		backY=(((y-16)*3)/8)+BACK_MIN_Y;

			
		jobMeterColor(JOB_BLUE);
		scrollerSetPos(&frontScroll, x, y);
		scrollerSetPos(&backScroll, backX, backY);
		SCClose();

		if(raster) {
			TInextTable=(TInextTable==rasterData0)?rasterData1:rasterData0;
			dataPtr=TInextTable;
			rasterAddr=0x8400+car.baseSprite;

			if(p2&JOY_B) for(i=0;i<50000;i++);	//induce frameskipping

			TIbase=TI_ZERO+(cary>0?((384*cary)):0); //timing to first line

			displayedRasters=(car.info->tileHeight<<4)-(cary>=0?0:0-cary);
			if(displayedRasters+cary>224) displayedRasters=224-cary;
			if(displayedRasters<0) displayedRasters=0;

			i=(cary>=0)?0:0-cary;
			for(j=0;j<displayedRasters;j++) {
				*dataPtr++=rasterAddr;
				if(!(j&0x1))
					*dataPtr++=(carx+(sinTable[(i+tableShift)&0x3f]+32))<<4;
				else	*dataPtr++=(carx+(sinTable[(i+1+tableShift)&0x3f]+31))<<4;
				i++;
			}
			SC234Put(rasterAddr,carx<<7); //restore pos
			*dataPtr++=0x0000;
			*dataPtr++=0x0000;	//end
		} else {
			SC234Put(rasterAddr,carx<<7); //restore position
			TInextTable=0;
		}

		tableShift++;

		jobMeterColor(JOB_GREEN);
	}
}
*/
void sortSprites(aSprite *list[], int count) {
	//insertion sort
	int x,y;
	aSprite *tmp;
	
	for(x=1;x<count;x++) {
		y=x;
		while(y>0 && (list[y]->posY < list[y-1]->posY)) {
			tmp=list[y];
			list[y]=list[y-1];
			list[y-1]=tmp;
			y--;
		}
	}
}

#define POOL_MODE
#define LOTS

void demopart_sprite() {
	int x=87;
	int y=136;
	int i=0;
	int t =0;
	int t3=0;
	int relX,relY;
	short showdebug=false;
	scroller frontScroll;

	spritePool testPool;
	uint *drawTable[16+(6*6)+100];
	uint *drawTablePtr;
	int sortSize;
	aSprite sprites[6*6];
	picture ptr,tleft,bright;
	short way1=JOY_UP,way2=JOY_UP;
	short visible=true;

	initGfx();
	jobMeterSetup(true);

	for (i = 0; i < 6*6; i++) {
		x = 0;
		if (y%2 == 1) x = 11;
		aSpriteInit(&sprites[i],&bmary_spr,9,16,120+(i%6 *22)+x,120,i % 40,FLIP_NONE); 
	}

	palJobPut(16,bmary_spr_Palettes.palCount,&bmary_spr_Palettes.data);

	scrollerInit(&frontScroll, &isobg, 1, 32, 0, 0);
	palJobPut(32, isobg_Palettes.palCount, isobg_Palettes.data);

	spritePoolInit(&testPool,200,80);	//54 100
	drawTablePtr=(int*)drawTable;
	*drawTablePtr++=0;
		for (i = 0; i < 6*6; i++) {
			*drawTablePtr++=(uint)&sprites[i];
		}
		sortSize=(6*6);
	*drawTablePtr=0;

	//sortSprites((aSprite*)&drawTable[1],sortSize);
	spritePoolDrawList(&testPool,&drawTable[1]);
	spritePoolClose(&testPool);

	SCClose();

	startframe = DAT_frameCounter+1;

	while(1) {
		waitVBlank();
	backgroundColor(0x0000+t*1); //BG color

		p1=volMEMBYTE(P1_CURRENT);
		p1e=volMEMBYTE(P1_EDGE);
		p2=volMEMBYTE(P2_EDGE);
		ps=volMEMBYTE(PS_CURRENT);

		if((ps&P1_START)&&(ps&P2_START)) {
			clearSprites(1, 150);
			clearSprites(200, 3);
			SCClose();
			waitVBlank();
			return;
		}

		if(p1&JOY_UP)	y--;
		if(p1&JOY_DOWN)	y++;
		if(p1&JOY_LEFT)	x--;
		if(p1&JOY_RIGHT)	x++;

		while((volMEMWORD(0x3c0006)>>7)!=0x120); //wait raster line 16
		jobMeterColor(JOB_BLUE);

		//while((volMEMWORD(0x3c0006)>>7)!=0x120); //wait raster line 16
//		sortSprites(&drawTable[1],sortSize);
		jobMeterColor(JOB_PINK);

		for (i = 0; i < 6*6; i++) {
			aSpriteSetPos(&sprites[i],30+(((sintab[i*1+t&1023]+sintab[i*1+t&1023]))%15 * 21), i*2+(21+(sintab[i*4+(t+256)&1023]))-100);
		}

		if(testPool.way==WAY_UP)
				spritePoolDrawList(&testPool,&drawTable[1]);
		else	spritePoolDrawList(&testPool,drawTablePtr);

		spritePoolClose(&testPool);

		t3 = DAT_frameCounter-startframe;

		millis = t3*(t3*0.00007); 
		t = millis;

		scrollerSetPos(&frontScroll, 0, 0);

		SCClose();

				draw_transition();



	}
}


void demopart_letter2() {
		int x=FRONT_START_X;
	int y=FRONT_START_Y;
	int lx=0;
	int ly=-190;
	int scrollx=100;
	int scrolly=30;
	int tox = 7;
	int toy = 12;
	int lt = 0;
	int lt2 = 0;
	int tl = 0;
	int ee = 0;
	int backX;
	int backY;
	int t;
	int t2;
	int t3=0;
	int fixed;
	int i,j;
	WORD raster=true;
	WORD tableShift=0;
	ushort rasterData0[512];
	ushort rasterData1[512];
	ushort rasterAddr;
	ushort *dataPtr;
	short displayedRasters;

//	pictureInfo* animList[12] = { &spin1, &spin2, &spin3, &spin4, &spin5, &spin6, &spin7, &spin8, &spin9, &spin10, &spin11, &spin12 };
//	paletteInfo* palList[12] = { &spin1_Palettes, &spin2_Palettes, &spin3_Palettes, &spin4_Palettes, &spin5_Palettes, &spin6_Palettes, &spin7_Palettes, &spin8_Palettes, &spin9_Palettes, &spin10_Palettes, &spin11_Palettes, &spin12_Palettes };

	scroller backScroll, frontScroll, letterScroll;
	picture scroll;

	pictureInfo* cur;
	paletteInfo* curpal;
	const char *text[8];

	text[0] = "   Anyway, I've      ";
	text[1] = "learned to accept it ";
	text[2] = "and the pain subsided.";
	text[3] = "Felt like forever... ";
	text[4] = " At least we'll have ";
	text[5] = "the memories. Those..";
	text[6] = " Those will never go ";
	text[7] = "away. Will never die.";

	LSPCmode=0x1c00;	//autoanim speed

	clearFixLayer();
	initGfx();
	jobMeterSetup(true);

	loadTIirq(TI_MODE_SINGLE_DATA);

//	rasterAddr=0x8400+scroll.baseSprite;

	scrollerInit(&backScroll, &school, 256+1, 16, 0, 0);
	palJobPut(16, school_Palettes.palCount, school_Palettes.data);

	scrollerInit(&frontScroll, &tree, 256+49, 16 + school_Palettes.palCount, 0, -16);
	palJobPut(16 + school_Palettes.palCount,tree_Palettes.palCount, tree_Palettes.data);

	scrollerInit(&letterScroll, &ttbg_c, 256+49+48, 16 + school_Palettes.palCount + tree_Palettes.palCount, lx, ly);
	palJobPut(16 + school_Palettes.palCount + tree_Palettes.palCount, ttbg_c_Palettes.palCount, ttbg_c_Palettes.data);


//	fixPrint(2,3,0,0,"1P \x12\x13\x10\x11: scroll");

	volMEMWORD(0x400000)=0x8000; // black
	volMEMWORD(0x400002)=0xcccc; // black
	volMEMWORD(0x400004)=0x9999; // black
	volMEMWORD(0x400006)=0x2222; // black
	SCClose();

	t = 0;
	t2 = 0;

		waitVBlank();

	while(1) {
		// BIOSF_SYSTEM_IO
		//cur = animList[(tableShift/4) % 11];
		//curpal = palList[(tableShift/4) % 11];
		waitVBlank();


		// init next frame of animation, and the palette for it
//		pictureInit(&scroll, cur, 43, 16 + ttbg_a_Palettes.palCount + ttbg_b_Palettes.palCount, scrollx, scrolly, FLIP_NONE);
//		palJobPut(16 + ttbg_a_Palettes.palCount + ttbg_b_Palettes.palCount, curpal->palCount, curpal->data);

		// BIOSF_SYSTEM_IO

		while((volMEMWORD(0x3c0006)>>7)!=0x120); //wait raster line 16
		jobMeterColor(JOB_PURPLE);

		p1=volMEMBYTE(P1_CURRENT);
		p2=volMEMBYTE(P2_CURRENT);
		ps=volMEMBYTE(PS_CURRENT);
		
		if((ps&P1_START)&&(ps&P2_START)) {
			clearSprites(1, 42+ffbg_c.tileWidth);
			SCClose();
			waitVBlank();
			return;
		}

		if(p1&JOY_UP)	y--;
		if(p1&JOY_DOWN)	y++;
//		if(p1&JOY_LEFT)	x--;
//		if(p1&JOY_RIGHT)	x++;

		t2++;
		if (t2>1) { millis = DAT_frameCounter/(2); t = millis; t2=0; }

//		fixPrintf(2,3,0,0,"%d", millis);

		if (millis > 880 && texto == 0) {
			clearSprites(0, 512);
			SCClose();
			waitVBlank();

			break;
		}

		if (millis > 880 && texto == 1) {
			clearSprites(0, 512);
			SCClose();
			waitVBlank();

			break;
		}

		x=((t*1)%400);

		if (t >= 50) t3 = t-50;

		if (t3 > 0) {

			if (ly >= -15) { 
				ly = -15;
				if (tl < 9) {
					if (text[tl][lt] != '0') fixPrintf(tox+5+lt,toy+tl*2+ee,0,0,"%c",text[tl][lt]);
					lt2++;
					if (lt2>6) {lt++; lt2=0;}
					if (lt >= strlen(text[tl])) { lt = 0; tl++; if (tl == 1 && texto == 0) ee++; if (tl == 4) ee++; }
					if (tl >= 8) tl = 9;
				}
			}
			else {
				lx = -105+sintab[t3*8 % 1023]>>5;
				ly = (-198+t3)+(sintab[t3*8 % 1023]>>4);
			}
		}
		if(x<FRONT_MIN_X) x=FRONT_MIN_X;
		else if(x>FRONT_MAX_X) x=FRONT_MAX_X;
		if(y<FRONT_MIN_Y) y=FRONT_MIN_Y;
		else if(y>FRONT_MAX_Y) y=FRONT_MAX_Y;

//		pictureSetPos(&scroll,320,scrolly);

		backX=(((x-8)*141)/299)+BACK_MIN_X;
		backY=(((y-16)*3)/8)+BACK_MIN_Y;

			
		jobMeterColor(JOB_BLUE);

		scrollerSetPos(&backScroll, 0, 0);
		scrollerSetPos(&letterScroll, lx+8, ly);

		SCClose();

		draw_transition();
/*
		if(raster) {
			TInextTable=(TInextTable==rasterData0)?rasterData1:rasterData0;
			dataPtr=TInextTable;
			rasterAddr=0x8400+scroll.baseSprite;

			if(p2&JOY_B) for(i=0;i<50000;i++);	//induce frameskipping

			TIbase=TI_ZERO+(scrolly>0?((384*scrolly)):0); //timing to first line

			displayedRasters=(scroll.info->tileHeight<<4)-(scrolly>=0?0:0-scrolly);
			if(displayedRasters+scrolly>224) displayedRasters=224-scrolly;
			if(displayedRasters<0) displayedRasters=0;

			i=(scrolly>=0)?0:0-scrolly;
			for(j=0;j<displayedRasters;j++) {
				*dataPtr++=rasterAddr;
				if(!(j&0x1))
					*dataPtr++=(scrollx+(sinTable[(i+tableShift)&0x3f]+32))<<4;
				else	*dataPtr++=(scrollx+(sinTable[(i+1+tableShift)&0x3f]+31))<<4;
				i++;
			}
			SC234Put(rasterAddr,scrollx<<7); //restore pos
			*dataPtr++=0x0000;
			*dataPtr++=0x0000;	//end
		} else {
			SC234Put(rasterAddr,scrollx<<7); //restore position
			TInextTable=0;
		}

		tableShift++;
*/
		tableShift++;
		jobMeterColor(JOB_GREEN);

	}


}

void demopart_4k() {
	int x=87;
	int y=136;
	int i=0;
	int t =0;
	int t3=0;
	int relX,relY;
	short showdebug=false;
	scroller frontScroll;

	spritePool testPool;
	uint *drawTable[16+(6*6)+100];
	uint *drawTablePtr;
	int sortSize;
	aSprite sprites[6*6];
	picture ptr,tleft,bright;
	short way1=JOY_UP,way2=JOY_UP;
	short visible=true;

	initGfx();
	jobMeterSetup(true);

	scrollerInit(&frontScroll, &fourk, 1, 32, 0, 0);
	palJobPut(32, fourk_Palettes.palCount, fourk_Palettes.data);

	SCClose();

	startframe = DAT_frameCounter+1;

	while(1) {
		waitVBlank();
	backgroundColor(0x0000+t*1); //BG color

		p1=volMEMBYTE(P1_CURRENT);
		p1e=volMEMBYTE(P1_EDGE);
		p2=volMEMBYTE(P2_EDGE);
		ps=volMEMBYTE(PS_CURRENT);

		if((ps&P1_START)&&(ps&P2_START)) {
			clearSprites(1, 150);
			clearSprites(200, 3);
			SCClose();
			waitVBlank();
			return;
		}

		if(p1&JOY_UP)	y--;
		if(p1&JOY_DOWN)	y++;
		if(p1&JOY_LEFT)	x--;
		if(p1&JOY_RIGHT)	x++;

		while((volMEMWORD(0x3c0006)>>7)!=0x120); //wait raster line 16
		jobMeterColor(JOB_BLUE);

		//while((volMEMWORD(0x3c0006)>>7)!=0x120); //wait raster line 16
//		sortSprites(&drawTable[1],sortSize);
		jobMeterColor(JOB_PINK);


		t3 = DAT_frameCounter-startframe;

		millis = t3*(t3*0.00007); 
		t = millis;

		scrollerSetPos(&frontScroll, 0, 0);

		SCClose();

				draw_transition();



	}
}


void startDemologic() {
	int i = 0;

	// call cdda, start playing track 2

	asm("clr.w %d0");
	asm("move.w #0x0502,%d0");
	asm("jsr 0xC0056A");


//	demopart_4k();

	demopart_letter();

	demopart_phone();

	demopart_sakura();

	demopart_meta();

	demopart_kiss();

	texto = 1;
	trastart = 0;
	traframe = 0;
	targetframe = 1705;
	firstframe = 1700;
	traend = 0;
	tradir = 0;

	demopart_letter2();

	demopart_sprite();


}


int main(void) {
		startDemologic();		
}
