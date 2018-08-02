#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <fstream>
#include <unistd.h> //used for launching programs and getting user name
#include <dirent.h> //used for getting file in folder lists
#include <sys/stat.h> //used for telling if a file is really a folder
#include "WidapStd.h"

extern double UI_SCALE;

class AnimValue //a value which can be animated
{
public:
	
	double pos0, pos1;
	double time0, time1;
	bool done; //if done, time should be greater then time1 and pos1 can be used
	
	AnimValue()
	{
		pos0=0;
		pos1=0;
		time0=0;
		time1=0;
		done=1;
	}
	
	AnimValue(double newPos)
	{
		pos0=newPos;
		pos1=newPos;
		time0=0;
		time1=0;
		done=1;
	}
	
	double getPos(double timeNow, int intrpMthd)
	{
		if (timeNow>=time1)
		{
			done=1;
			return pos1;
		}
		else if (timeNow<=time0)
			return pos0;
		else
		{
			if (intrpMthd==1) //linear interpolation
				return grdnt(timeNow, time0, time1, pos0, pos1);
			
			else if (intrpMthd==2) //cosine interpolation (smooth)
				return (cos((timeNow-time0)*PI/(time0-time1))*0.5-0.5)*(pos0-pos1)+pos0;
			
			else if (intrpMthd==3) //sine interpolation (smooth end)
				return (sin((timeNow-time0)*PI*0.5/(time0-time1)))*(pos0-pos1)+pos0;
			
			else //no interpolation
				return pos1;
		}
	}
	
	void setPos(double newPos1, double timeNow, double durration, int intrpMthd)
	{
		pos0=getPos(timeNow, intrpMthd);
		pos1=newPos1;
		time0=timeNow;
		time1=durration+timeNow;
		if (durration)
			done=1;
		else
			done=0;
	}
};

struct App //an application that can be launched
{
	int popScore; //this app's popularity score
	char name[256]; //the display name of the app
	char exec[256]; //this app's execution command
	App *nxtApp; //a pointer to the next app in the linked list
	App *nxtVsbl, *prvVsbl;
	AnimValue pos;
	AnimValue vsbl;
};

const int WDTH=640, HGHT=320;
const int FPS=30;
const double MOVE_ANIM_TIME=0.5, FADE_ANIM_TIME=0.3; //time for all animations
const char PROG_NAME[]="Widap Launcher";
const char FONT_PATH[]="/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
const char APP_LAUNCHER_PATH[]="/usr/share/applications";
const char APP_LIST_PATH[]="%s/.config/widap_launcher.data";
const char BKND_IMAGE_PATH[]="%s/.config/widap_launcher_bknd.png";
const int MAX_APPS=1000; //maximum number of apps
const int MAX_POP_SCORE=10000000; //the highest the popularity score can get
const int POP_SCORE_INC=50000; //what is added to the launched app
const double POP_SCORE_FACTOR=MAX_POP_SCORE/double(POP_SCORE_INC+MAX_POP_SCORE); //what each popScore is multiplied by each time
const int DSPLY_APP_NUM=4;

const sf::Color bkndClr0(0, 16, 32, 255);
const sf::Color bkndClr1(0, 64, 0, 255);
const sf::Color bkndClr2(255, 255, 255, 255);

extern App *frstApp;
extern int appNum;
extern App *slctApp; //currently selected app
extern char srchPtrn[256]; //what has been searched
extern AnimValue viewPos;
extern double currentTime;

extern sf::RenderWindow window;
extern sf::Texture bkndTexture;
extern sf::Sprite bkndSprite;
extern sf::Font font; //this font will be used for everything

//main.cpp
void processEvents();
void loadFiles(const char *foldername);
void loadAppFromLauncher(const char *filename);
void addApp(const char *name, const char *exec, int popScore);
void integrateLaunchers(); //integrate the apps list with freshly loaded launchers
void saveAppList();
bool loadAppList(); //returns 1 on success, 0 on failure
bool compareStr(const char *str0, const char *str1, bool matchCase=0); //returns 1 if the two are the same up to the length of the shortest one
bool srchStr(const char *str0, const char *str1, bool matchCase=0); //returns 1 if 2nd contains 1st
void cpyStrLine(char *str0, const char *str1); //copies until the end of str1 or a new line
void sortApps(); //sorts the apps list
bool compareAppsForSort(App* app0, App* app1); //used by the app sorting function
void changeSlctApp(int vctr); //move the selected app by vctr (usually 1 or -1)
void rfrshSrch();
void launchApp(App *app);

//graphics.cpp
void graphicsInit();
void makeBknd();
void display();
void drawSrchBox();
void drawAppIcon(App *app, double x, double y, double scale=1, double rot=0, double alpha=1);

