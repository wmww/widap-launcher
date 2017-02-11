#include "WidapLauncher.h"

App *frstApp=0;
App *frstVsbl=0;
int appNum=0;
App *slctApp=0; //currently selected app
char srchPtrn[256]; //what has been searched
double currentTime=0;
AnimValue viewPos(0);

sf::Clock myClock;
sf::Time frameTime=sf::seconds(1.0/FPS);

int main()
{
	window.setPosition(sf::Vector2i(0, 0));
	
	window.create(sf::VideoMode(WDTH, HGHT), PROG_NAME, sf::Style::None); //must be first to start capturing events immediately
	
	display();
	
	myClock.restart();
	
	graphicsInit();
	
	srchPtrn[0]=0;
	
	std::cout << "\nloading app list...\n";
	if (!loadAppList())
		loadFiles(APP_LAUNCHER_PATH);
	
	std::cout << std::endl << "loaded " << appNum << " apps." << std::endl;
	
	std::cout << std::endl;
	
	changeSlctApp(0);
	
	std::cout << std::endl << "integrating launchers..." << std::endl;
	integrateLaunchers();
	
	std::cout << std::endl << "sorting apps..." << std::endl;
	sortApps();
	
	std::cout << std::endl;
	
	changeSlctApp(0);
	
	display();
	
    while (window.isOpen())
    {
        processEvents();
        
        display();
        
        //cut down on excessive CPU
		if (myClock.getElapsedTime()<frameTime)
			sf::sleep(frameTime-myClock.getElapsedTime());
		
		myClock.restart();
		
		currentTime+=1.0/FPS;
	}
    
    std::cout << std::endl << "saving apps list..." << std::endl;
	saveAppList();
	std::cout << std::endl << "all done." << std::endl;
    
    return EXIT_SUCCESS;
}

void processEvents()
{
	///event polling
	
	sf::Event event;
	while (window.pollEvent(event))
	{
		switch (event.type)
		{
		// window closed
		case sf::Event::Closed:
			window.close();
			break;
		
		case sf::Event::TextEntered:
			if ((char)event.text.unicode<=126 && (char)event.text.unicode>=32)
			{
				int i=strlen(srchPtrn);
				srchPtrn[i]=(char)event.text.unicode;
				srchPtrn[i+1]=0;
				
				rfrshSrch();
			}
			break;
				
		// key pressed
		case sf::Event::KeyPressed:
			
			{
				switch (event.key.code)
				{
				case sf::Keyboard::BackSpace:
					if (srchPtrn[0])
					{
						srchPtrn[strlen(srchPtrn)-1]=0;
						rfrshSrch();
					}
					break;
					
				case sf::Keyboard::Down:
					changeSlctApp(-1);
					break;
					
				case sf::Keyboard::Up:
					changeSlctApp(1);
					break;
					
				case sf::Keyboard::Return:
					launchApp(slctApp);
					break;
					
				case sf::Keyboard::Tab:
					if (slctApp)
					{
						strcpy(srchPtrn, slctApp->exec);
						strcat(srchPtrn, " ");
						rfrshSrch();
					}
					break;
					
				case sf::Keyboard::Escape:
					window.close();
					break;
					
				default:
					break;
				}
			}

		// we don't process other types of events
		default:
			break;
		}
	}
}

void loadFiles(const char *foldername) //loads all the launchers
{
	DIR *dp;
	struct dirent *dirp;
	struct stat filestat;
	char filename[256];
	char path[256];
	
	if(!(dp  = opendir(foldername))) {
		std::cout << "Error opening applications directory" << std::endl;
		return;
	}
	
	while ((dirp = readdir(dp))) {
		
		strcpy(filename, dirp->d_name);
		
		//Skip current object if it is this directory or parent directory
		if(!compareStr(".", filename))
		{
			sprintf(path, "%s/%s", foldername, filename);
			
			//get stats on file/folder
			if (stat(path, &filestat))
			{
				std::cout << "!! error getting stats !!" << std::endl;
				return;
			}
			
			//Recursively call this function if current object is a directory
			if(S_ISDIR(filestat.st_mode))
				loadFiles(path);
			else if (srchStr(".desktop", filename)) //else, try to load it as a launcher
				loadAppFromLauncher(path);
		}
	}
	
	closedir(dp);
}

