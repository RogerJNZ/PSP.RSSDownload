/***************************************************************************************
// Program : 	DownloadLinks
// DateTime: 	Feb 2009
// Author: 		R Johnston
// Description: 	Download any type of file using HTTP. Also convert RSS files into HTML(ish) so that
//			a standard web browser can be used to read them.
//			
//			So what's it for? I often travel overseas and have limited access to a wifi hotspot.
//			Generally there are free hotspots in cafes but it is often not convenient to try and surf all
//			my favourite webpages while ignoring the company I am with (especially if they are paying ;-). 
//			Using download links I can quickly download my favourite rss feeds or even html pages 
// 			for reading later at my leisure. This is also handy when stuck in traffic in the mornings 
//			as it allows me to catchup on the news anywhere.
//
//			Requires:
//			* Feeds.txt - List of files to be downloaded. Format is <name>[space]<url>
//				The name is used as the filename and also as a reference to the file
// 				in the index.html page of links and also in the header and footer of each page
//	 		* config.ini - Optional, allows overwriting default configuration values. 
//
// Installation:	1. Copy Downloadlinks directory to /PSP/GAME
//                            2. Add the files that you want to download to feeds.txt. Filles are added in the following format:
//				[name][space][url]
//                                Note that [name] is generally just used as the name of the file but if it contains .html or .rss then the following
//			    additional processing is performed:
//                                a) .html	a header and footer is added to the file
//			    b) .rss	the xml file downloaded is converted into HTMLish (good enough to view in your favourite browser).
//			    c) all other files are simply downloaded
//                                All files downloaded are added to index.html for easy viewing
//			3. Run your favoutire web browser (Links2, Netfront Internet Browser by P86 or the standard PSP Browser)
//                                Open /PSP/COMMON/INDEX.HTML amd add it as a bookmark for easy retrieval everytime
//
// Known Issues:	1. Downloadlinks has been compiled with curl 7.16 (http://curl.haxx.se/libcurl/c/libcurl-easy.html) 
//			and as a result the older style configuration for username and passwords i.e. username:password 
//			format is used instead of separate variables in the newer version
//
// 25 Mar 2010	RJ	Changed background to original. Added icon struct. Fix page up bug
'***************************************************************************************/
#include <pspkernel.h>
#include <pspsdk.h>
#include <time.h>
#include <oslib/oslib.h>
#include <math.h>

#include <sys/select.h>
#include <curl/curl.h>
#include "fileOperation.h"
#include "network.h"
#include "media.h"

