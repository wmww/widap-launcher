#include "WidapLauncher.h"

sf::RenderWindow window;
sf::Texture bkndTexture;
sf::Sprite bkndSprite;
sf::Font font; //this font will be used for everything

void nightBknd(sf::Color *bits);
void vortexList();
void prspctvList();

void blendRGBA(sf::Color *clr0, sf::Color clr1, double factor);

void graphicsInit()
{
	makeBknd();
    
	if(
		!font.loadFromFile("/usr/share/fonts/truetype/ubuntu-font-family/UbuntuMono-R.ttf") &&
		!font.loadFromFile("/usr/share/fonts/truetype/freefont/FreeMono.ttf") &&
		!font.loadFromFile("/usr/share/fonts/TTF/DejaVuSans.ttf")
	)
	{
		std::cout << "!! font not found !!" << std::endl;
	}
    
   // bkndSprite.setTexture(bkndTexture);
    
    //bkndSprite.setColor(sf::Color(32, 32, 32));
}

void makeBknd()
{
	/*//int x, y, yFactor, loc;
	sf::Color *bits;
	
	bits=new sf::Color[WDTH*HGHT*4];
	
	if (bits)
	{
		
		nightBknd(bits);
		
		bkndTexture.create(WDTH, HGHT);
		bkndTexture.update(&bits->r);
		
		delete[] bits;
	}*/
	
	//char filename[256];
	
	//sprintf(filename, BKND_IMAGE_PATH, getenv("HOME"));
	
	//if (!bkndTexture.loadFromFile(filename))
		//std::cout << "\n!! background not created !!\n";
}

/*void nightBknd(sf::Color *bits)
{
	int x, y, i, yFactor, loc;
	int left, right, top, btm;
	sf::Color groundClr(32, 64, 32, 255);
	sf::Color skyClr(0, 0, 32, 255);
	sf::Color moonClr(255, 255, 255, 255);
	sf::Color starClr(255, 255, 255, 255);
	
	double hrznStrt=0.8, hrznEnd=0.75;
	double moonX=0.85, moonY=0.15, moonRds=0.1, moonGlowRds=0.2;
	double dist;
	int starNum=600;
	
	for (y=0; y<HGHT; ++y)
	{
		yFactor=y*WDTH;
		
		for (x=0; x<WDTH; ++x)
		{
			loc=x+yFactor;
			
			bits[loc]=skyClr;
		}
	}
	
	moonX=grdnt(moonX, 0, 1, 0, WDTH);
	moonY=grdnt(moonY, 0, 1, 0, HGHT);
	moonRds=grdnt(moonRds, 0, 1, 0, HGHT);
	moonGlowRds=grdnt(moonGlowRds, 0, 1, 0, HGHT);
	
	left=clamp(moonX-moonGlowRds, 0, WDTH);
	right=clamp(moonX+moonGlowRds, 0, WDTH);
	btm=clamp(moonY+moonGlowRds, 0, HGHT);
	top=clamp(moonY-moonGlowRds, 0, HGHT);
	
	for (y=top; y<btm; ++y)
	{
		yFactor=y*WDTH;
		
		for (x=left; x<right; ++x)
		{
			loc=x+yFactor;
			
			dist=dst(x-moonX, y-moonY);
			
			if (dist<moonRds)
				bits[loc]=moonClr;
			else
				blendRGBA(bits+loc, moonClr, grdnt(dist, 0, moonGlowRds, 0.5, 0));
		}
	}
	
	for (i=0; i<starNum; ++i)
	{
		x=drand(0, WDTH);
		y=drand(0, hrznStrt*HGHT);
		
		blendRGBA(bits+y*WDTH+x, starClr, (double)i/starNum);
	}
	
	for (y=HGHT*hrznEnd; y<HGHT; ++y)
	{
		yFactor=y*WDTH;
		
		for (x=0; x<WDTH; ++x)
		{
			loc=x+yFactor;
			blendRGBA(bits+loc, groundClr, grdnt(y, hrznStrt*HGHT, hrznEnd*HGHT, 1, 0));
			//bits[loc+0]=sf::Color(0, 255, 255, 255);
		}
	}
}*/

void display()
{
	const double ANIM_FRAMS=12;
	static int counter=ANIM_FRAMS+3; //+3 makes it smoother when it comes out
	
	if (counter>0)
	{
		--counter;
		window.setPosition(sf::Vector2i(clamp(-(counter/ANIM_FRAMS)*(counter/ANIM_FRAMS)*WDTH, -WDTH, 0), sf::VideoMode::getDesktopMode().height-HGHT-28-2));
	}
	
	window.clear(sf::Color(4, 4, 4));
	
	//makeBknd();
	
	//window.draw(bkndSprite);
	
	drawSrchBox();
	
	prspctvList();
	
	window.display();
}

