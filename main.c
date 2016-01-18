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
	text[2] = " and pain subsided.  ";
	text[3] = "Felt like forever... ";
	text[4] = " At least we'll have ";
	text[5] = "the memories. Those..";
	text[6] = " Those will never go ";
	text[7] = "away. Will never die.";

}

	backgroundColor(0xffff); //BG color
	LSPCmode=0x1c00;	//autoanim speed

	clearFixLayer();
	initGfx();

	loadTIirq(TI_MODE_SINGLE_DATA);

//	rasterAddr=0x8400+scroll.baseSprite;

	scrollerInit(&backScroll, &ttbg_a, 256+1, 16, (((x-8)*141)/299)+BACK_MIN_X, (((y-16)*3)/8)+BACK_MIN_Y);
	palJobPut(16, ttbg_a_Palettes.palCount, ttbg_a_Palettes.data);

	scrollerInit(&frontScroll, &ttbg_b, 256+49, 16 + ttbg_a_Palettes.palCount, x, y);
	palJobPut(16 + ttbg_a_Palettes.palCount, ttbg_b_Palettes.palCount, ttbg_b_Palettes.data);

	scrollerInit(&letterScroll, &ttbg_c, 256+49+48, 16 + ttbg_a_Palettes.palCount + ttbg_b_Palettes.palCount, lx, ly);
	palJobPut(16 + ttbg_a_Palettes.palCount + ttbg_b_Palettes.palCount, ttbg_c_Palettes.palCount, ttbg_c_Palettes.data);


//	fixPrint(2,3,0,0,"1P \x12\x13\x10\x11: scroll");

	volMEMWORD(0x400004)=0xeeee; // white
	volMEMWORD(0x400002)=0x4444; // black


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

		if (millis > 900 && texto == 0) {
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

		scrollerSetPos(&frontScroll, sintab[t*3 % 1023]>>3, 0);
		scrollerSetPos(&backScroll, backX, backY);
		scrollerSetPos(&letterScroll, lx+8, ly);

		SCClose();
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
	int tox = 4;
	int toy = 8;
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


	picture dovrot;
	pictureInfo* cur;
	paletteInfo* curpal;

	text[0] = "     I don't real";
	text[1] = "     remeber how ";
	text[2] = "      it all got   ";
	text[3] = "     stad... over  ";
	text[4] = "       sing petty  ";
	text[5] = "     anstupid the ";
	text[6] = "     laer schef  ";
	text[7] = "     thin bet.  ";

	text[8+0] = "    yea, I k. A  ";
	text[8+1] = "    bren rord, a  ";
	text[8+2] = "    game thasst   ";
	text[8+3] = "    all tun ftor.";
	text[8+4] = "    Someng truly  ";
	text[8+5] = "    rvescent u no. 1";
	text[8+6] = "     ";
	text[8+7] = "     ";
	text[8+8] = "    ";

	backgroundColor(0xffff); //BG color
	LSPCmode=0x1c00;	//autoanim speed

	clearFixLayer();
	initGfx();

	loadTIirq(TI_MODE_SINGLE_DATA);

	scrollerInit(&frontScroll, &map, 1, 16, x, y);
	palJobPut(16, map_Palettes.palCount, map_Palettes.data);
	scrollerInit(&layer2Scroll, &kaverit, 262+1, 16+16, 0, -224);
	palJobPut(16+16, kaverit_Palettes.palCount, kaverit_Palettes.data);

	scrollerInit(&layerbgScroll, &dovbg, 262+1+42, 16+16+16, 0, 45);
	palJobPut(16+16+16, dovbg_Palettes.palCount, dovbg_Palettes.data);
	scrollerInit(&layerfrScroll, &dov, 262+1+42+42, 16+16+16+16, 0, 4);
	palJobPut(16+16+16+16, dov_Palettes.palCount, dov_Palettes.data);


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
		if (t>840) break;

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

		x=t3*2;
		y=t3+(sintab[t3*3 % 1023]>>2);

		if (y<=0) y = 0;
		if (x>=2272) { x = 2272; x2=(t3*2)-x;  }
		if (y>=1644-64) { y = 1644-64; } 

		millis = t3/(2); 
		t = millis;
			
		jobMeterColor(JOB_BLUE);

			if (t >= 760 && t < 835 && ml < 15) { 
				ly = -15;
				if (tl < 9) {
					if (text[tl+offer][lt] == '1') { ml = 50; continue; }
					if (text[tl+offer][lt] != '0') fixPrintf(tox+5+lt,toy+tl*2+ee,0,0,"%c%c%c",text[tl+offer][lt],text[tl+offer][lt+1],text[tl+offer][lt+2]);
					lt+=3;
					if (lt >= strlen(text[tl+offer])) { 
						lt = 0; tl++;
						ml++;
						if (tl >= 8) { 
							offer += tl;
							tl = 0; 	
							clearFixLayer();
 						} 
 					}
					if (tl >= 8) tl = 9;
				}
			}
			else {
				lx = -105+sintab[t3*8 % 1023]>>5;
				ly = (-198+t3)+(sintab[t3*8 % 1023]>>4);
			}


			scrolly=y+(sintab[(t3*7)%1023]>>5);

		scrollerSetPos(&frontScroll, x-x2, scrolly);

		if (t2>3) scrollerSetPos(&layerbgScroll, 0, t2+42);
		if (t2>3) scrollerSetPos(&layerfrScroll, 0, -1+t2+(sintab[(t3*7)%1023]>>5));

		lp2 = -180+(t-600)+(sintab[(t3*4)%1023]>>5);
		if (lp2 >= -20) lp2 = -20;
		if (t > 600) scrollerSetPos(&layer2Scroll, 0, lp2);
		if (t > 850) scrollerSetPos(&layer2Scroll, 0, lp2+(850-t));

		t2=-40+millis;
		if (t2 >= 464-260) t2=464-260;

/*
		if (t > 838) {
			if (clearflag == 1) {
			clearflag = 0;
				scrollerInit(&errorScroll, &error, 1, 16, 0, 20);
				palJobPut(16, error_Palettes.palCount, error_Palettes.data);
	 	scrollerSetPos(&errorScroll,0,0);
}
		}
*/


			SCClose();


		jobMeterColor(JOB_GREEN);

	}
	SCClose();

	clearSprites(1, 512);
	waitVBlank();
}