void loadAppFromLauncher(const char *filename)
{
	std::ifstream fileStream; //the stream to read the file
	char *file=0; //will be set to an array fileLng int which will contain the entire file
	int fileLng; //length of the file
	int i=0, j;
	bool correctSection=0; //true only when we are in the [Desktop Entry] section of the file
	bool foundName=0, foundExec=0; //if we have found these components within the launcher
	char name[256], exec[256];
	
	//std::cout << std::endl << "trying to load \"" << filename << "\"" << std::endl; 
	
	//open a stream for the file
	fileStream.open(filename, std::ifstream::binary);
	
	if (appNum>=MAX_APPS)
	{
		std::cout << "!! too many apps" << std::endl;
		fileStream.close();
		return;
	}
	
	if (!fileStream.is_open()) //file did not load
	{
		std::cout << "!! failed to load \"" << filename << "\" launcher !!" << std::endl;
		return;
	}
	
	//find the length of the file, then go back to the beginning
	fileStream.seekg (0, fileStream.end);
	fileLng = fileStream.tellg();
	fileStream.seekg (0, fileStream.beg);
	
	if (!(fileLng>0 && fileLng<1000000)) //if file length is not right (some error must have occurred)
	{
		std::cout << std::endl << "!! file length of \"" << filename << "\" is " << fileLng << ". this is a problem !!" << std::endl;
		fileStream.close();
		return;
	}
	
	file = new char[fileLng];
	
	if (!file) //if there has been a problem allotting the memory needed to load the file
	{
		std::cout << std::endl << "!! dynamic memory failed to initialize for file !!" << std::endl;
		fileStream.close();
		return;
	}
	
	//load the file
	fileStream.read (file, fileLng);
	
	if (!fileStream) //if an error has occurred (some of these error checks may be redundant but I do not yet completely understand fstream so they stay
	{
		std::cout << std::endl << "!! launcher loading error: only " << fileStream.gcount() << " bytes could be read !!" << std::endl;
		delete[] file;
		return;
	}
	
	//close the file
	fileStream.close();
	
	//loop through the file looking for new lines
	while (i<fileLng)
	{
		if (correctSection)	//if we are in the desktop entry section of the file
		{
			if (compareStr("Name=", file+i)) //we check to see if this line is specifying the app name
			{
				if (foundName)
					std::cout << std::endl << "?? found multiple app names in \"" << filename << "\" ??" << std::endl;
				
				cpyStrLine(name, file+i+5); //the +5 is to skip over the "Name="; it would be better if it were not hard coded
				
				foundName=1;
			}
			
			else if (compareStr("Exec=", file+i)) //check for execution command
			{
				if (foundExec)
					std::cout << std::endl << "?? found multiple app exec commands in \"" << filename << "\" ??" << std::endl;
				
				cpyStrLine(exec, file+i+5); //again, +5 is to skip "Exec="
				
				for (j=0; (exec[j]!=' ' || exec[j+1]!='%') && exec[j]!=0; ++j) {} //stop those %U things
				exec[j]=0; //this will do nothing if there was no % because j will be at the last point in the string
				
				foundExec=1;
			}
		}
		
		if (file[i]=='[') //deferent launcher sections start with the section name in brackets, we only use the [Desktop Entry] section
		{
			if (compareStr("[Desktop Entry]", file+i))
				correctSection=1;
			else
				correctSection=0;
		}
		
		do //loop until i is one past a new line
		{
			++i;
		} while (i<fileLng && file[i-1]!=10);
	}
	
	if (!foundExec) //if no command was found
	{
		//std::cout << std::endl << "!! no execution command was found in \"" << filename << "\" !!" << std::endl;
		delete[] file;
		return;
	}
	
	if (!foundName) //if no name was provided, copy the command
	{
		std::cout << std::endl << "?? no name found in \"" << filename << "\". substituting command ??" << std::endl;
		sprintf(name, "[%s]", exec);
	}
	
	//display app data
	/*std::cout << std::endl <<
		"Loaded \"" << filename << "\":"<< std::endl <<
		"	name: " << apps[appNum].name << std::endl <<
		"	command: " << apps[appNum].exec << std::endl;
	*/
	
	delete[] file; //clear dynamic memory (this must be done if exiting early for an error)
	
	addApp(name, exec, 0);
}