void prspctvList()
{
	App *ptr;
	double pos, vsbl, posStrt=0; //forward/backward position, 0 to 1 how visible it is, the position that should be closest fully in view to the camera
	
	const int before=0, after=20;
	
	ptr=frstApp;
	
	if (slctApp)
	{
		posStrt=viewPos.getPos(currentTime, 3);
	}
	
	while (ptr)
	{
		pos=ptr->pos.getPos(currentTime, 3)-posStrt;
		vsbl=ptr->vsbl.getPos(currentTime, 1);
		
		if (vsbl>0.0001 && pos>-1 && pos<DSPLY_APP_NUM+after)
		{
			drawAppIcon(ptr, 420-750/(pos+2), 20+(380+(1-vsbl)*(1-vsbl)*800)/(pos+2), 2.0/(pos+2), 0, vsbl*clamp(1-pos/(DSPLY_APP_NUM+before+after), 0, 1));
		}
		
		ptr=ptr->nxtApp;
	}
}

/*void vortexList()
{
	App *ptr;
	int pos=0;
	double ang, rds, xCntr, yCntr;
	
	xCntr=400;
	yCntr=100;
	
	ptr=slctApp;
	
	for (int i=0; i<viewPos && ptr; ++i)
		ptr=ptr->prvVsbl;
	
	if (ptr)
	{
		do
		{
			ang=pos*180/6;
			rds=320/(pos+8);
			
			drawAppIcon(ptr, sin(deg2rad(ang))*rds+xCntr, cos(deg2rad(ang))*rds+yCntr, 0.5, -ang);
			++pos;
		} while (pos<DSPLY_APP_NUM && (ptr=ptr->nxtVsbl));
	}
}*/

void drawSrchBox()
{
	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(48);
	
	if (srchPtrn[0])
	{
		text.setString(srchPtrn);
		text.setColor(sf::Color(0, 255, 255, 192));
	}
	else
	{
		text.setString("[Search]");
		text.setColor(sf::Color(255, 255, 255, 64));
	}
	
	text.setPosition(20, 10);
	
	//text.setScale(.50, .50);
	window.draw(text);
}

void drawAppIcon(App *app, double x, double y, double scale, double rot, double alpha)
{
	//sf::Color bkndClr(0, 0, 0, 128);
	sf::Color txtClr(0, 255, 128, 128*alpha);
	sf::Color slctClr(0, 255, 0, 255*alpha);
	
	double xTxt=x, yTxt=y;
	int sizeTxt=52;
	
	/*sf::CircleShape bullet;
	bullet.setRadius(sizeTxt/2);
	
	if (app==slctApp)
		bullet.setFillColor(sf::Color::Cyan);
	else
		bullet.setFillColor(sf::Color::Green);
	
	//bullet.setOutlineColor(sf::Color::Cyan);
	//bullet.setOutlineThickness(4);
	bullet.setPosition(xTxt, yTxt+sizeTxt/4);
	window.draw(bullet);*/
	
	sf::Text text;
	text.setFont(font);
	if (app==slctApp)
		text.setColor(slctClr);
	else
		text.setColor(txtClr);
	text.setCharacterSize(sizeTxt);
	text.setString(app->name);
	text.setPosition(xTxt+sizeTxt/4, yTxt);
	text.setScale(scale, scale);
	text.setRotation(rot);
	
	/*sf::FloatRect textRect = text.getGlobalBounds();
	sf::RectangleShape bkndRect(sf::Vector2f(textRect.width, textRect.height));
	bkndRect.setPosition(textRect.left, textRect.top);
	bkndRect.setFillColor(bkndClr);
	
	window.draw(bkndRect);*/
	window.draw(text);
}

void blendRGBA(sf::Color *clr0, sf::Color clr1, double factor)
{
	if (factor<0)
		return;
	else if (factor>1)
		*clr0=clr1;
	else
	{
		clr0->r=grdnt(factor, 0, 1, clr0->r, clr1.r);
		clr0->g=grdnt(factor, 0, 1, clr0->g, clr1.g);
		clr0->b=grdnt(factor, 0, 1, clr0->b, clr1.b);
		clr0->a=grdnt(factor, 0, 1, clr0->a, clr1.a);
	}
}