void demopart_sakura() {
	int x=0;
	int x2=0;
	int y=70;
	int t;
	int once = 0;
		int tox = 0;
	int toy = 24;
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

	backgroundColor(0x0000); //BG color
	LSPCmode=0x1c00;	//autoanim speed

	clearFixLayer();
	initGfx();

	loadTIirq(TI_MODE_SINGLE_DATA);

	scrollerInit(&layerbgScroll, &cherries, 1, 16, 0, 0);
	palJobPut(16, cherries_Palettes.palCount, cherries_Palettes.data);

	scrollerInit(&layerfg1Scroll, &cherries_b, 1+32, 16+16, 0,0);
	palJobPut(16+16, cherries_b_Palettes.palCount, cherries_b_Palettes.data);

	scrollerInit(&layerfg2Scroll, &cherries_a, 1+32+32, 16+16+16, 0,0);
	palJobPut(16+16+16, cherries_a_Palettes.palCount, cherries_a_Palettes.data);

	scrollerInit(&animScroll, &sakura, 1+32+32+32, 16+16+16+16, -32,0);
	palJobPut(16+16+16+16, sakura_Palettes.palCount, sakura_Palettes.data);



	volMEMWORD(0x400004)=0xffff; // white
	volMEMWORD(0x400002)=0xffff; // black

		scrollerSetPos(&layerfg1Scroll,-32-(scrolly >> SHIFT_AMOUNT+5),-40+(scrolly*2 >> (SHIFT_AMOUNT+4)));
		scrollerSetPos(&layerfg2Scroll,48+(scrolly >> SHIFT_AMOUNT+5),50+(scrolly*3 >> (SHIFT_AMOUNT+4)));

	SCClose();

	t = 0;
	t2 = 0;
		waitVBlank();

		startframe = DAT_frameCounter+1;

 	while(1) {
		// BIOSF_SYSTEM_IO

		waitVBlank();
/*
		if (t < 30) { 

		if (j < 6) j = (t3/10) % 7;
		else j = 6;

		if (j != prevj) { 
			prevj = j;

			cur = animList[j];
			curpal = palList[j];

			pictureInit(&dovrot, cur, 32, 16 + 16, 0, 0, FLIP_NONE);
			palJobPut(16+16, curpal->palCount, curpal->data);
		}
*/
//		if (t < 30) pictureSetPos(&dovrot,-j*5,t*7 );
//		} else {
//			if (t<31)pictureHide(&dovrot);
//		}

		if (t > 900) break;

		t2 = (DAT_frameCounter-startframe-yy)/2;
		if (once !=2) once = 0;

		if (t >400 && t < 559) {
			once = 1;
			yy++;
			if (yy > 5) {
			yy = 0;
			xx++;
			if (xx > 52) xx = 0;
			scrollerSetPos(&animScroll, 0, xx*224);
			}
		}

		if (once == 0 && t >= 559) {
			once = 2;
			clearSprites(1+32+32+32,100);
			scrollerInit(&textboxScroll, &textbox, 1+32+32+32+32, 16+16+16+16+16, 0,-224);
			palJobPut(16+16+16+16+16, textbox_Palettes.palCount, textbox_Palettes.data);

		}

		if (t > 0 && t < 330*2) { 
			y=sintab[(t << SHIFT_AMOUNT) & 1023] << SHIFT_AMOUNT-7;
			if (scrolly > 1200 << SHIFT_AMOUNT) { 
				y-=(scrolly-(1200 << SHIFT_AMOUNT))*0.02;
				if (y <= 0) y = 0;
			}
			scrolly += y;


			scrollerSetPos(&layerbgScroll,0,scrolly >> (SHIFT_AMOUNT+2));
		}
		if (scrolly >= 200 << SHIFT_AMOUNT) scrollx = ((scrolly-(200 << SHIFT_AMOUNT)) >> SHIFT_AMOUNT+5);
		if (scrollx > 20) scrollx = 20;
		scrollerSetPos(&layerfg1Scroll,-32-scrollx,-40+(scrolly*2 >> (SHIFT_AMOUNT+2)));
		scrollerSetPos(&layerfg2Scroll,48+scrollx*0.5,50+(scrolly*3 >> (SHIFT_AMOUNT+2)));
//		scrollerSetPos(&layerfg1Scroll,-32,-40+(scrolly*2 >> (SHIFT_AMOUNT+2)));
//		scrollerSetPos(&layerfg2Scroll,0,50+(scrolly*3 >> (SHIFT_AMOUNT+2)));


//		if (t > 30 && t < 330*2) scrollerSetPos(&layerbgScroll,0,(t-30)/2);
//		if (t > 30 && t < 490) scrollerSetPos(&layerfg1Scroll,-32,(t-30));
//		if (t > 30 && t < 488) scrollerSetPos(&layerfg2Scroll,48,(t*2-30)+40);


		if (t > 600 && t < 663)
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

		t3 = DAT_frameCounter+DAT_droppedFrames-startframe;

		y=sintab[(t3 << SHIFT_AMOUNT) & 1023] << SHIFT_AMOUNT-2;
		x += (y+(x2*1000)) >> (SHIFT_AMOUNT+7);
		x2 += y >> (SHIFT_AMOUNT);

		if (x>=4864-320) { 		
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
	scroller c1;
	scroller c2;

	picture dovrot;
	pictureInfo* cur;
	paletteInfo* curpal;

	backgroundColor(0x77aa); //BG color
	LSPCmode=0x1c00;	//autoanim speed

	clearFixLayer();

	initGfx();

	loadTIirq(TI_MODE_SINGLE_DATA);

	scrollerInit(&frontScroll, &kiss, 1, 16, x, y);
	palJobPut(16, kiss_Palettes.palCount, kiss_Palettes.data);

	scrollerInit(&c1, &credits1, 1+205, 16+16, 0, -256);
	palJobPut(16+16, credits1_Palettes.palCount, credits1_Palettes.data);

	scrollerInit(&c2, &credits2, 1+205+32, 16+16+16, 0, -256);
	palJobPut(16+16+16, credits2_Palettes.palCount, credits2_Palettes.data);

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

		if (lp2 == 0) {
			if (t2/4 > 0 && x < 38*256) x = ((t2/4)%39) *256;
			if (t2/4 > 39*3) { x = ((t2/4)%39) *256; xx = 9984 - x; if (xx <=256) {y = t2+250; xx = 0; x=0; } }

		if (xx == 0) scrollerSetPos(&frontScroll, 0, x);
			else if (xx > 0) scrollerSetPos(&frontScroll, 0, xx);

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

	clearSprites(1, 512);
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

	clearFixLayer();
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
	backgroundColor(0x0000+t*3); //BG color

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
//		jobMeterColor(JOB_BLUE);

		//while((volMEMWORD(0x3c0006)>>7)!=0x120); //wait raster line 16
//		sortSprites(&drawTable[1],sortSize);
//		jobMeterColor(JOB_PINK);

		for (i = 0; i < 6*6; i++) {
			aSpriteSetPos(&sprites[i],(50+(sintab[i*15+t&1023]+sintab[i*5+t/2&1023]>>1)>>4)%15 * 21, i*5+(90+(sintab[i*8+(t+256)&1023]>>4)));
		}

		if(testPool.way==WAY_UP)
				spritePoolDrawList(&testPool,&drawTable[1]);
		else	spritePoolDrawList(&testPool,drawTablePtr);

		spritePoolClose(&testPool);

		t3 = DAT_frameCounter-startframe;

		millis = t3*(2); 
		t = millis;

		scrollerSetPos(&frontScroll, 0, 0);

		SCClose();


	}
}

/*
void pictureDemo() {
	int x=94+48;
	int y=54;
	picture testPict;
	int i,j;
	WORD raster=false;
	WORD tableShift=0;
	ushort rasterData0[512];
	ushort rasterData1[512];
	ushort rasterAddr;
	ushort *dataPtr;
	short displayedRasters;

	clearFixLayer();
	backgroundColor(0x7bbb); //BG color
	initGfx();
	jobMeterSetup(true);

	LSPCmode=0x1c00;
	loadTIirq(TI_MODE_SINGLE_DATA);

	pictureInit(&testPict, &terrypict,1, 16, x, y,FLIP_NONE);
	palJobPut(16,terrypict_Palettes.palCount,terrypict_Palettes.data);

	rasterAddr=0x8400+testPict.baseSprite;

	fixPrint(2,3,0,0,"1P \x12\x13\x10\x11: move picture");
	fixPrint(2,4,0,0,"1P ABCD: flip mode");
	fixPrint(2,5,0,0,"2P A: toggle rasters");
	
	SCClose();
	while(1) {
		waitVBlank();

		ps=volMEMBYTE(PS_CURRENT);
		p1=volMEMBYTE(P1_CURRENT);
		p2=volMEMBYTE(P2_CURRENT);
		p1e=volMEMBYTE(P1_EDGE);
		p2e=volMEMBYTE(P2_EDGE);
		
		if((ps&P1_START)&&(ps&P2_START)) {
			clearSprites(1, terrypict.tileWidth);
			TInextTable=0;
			SCClose();
			waitVBlank();
			unloadTIirq();
			return;
		}

		if(p1&JOY_UP)	y--;
		if(p1&JOY_DOWN)	y++;
		if(p1&JOY_LEFT)	x--;
		if(p1&JOY_RIGHT)	x++;
		
		if(p2e&JOY_A)	raster^=1;

		//fixPrintf(2,2,0,0,"%d",DAT_droppedFrames);
		while((volMEMWORD(0x3c0006)>>7)!=0x120); //wait raster line 16
		jobMeterColor(JOB_BLUE);

		if(p1e&JOY_A)	pictureSetFlip(&testPict,FLIP_NONE);
		if(p1e&JOY_B)	pictureSetFlip(&testPict,FLIP_X);
		if(p1e&JOY_C)	pictureSetFlip(&testPict,FLIP_Y);
		if(p1e&JOY_D)	pictureSetFlip(&testPict,FLIP_XY);

		pictureSetPos(&testPict, x, y);

		if(raster) {
			TInextTable=(TInextTable==rasterData0)?rasterData1:rasterData0;
			dataPtr=TInextTable;
			rasterAddr=0x8400+testPict.baseSprite;

			if(p2&JOY_B) for(i=0;i<50000;i++);	//induce frameskipping

			TIbase=TI_ZERO+(y>0?384*y:0); //timing to first line

			displayedRasters=(testPict.info->tileHeight<<4)-(y>=0?0:0-y);
			if(displayedRasters+y>224) displayedRasters=224-y;
			if(displayedRasters<0) displayedRasters=0;

			i=(y>=0)?0:0-y;
			for(j=0;j<displayedRasters;j++) {
				*dataPtr++=rasterAddr;
				if(!(j&0x1))
					*dataPtr++=(x+(sinTable[(i+tableShift)&0x3f]-32))<<7;
				else	*dataPtr++=(x-(sinTable[(i+1+tableShift)&0x3f]-32))<<7;
				i++;
			}
			SC234Put(rasterAddr,x<<7); //restore pos
			*dataPtr++=0x0000;
			*dataPtr++=0x0000;	//end
		} else {
			SC234Put(rasterAddr,x<<7); //restore position
			TInextTable=0;
		}

		tableShift++;
		jobMeterColor(JOB_GREEN);
		SCClose();
	}
}
*/

#define SCROLLSPEED 1.06
void rasterScrollDemo() {
	BYTE p1,ps;
	pictureInfo frontLayerInfo, backLayerInfo;
	picture frontLayer, backLayer;
	short posY=-192;
	ushort rasterData0[256],rasterData1[256];
	ushort *rasterData;
	float scrollAcc;
	int scrollPos[34];
	int scrollValues[34];
	ushort backAddr=0x8401;
	ushort frontAddr=0x8421;
	int x,y;
	short frontPosX[13],backPosX[13];
	ushort skipY;
	ushort firstLine;

	//layers were merged to save up tiles/palettes
	frontLayerInfo.colSize=tf4layers.colSize;
	backLayerInfo.colSize=tf4layers.colSize;
	frontLayerInfo.tileWidth=32;
	backLayerInfo.tileWidth=32;
	frontLayerInfo.tileHeight=tf4layers.tileHeight;
	backLayerInfo.tileHeight=tf4layers.tileHeight;
	//only using first map
	frontLayerInfo.maps[0]=tf4layers.maps[0];
	backLayerInfo.maps[0]=tf4layers.maps[0]+(tf4layers.colSize*32);

	clearFixLayer();
	initGfx();
	jobMeterSetup(true);
	loadTIirq(TI_MODE_DUAL_DATA);
	TInextTable=0;

	scrollValues[0]=1024;
	scrollPos[0]=0;
	scrollAcc=1024;
	for(x=1;x<34;x++) {
		scrollAcc*=SCROLLSPEED;
		scrollValues[x]=(int)(scrollAcc+0.5);
		scrollPos[x]=0;
	}

	pictureInit(&backLayer, &backLayerInfo,1,16,0,0,FLIP_NONE);
	pictureInit(&frontLayer, &frontLayerInfo,33,16,0,0,FLIP_NONE);
	palJobPut(16,tf4layers_Palettes.palCount,tf4layers_Palettes.data);

	backgroundColor(0x38db);
	fixPrint(0,1,0,0,"                                       ");
	fixPrint(0,30,0,0,"                                       ");
	SCClose();

	while(1) {
		waitVBlank();

		while((volMEMWORD(0x3c0006)>>7)!=0x120); //line 16
		jobMeterColor(JOB_BLUE);
		
		p1=volMEMBYTE(P1_CURRENT);
		ps=volMEMBYTE(PS_CURRENT);

		if((ps&P1_START)&&(ps&P2_START)) {
			clearSprites(1, 64);
			TInextTable=0;
			SCClose();
			waitVBlank();
			unloadTIirq();
			return;
		}
		if(p1&JOY_UP) if(posY<0) posY++;
		if(p1&JOY_DOWN) if(posY>-288) posY--;

		//update scroll values
		for(x=0;x<34;x++) scrollPos[x]+=scrollValues[x];
		frontPosX[0]=								(short)(0-(scrollPos[32]>>3));
		frontPosX[1]=frontPosX[2]=					(short)(0-(scrollPos[24]>>3));
		frontPosX[3]=frontPosX[4]=					(short)(0-(scrollPos[16]>>3));
		frontPosX[5]=								(short)(0-(scrollPos[8]>>3));
		frontPosX[6]=frontPosX[7]=frontPosX[8]=		(short)(0-(scrollPos[0]>>3));
		frontPosX[9]=frontPosX[10]=frontPosX[11]=	(short)(0-(scrollPos[1]>>3));
		frontPosX[12]=								(short)(0-(scrollPos[32]>>3));

		backPosX[0]=								(short)(0-(scrollPos[24]>>3));
		backPosX[1]=backPosX[2]=					(short)(0-(scrollPos[16]>>3));
		backPosX[3]=backPosX[4]=					(short)(0-(scrollPos[8]>>3));
		backPosX[5]=								(short)(0-(scrollPos[0]>>3));
		backPosX[6]=backPosX[7]=backPosX[8]=		(short)(0-(scrollPos[0]>>4));
		backPosX[9]=backPosX[10]=backPosX[11]=		(short)(0-(scrollPos[0]>>3));
		backPosX[12]=								(short)(0-(scrollPos[1]>>3));

		skipY=0-posY;
		x=skipY>>5;
		firstLine=32-(skipY&0x1f);

		TIbase=TI_ZERO+(384*firstLine); //timing to first raster line
		TInextTable=(TInextTable==rasterData0)?rasterData1:rasterData0;
		rasterData=TInextTable;

		pictureSetPos(&frontLayer,frontPosX[x]>>7,posY);
		pictureSetPos(&backLayer,backPosX[x]>>7,posY);
		//might need to force the update if base scroll position didn't change
		SC234Put(frontAddr,frontPosX[x]);
		SC234Put(backAddr,backPosX[x]);

		if(skipY<164) { //can we see water?
			TIreload=384*32;	//nope, 32px chunks
			for(x++;x<13;x++) {
				*rasterData++=frontAddr;
				*rasterData++=frontPosX[x];
				*rasterData++=backAddr;
				*rasterData++=backPosX[x];
				firstLine+=32;
				if(firstLine>=224) break;
			}
		} else {
			TIreload=384*4;		//yup, 4px chunks
			for(x++;x<12;x++) {
				for(y=0;y<8;y++) {
					*rasterData++=frontAddr;
					*rasterData++=frontPosX[x];
					*rasterData++=backAddr;
					*rasterData++=backPosX[x];
				}
				firstLine+=32;
			}
			x=1;
			while(firstLine<224) {
				*rasterData++=frontAddr;
				*rasterData++=frontPosX[12];
				*rasterData++=backAddr;
				*rasterData++=0-(scrollPos[x++]>>3);
				firstLine+=4;
			}
		}
		*rasterData++=0x0000;
		*rasterData++=0x0000;
		jobMeterColor(JOB_GREEN);
		SCClose();
	}
}

void tempTests() {
/*
	int x=0;
	int y=0;

	scroller scroll;

	backgroundColor(0x7bbb); //BG color
	clearFixLayer();
	initGfx();
	jobMeterSetup(true);

	scrollerInit(&scroll, &wohd, 1, 16, x, y);
	palJobPut(16, wohd_Palettes.palCount, wohd_Palettes.data);

	SCClose();
	while(1) {
		waitVBlank();

		p1=volMEMBYTE(P1_CURRENT);
		p1e=volMEMBYTE(P1_EDGE);
		ps=volMEMBYTE(PS_CURRENT);
		
		if((ps&P1_START)&&(ps&P2_START)) {
			clearSprites(1, 21);
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
		
		if(p1e&JOY_C) scrollerInit(&scroll, &wohd, 1, 16, x, y);
			else	scrollerSetPos(&scroll, x, y);
		
		jobMeterColor(JOB_GREEN);
		fixPrintf(2,4,0,0,"%04d\xff%04d\xff\xff",x,y);
		SCClose();
	}
*/
}



void startDemologic() {
	int i = 0;

	// call cdda, start playing track 2

	asm("clr.w %d0");
	asm("move.w #0x0502,%d0");
	asm("jsr 0xC0056A");

	demopart_letter();

	demopart_phone();

	demopart_sakura();


	demopart_meta();

	demopart_kiss();

	texto = 1;

	demopart_letter();

	demopart_sprite();


}


int main(void) {
		startDemologic();		
}