void addApp(const char *name, const char *exec, int popScore)
{
	App *ptr=new App;
	
	strcpy(ptr->name, name);
	strcpy(ptr->exec, exec);
	ptr->popScore=popScore;
	ptr->pos.setPos(20, currentTime, 0, 0);
	ptr->vsbl.setPos(0, currentTime, 0, 0);
	ptr->vsbl.setPos(1, currentTime, MOVE_ANIM_TIME, 0);
	
	ptr->nxtApp=frstApp;
	frstApp=ptr;
	
	ptr->nxtVsbl=frstVsbl;
	ptr->prvVsbl=0;
	if (frstVsbl)
		frstVsbl->prvVsbl=ptr;
	frstVsbl=ptr;
	
	/*if (frstVsbl)
	{
		ptr->nxtVsbl=frstVsbl;
		ptr->prvVsbl=frstVsbl->prvVsbl;
		frstVsbl->prvVsbl->nxtVsbl=ptr;
		frstVsbl->prvVsbl=ptr;
		frstVsbl=ptr;
	}
	else
	{
		ptr->prvVsbl=ptr;
		ptr->nxtVsbl=ptr;
		frstVsbl=ptr;
	}*/
	
	++appNum;
}

void integrateLaunchers() //delete all the apps in the list
{
	App *ptr0, *ptr1, *oldStrt;
	
	oldStrt=frstApp;
	
	loadFiles(APP_LAUNCHER_PATH);
	
	ptr0=frstApp;
	
	appNum=0;
	
	while (ptr0)
	{
		ptr1=oldStrt;
		
		while (ptr1)
		{
			if (!strcmp(ptr0->exec, ptr1->exec))
			{
				ptr0->popScore=ptr1->popScore;
			}
			
			ptr1=ptr1->nxtApp;
		}
		
		if (ptr0->nxtApp==oldStrt)
			ptr0->nxtApp=0;
		
		++appNum;
		
		ptr0=ptr0->nxtApp;
	}
	
	ptr0=oldStrt;
	
	while (ptr0)
	{
		ptr1=ptr0;
		ptr0=ptr0->nxtApp;
		delete ptr1;
	}
}

void saveAppList()
{
	char *ary;
	App *ptr;
	int i=0, j;
	std::ofstream file;
	
	ary=new char[appNum*3*256];
	
	strcpy(ary, "this is an automatically generated file for Widap Launcher, DO NOT EDIT!\n\n");
	
	i=strlen(ary);
	
	ptr=frstApp;
	
	while (ptr)
	{
		ptr->popScore*=POP_SCORE_FACTOR;
		
		j=0; while ((ary[i+j]=ptr->name[j])) {++j;} i+=j+1; ary[i-1]=10;
		j=0; while ((ary[i+j]=ptr->exec[j])) {++j;} i+=j+1; ary[i-1]=10;
		ary[i]=((ptr->popScore/10000000)%10)+48; ++i;
		ary[i]=((ptr->popScore/1000000)%10)+48; ++i;
		ary[i]=((ptr->popScore/100000)%10)+48; ++i;
		ary[i]=((ptr->popScore/10000)%10)+48; ++i;
		ary[i]=((ptr->popScore/1000)%10)+48; ++i;
		ary[i]=((ptr->popScore/100)%10)+48; ++i;
		ary[i]=((ptr->popScore/10)%10)+48; ++i;
		ary[i]=((ptr->popScore/1)%10)+48; ++i;
		ary[i]=10; ++i;
		
		ptr=ptr->nxtApp;
	}
	
	ary[i]=0;
	char filename[256];
	
	sprintf(filename, APP_LIST_PATH, getenv("HOME"));
	
	file.open(filename);
	
	if (file.is_open())
	{
		file << ary;
		file.close();
	}
	else
	{
		std::cout << "\n!! error saving data file !!\n";
	}
	delete[] ary;
}