PSP_MODULE_INFO("DownloadLinks", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(12*1024);

#define ANALOG_SENS 80
#define VERSION "4.3.0"


/* Globals: */
struct homebrew HBlist[MAX_HB];
OSL_IMAGE *bkg,*startb, *selectb, *cross,*circle, *R, *L, *imgRSS, *imgHTML, *imgUnselect, *imgSquare, *imgSelect;
OSL_FONT *pgfFont;

struct Icon {
	OSL_IMAGE *Image;
	int Height;   
	int Width; 
};



/*  Main menu: */
int mainMenu() {
	CURL *curl = NULL;
	int skip = 0;
    int start = 26;
    int first = 0;
    int total = 0;
	char hbfound[20];
    int visible = 13;
    int selected = 0;
    int i = 0, timer = 0;
    int MoveLinkFlag = 0;
	int HBDownloading = 0;  // 1 if currently download 0 otherwise
	int HBListChanged = 0; // Don't save list of links if the user didn't make any changes
	int HBSelectAll = 1; // flag if currently selecting to download all
	int HBType = 0;
	
	struct Icon HBIcon[TYPE_OTHER];
	HBIcon[TYPE_OTHER].Image = imgUnselect;
	HBIcon[TYPE_OTHER].Height = oslGetImageHeight(imgUnselect) + 1;
	HBIcon[TYPE_OTHER].Width = oslGetImageWidth(imgUnselect);
	HBIcon[TYPE_RSS].Image = imgRSS;
	HBIcon[TYPE_RSS].Height = oslGetImageHeight(imgRSS) + 2;
	HBIcon[TYPE_RSS].Width = oslGetImageWidth(imgRSS);
	HBIcon[TYPE_HTML].Image = imgRSS;
	HBIcon[TYPE_HTML].Height = oslGetImageHeight(imgHTML) + 2;
	HBIcon[TYPE_HTML].Width = oslGetImageWidth(imgHTML);

	total = getHBlist(HBlist);
	if (total <	1) strcpy(ERRORDESC, "E001 No links found in feeds.txt");
	strcpy(HBlist[0].error, "GUI and code 'borrowed' from Sakya's Homebrew Sorter. Thanks Sakya!");
		
    while (!osl_quit){
        if(!skip){
			oslStartDrawing();
            oslDrawImageXY(bkg, 0, 0);
			// Top Toolbar
		    //oslDrawFillRect(0,0,480,272,RGBA(0,0,0,170)); // Top Bar
		    //
			oslDrawFillRect(0,0,480,15,RGBA(0,0,0,170)); // Top Bar
		    oslDrawString(5,0,"DownloadLinks by RogerJ");
			oslIntraFontSetStyle(pgfFont, 0.5, RGBA(255,255,255,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_RIGHT);
			sprintf(hbfound,"Links found: %i", total);
			oslDrawString(470,0, hbfound);
			
			// Key Help
            oslDrawFillRect(290,22,475,113,RGBA(0,0,0,170)); 
			oslIntraFontSetStyle(pgfFont, 0.5, RGBA(255,255,255,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_LEFT);
			oslDrawString(350,22,"Modify List");
			oslDrawString(350,183,"Download Files");
			if (HBDownloading) 
				oslIntraFontSetStyle(pgfFont, 0.5, RGBA(255,255,255,100), RGBA(0,0,0,0), INTRAFONT_ALIGN_LEFT);
			oslDrawImageXY(cross,360,41);
			if (MoveLinkFlag) {
				oslDrawString(390,42,"Release Link");
			} else {
				oslDrawString(390,42,"Move Link");
			}
			oslDrawImageXY(L,359,62);
			oslDrawString(390,62,"Page Up");
			oslDrawImageXY(R,359,82);
			oslDrawString(390,82,"Page Down");
			
			oslDrawImageXY(imgSquare,360,102);
			if (HBlist[selected].download) {
				oslDrawString(390,102,"Skip Link");
			} else {
				oslDrawString(390,102,"Download Link");
			}
			oslDrawImageXY(imgSelect,350,121);
			if (HBSelectAll) {
				oslDrawString(390,122,"Skip All Links");
			} else {
				oslDrawString(390,122,"Download All");
			}
			
			oslDrawImageXY(startb,350,202);
			oslDrawString(390,203,"Start Download");
			oslDrawImageXY(circle,360,223);
			if (!HBDownloading) 
				oslIntraFontSetStyle(pgfFont, 0.5, RGBA(255,255,255,170), RGBA(0,0,0,0), INTRAFONT_ALIGN_LEFT);
			else
				oslIntraFontSetStyle(pgfFont, 0.5, RGBA(255,255,255,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_LEFT);
			oslDrawString(390,223,"Cancel Download");
		
            oslDrawFillRect(5,22,340,238,RGBA(68,162,231,170)); // File List Background
			oslDrawFillRect(0,254,480,272,RGBA(0,0,0,170)); // Bottom Bar

            //Draw list
            for (i = first; i <= first + visible; i++) {
                if (i < total) {
					if (!HBlist[i].download) {
						HBType = TYPE_OTHER;
						oslDrawImageXY(HBIcon[TYPE_HTML].Image, 12, start + (i - first) * HBIcon[TYPE_HTML].Height); // Workaround for bug in osLIB which results in an image being displayed only if it is different from the previous image...
					} else { 
						HBType = HBlist[i].type;
						oslDrawImageXY(HBIcon[TYPE_OTHER].Image, 12, start + (i - first) * HBIcon[TYPE_OTHER].Height); // Workaround for bug in osLIB which results in an image being displayed only if it is different from the previous image...
					}
					oslDrawImageXY(HBIcon[HBType].Image, 12, start + (i - first) * HBIcon[HBType].Height);
	                if (i == selected) {
	                    oslIntraFontSetStyle(pgfFont, 0.5, RGBA(20,20,20,255), RGBA(255,255,255,200), INTRAFONT_ALIGN_LEFT);
	                } else {
	                    oslIntraFontSetStyle(pgfFont, 0.5, RGBA(255,255,255,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_LEFT);	                    
	                }
					oslSetFont(pgfFont);
					oslDrawString(15 + HBIcon[HBType].Width,start +(i - first)*HBIcon[HBType].Height, HBlist[i].name);
					oslIntraFontSetStyle(pgfFont, 0.5, RGBA(255,255,255,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_RIGHT);
					oslDrawString(280, start + (i - first)*HBIcon[HBType].Height, HBlist[i].size);
					oslIntraFontSetStyle(pgfFont, 0.5, RGBA(255,255,255,100), RGBA(0,0,0,0), INTRAFONT_ALIGN_RIGHT);
					oslDrawString(335, start + (i - first)*HBIcon[HBType].Height, HBlist[i].speed);
                }
            }
			oslIntraFontSetStyle(pgfFont, 0.5, RGBA(255,255,255,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_LEFT);
			if (HBDownloading == 1) 
				oslDrawString(5, 240, ERRORDESC); // Maintain the error on the screen even if processing the next file so that the user has time to read it
			else
				oslDrawString(5, 240, HBlist[selected].error);
			oslDrawString(5, 256, HBlist[selected].path);
            oslEndDrawing();
        }
        oslEndFrame();
		skip = oslSyncFrame();  // oslSyncFrameEx(0, 6, 0); 
		
        oslReadKeys();
		if (!HBDownloading) {
            if (timer++ > 100) timer = 11;
	        if (osl_keys->pressed.up || (osl_keys->held.up && timer>10)) {
				timer = 0;
				if (MoveLinkFlag) {
					HBListChanged = 1;
					moveHBup(HBlist, selected, 1);
				}
				if (--selected < 0) { 
					selected = 0;	 
				}
				if (selected < first) {
					first -= (visible + 1);
					if (first < 0) {
						first = 0;
					}
				}
			} else if (osl_keys->pressed.down || (osl_keys->held.down && timer>10)) {
				timer = 0;
				if (MoveLinkFlag) {
					HBListChanged = 1;
					moveHBdown(HBlist, selected, 1);
				}
				if (++selected >= total) 
					selected = total - 1;
				if (selected > (first + visible)) {
					first += (visible + 1);
					if (first >= total) {
						first = total - 1;
					}
				}
	        } else if (osl_keys->released.L){
				i = selected;
				selected -= (visible + 1);
				if (selected < 0) 
					selected = 0;	        
				if (selected < first) { 
					first -= (visible + 1);
					if (first < 0) {
						first = 0;
					}
				}
				if (MoveLinkFlag && (i != selected)) {
					HBListChanged = 1;
					moveHBup(HBlist, i, i - selected );
				}
	        } else if (osl_keys->released.R) {
				i = selected;
				selected += (visible + 1);
				if (selected >= total) 
					selected = total - 1;
				if (selected > (first + visible)) {
					first +=  (visible + 1);
					if (first >= total) {
						first = total - 1;
					}
				}
				if (MoveLinkFlag && (i != selected)) {
					HBListChanged = 1;
					moveHBdown(HBlist, i, selected - i);
				}
			}  else if (osl_keys->released.cross){
				MoveLinkFlag ^= 1;
	        } else if (osl_keys->released.square){
				HBlist[selected].download ^= 1;
			} else if (osl_keys->released.select){
				HBSelectAll ^= 1;
				for (i = 0; i < total; i++) {
					HBlist[i].download = HBSelectAll;
				}			
			} else if (osl_keys->released.start){
				if (!HBDownloading) {
					if (HBListChanged) {
						saveHBlist(HBlist, total);
					}				
					if (initNetwork() == 0) { // 0 for successfully connected, 1 if user cancelled and an error code if there was some error.
						HBDownloading = 1;
						// Refresh Download Stats
						for (i = 0; i <= total ; i++) {
							strcpy(HBlist[i].size, "");
							strcpy(HBlist[i].speed, "");
							strcpy(HBlist[i].error, "");
						}
						selected = 0;
						first = 0;
						HBListChanged = 0;
						curl = curl_easy_init();
						configCurl(curl);						
					}
					
					oslInit(0);
					oslInitGfx(OSL_PF_8888, 1);
				}
	        } 
		} else {
			// If downloading then check for cancel or process next link
        	if (osl_keys->held.circle){
				if (HBDownloading) {
					HBDownloading = 0;
					strcpy(HBlist[selected].speed, "CANCELLED");
					curl_easy_cleanup(curl);
					netTerm();
					UnloadModules();							

				}
			}
			
			// Download files
			if (HBDownloading) { // Wait a few interations to allow time to be redrawn after network options displayed
				if (selected < total) {
					if (HBlist[selected].download) {
						if (get_link(curl, HBlist, selected, total - 1) != 1) {
							strcpy(HBlist[selected].error, ERRORDESC);
						} else if (write_html(HBlist, selected, total - 1) != 1) {
							strcpy(HBlist[selected].error, ERRORDESC);
						}
					} else {
						strcpy(HBlist[selected].speed, "SKIPPED");
					}
					// Add the file to the index even if it is not updated this session so that the user can see the old version of the file
					if (update_index(HBlist, selected) != 1) {
						oslDebug("update_index completed");
						strcpy(HBlist[selected].error, ERRORDESC);
					}
					if (strcmp(HBlist[selected].error, "") != 0) {
						strcpy(HBlist[selected].speed, "FAILED");
					}
					if (++selected > first + visible) {
						first++;
					}
					
				} else {
					HBDownloading = 0;
					strcpy(HBlist[total].error, "Download completed");
					curl_easy_cleanup(curl);
					netTerm();
					UnloadModules();	
				}
			}
		}
    }
    return 0;
}


const OSL_VIRTUALFILENAME __image_ram_files[] = {
	{"ram:/Media/bkg.png", (void*)bkg_png, sizeof(bkg_png), &VF_MEMORY},
	{"ram:/Media/start.png", (void*)start_png, sizeof(start_png), &VF_MEMORY},
	{"ram:/Media/cross.png", (void*)cross_png, sizeof(cross_png), &VF_MEMORY},
	{"ram:/Media/circle.png", (void*)circle_png, sizeof(circle_png), &VF_MEMORY},
	{"ram:/Media/square.png", (void*)square_png, sizeof(square_png), &VF_MEMORY},
	{"ram:/Media/rss.png", (void*)rss_png, sizeof(rss_png), &VF_MEMORY},
	{"ram:/Media/html.png", (void*)html_png, sizeof(html_png), &VF_MEMORY},
	{"ram:/Media/select.png", (void*)select_png, sizeof(select_png), &VF_MEMORY},
	{"ram:/Media/unselect.png", (void*)unselect_png, sizeof(unselect_png), &VF_MEMORY},
	{"ram:/Media/R.png", (void*)R_png, sizeof(R_png), &VF_MEMORY},
	{"ram:/Media/L.png", (void*)L_png, sizeof(L_png), &VF_MEMORY}
};

int initOSLib(){
    oslSetQuitOnLoadFailure(1);
	oslAddVirtualFileList((OSL_VIRTUALFILENAME*)__image_ram_files, oslNumberof(__image_ram_files));
	bkg = oslLoadImageFilePNG("ram:/Media/bkg.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	imgRSS = oslLoadImageFilePNG("ram:/Media/rss.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	imgHTML = oslLoadImageFilePNG("ram:/Media/html.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	imgUnselect = oslLoadImageFilePNG("ram:/Media/unselect.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	startb = oslLoadImageFilePNG("ram:/Media/start.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	imgSelect = oslLoadImageFilePNG("ram:/Media/select.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	cross = oslLoadImageFilePNG("ram:/Media/cross.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	circle = oslLoadImageFilePNG("ram:/Media/circle.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	imgSquare = oslLoadImageFilePNG("ram:/Media/square.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	R = oslLoadImageFilePNG("ram:/Media/R.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
	L = oslLoadImageFilePNG("ram:/Media/L.png", OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
    oslSetKeyAutorepeatInit(40);
    oslSetKeyAutorepeatInterval(10);
    oslIntraFontInit(INTRAFONT_CACHE_MED);
	pgfFont = oslLoadFontFile("flash0:/font/ltn0.pgf");
    oslIntraFontSetStyle(pgfFont, 0.5, RGBA(255,255,255,255), RGBA(0,0,0,0), INTRAFONT_ALIGN_LEFT);
    oslSetFont(pgfFont);
	oslSetKeyAnalogToDPad(ANALOG_SENS);
    return 0;
}





/* Main: */
int main(){
    oslInit(0);
    oslInitGfx(OSL_PF_8888, 1);
   // oslInitAudio();
    initOSLib();
    tzset();
	mainMenu();
    oslEndGfx();
	oslQuit();
    return 0;

}