bool loadAppList()
{
	std::ifstream fileStream; //the stream to read the file
	char *file;
	int fileLng;
	int i;
	int j;
	char name[256], exec[256];
	int popScore;
	char filename[256];
	
	sprintf(filename, APP_LIST_PATH, getenv("HOME"));
	fileStream.open(filename, std::ifstream::binary);
	
	if (!fileStream.is_open()) //file did not load
	{
		std::cout << "\napp list not found, importing launchers...\n";
		return 0;
	}
	
	//find the length of the file, then go back to the beginning
	fileStream.seekg (0, fileStream.end);
	fileLng = fileStream.tellg();
	fileStream.seekg (0, fileStream.beg);
	
	if (!(fileLng>0 && fileLng<10000000)) //if file length is not right (some error must have occurred)
	{
		std::cout << std::endl << "!! file length of apps list is " << fileLng << ". this is a problem !!" << std::endl;
		fileStream.close();
		return 0;
	}
	
	file = new char[fileLng];
	
	if (!file) //if there has been a problem allotting the memory needed to load the file
	{
		std::cout << std::endl << "!! dynamic memory failed to initialize for file !!" << std::endl;
		fileStream.close();
		return 0;
	}
	
	//load the file
	fileStream.read (file, fileLng);
	
	if (!fileStream) //if an error has occurred (some of these error checks may be redundant but I do not yet completely understand fstream so they stay
	{
		std::cout << std::endl << "!! launcher loading error: only " << fileStream.gcount() << " bytes could be read !!" << std::endl;
		delete[] file;
		return 0;
	}
	
	//close the file
	fileStream.close();
	
	i=2;
	
	while (i<fileLng && (file[i-2]!=10 || file[i-1]!=10)) {++i;}
	
	while (i<fileLng)
	{
		popScore=0;
		
		j=0; while (i+j<fileLng && (name[j]=file[i+j])!=10) {++j;} name[j]=0; i+=j+1;
		j=0; while (i+j<fileLng && (exec[j]=file[i+j])!=10) {++j;} exec[j]=0; i+=j+1;
		popScore+=(file[i]-48)*10000000; ++i;
		popScore+=(file[i]-48)*1000000; ++i;
		popScore+=(file[i]-48)*100000; ++i;
		popScore+=(file[i]-48)*10000; ++i;
		popScore+=(file[i]-48)*1000; ++i;
		popScore+=(file[i]-48)*100; ++i;
		popScore+=(file[i]-48)*10; ++i;
		popScore+=(file[i]-48)*1; ++i; ++i;
		
		addApp(name, exec, popScore);
	}
	
	delete[] file;
	
	return 1;
}

bool compareStr(const char *str0, const char *str1, bool matchCase) //returns 1 if the the 2nd starts with the 1st
{
	int i=0;
	int lng=0;
	bool same=1;
	char chr0, chr1;
	
	do
	{
		if (!str1[lng])
			return 0;
		
		++lng;
		
	} while (str0[lng]);
	
	while (same && i<lng)
	{
		chr0=str0[i];
		chr1=str1[i];
		
		if (!matchCase && chr0<=90 && chr0>=65)
			chr0+=32;
		
		if (!matchCase && chr1<=90 && chr1>=65)
			chr1+=32;
		
		if (chr0!=chr1)
			same=0;
		
		++i;
	}
	
	return same;
}

bool srchStr(const char *str0, const char *str1, bool matchCase) //returns 1 if 2nd contains 1st
{
	int pos1=0, pos0;
	char chr0, chr1;
	
	while (str1[pos1])
	{
		pos0=0;
		
		do
		{
			chr0=str0[pos0];
			chr1=str1[pos1+pos0];
			
			if (!matchCase && chr0<=90 && chr0>=65)
				chr0+=32;
			
			if (!matchCase && chr1<=90 && chr1>=65)
				chr1+=32;
			
			if (!chr0)
				return 1;
			
			++pos0;
			
		} while (chr0==chr1 && chr1);
		
		++pos1;
	}
	
	return 0;
}

void cpyStrLine(char *str0, const char *str1) //copies until the end of str1 or a new line
{
	for (int i=0; (str0[i]=(str1[i]*(abs(str1[i]*2-10)!=10))); ++i) {}
}

void sortApps() //sorts the apps list
{
	App **ary;
	App *ptr;
	int i;
	
	ary=new App*[appNum];
	ptr=frstApp;
	
	for (i=appNum-1; i>=0; --i)
	{
		ary[i]=ptr;
		ptr=ptr->nxtApp;
	}
	
	std::sort(ary, ary+appNum, compareAppsForSort);
	
	for (i=0; i<appNum-1; ++i)
	{
		ary[i]->nxtApp=ary[i+1];
	}
	
	frstApp=ary[0];
	ary[appNum-1]->nxtApp=0;
	
	delete[] ary;
	
	slctApp=0;
	
	rfrshSrch();
}

bool compareAppsForSort(App* app0, App* app1) //used by the app sorting function
{
	int i;
	char chr0, chr1;
	
	//fist, sort based on popularity score
	if (app0->popScore>app1->popScore)
		return 1;
	else if (app0->popScore<app1->popScore)
		return 0;
	
	//if pop score is the same, alphabetize
	
	i=0;
	
	while (app0->name[i] && app1->name[i])
	{
		chr0=app0->name[i];
		chr1=app1->name[i];
		
		if (chr0<=90 && chr0>=65)
			chr0+=32;
		
		if (chr1<=90 && chr1>=65)
			chr1+=32;
		
		if (chr0<chr1)
			return 1;
		else if (chr0>chr1)
			return 0;
		
		++i;
	}
	
	//if one string ends still the same, sort based on size
	if (!app0->name[i])
		return 1;
	else
		return 0;
}

void changeSlctApp(int vctr) //move the selected app by vctr (usually 1 or -1)
{
	int i;
	
	if (slctApp)
	{
		for (i=0; i<vctr && slctApp->nxtVsbl; ++i)
		{
			slctApp=slctApp->nxtVsbl;
			/*viewPos++;
			if (viewPos>=DSPLY_APP_NUM)
				viewPos=DSPLY_APP_NUM-1;*/
		}
		
		for (i=0; i>vctr && slctApp->prvVsbl; --i)
		{
			slctApp=slctApp->prvVsbl;
			/*viewPos--;
			if (viewPos<0)
				viewPos=0;*/
		}
		
		std::cout << "selected " << slctApp->name << std::endl;
		
		if (slctApp->pos.pos1<viewPos.pos1-0.5)
			viewPos.setPos(slctApp->pos.pos1, currentTime, MOVE_ANIM_TIME, 3);
		else if (slctApp->pos.pos1>viewPos.pos1+DSPLY_APP_NUM-1.5)
			viewPos.setPos(slctApp->pos.pos1-DSPLY_APP_NUM+1, currentTime, MOVE_ANIM_TIME, 3);
	}
	else
	{
		slctApp=frstVsbl;
		viewPos=0;
		
		viewPos.setPos(0, currentTime, 0, 3);
	}
}

void rfrshSrch()
{
	App *ptr, *prv=0;
	
	frstVsbl=0;
	
	int pos=0;
	
	ptr=frstApp;
	
	while (ptr)
	{
		if (!srchPtrn[0]
			|| compareStr(srchPtrn, ptr->name)
			//|| compareStr(srchPtrn, ptr->exec)
			)
		{
			ptr->prvVsbl=prv;
			
			if (prv)
				prv->nxtVsbl=ptr;
			else
			{
				frstVsbl=ptr;
			}
			
			if (ptr->vsbl.pos1<0.5)
			{
				ptr->vsbl.setPos(1, currentTime, MOVE_ANIM_TIME, 1);
			}
			
			ptr->pos.setPos(pos, currentTime, MOVE_ANIM_TIME, 3);
			
			++pos;
			
			prv=ptr;
		}
		else
		{
			if (ptr->vsbl.pos1>0.5)
			{
				ptr->vsbl.setPos(0, currentTime, FADE_ANIM_TIME, 3);
			}
		}
		
		ptr=ptr->nxtApp;
	}
	
	ptr=frstApp;
	
	while (srchPtrn[0] && ptr)
	{
		if ((ptr->vsbl.pos1<0.5) && (srchStr(srchPtrn, ptr->name) || compareStr(srchPtrn, ptr->exec)))
		{
			ptr->prvVsbl=prv;
			
			if (prv)
				prv->nxtVsbl=ptr;
			else
			{
				frstVsbl=ptr;
			}
			
			ptr->vsbl.setPos(1, currentTime, FADE_ANIM_TIME, 3);
			
			ptr->pos.setPos(pos, currentTime, MOVE_ANIM_TIME, 3);
			
			++pos;
			
			prv=ptr;
		}
		
		ptr=ptr->nxtApp;
	}
	
	if (prv)
		prv->nxtVsbl=0;
	
	//if (slctApp && slctApp->vsbl.pos1>0.5)
	//	viewPos.setPos(slctApp->pos.pos1, currentTime, ANIM_TIME);
	//else
	{
		slctApp=frstVsbl;
		viewPos.setPos(0, currentTime, MOVE_ANIM_TIME, 3);
	}
}

void launchApp(App *app)
{
	char exec[256];
	
	if (app) //run the app
	{
		app->popScore+=POP_SCORE_INC;
		
		std::cout << std::endl << "Launching " << app->name << " (\"" << app->exec << "\")" << std::endl;
		
		strcpy(exec, "setsid ");
		strcat(exec, app->exec);
		strcat(exec, " &"); //makes the system call non blocking
	}
	else //call the search pattern as a command
	{
		std::cout << std::endl << "running \"" << srchPtrn << "\"" << std::endl;
		
		strcpy(exec, "setsid ");
		strcat(exec, srchPtrn);
		strcat(exec, " &"); //makes the system call non blocking
	}
	
	if (!std::system(exec))
		window.close();
	else
		std::cout << "!! system call error !!" << std::endl;
}

