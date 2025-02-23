#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include <sys/select.h>
#include <curl/curl.h>
#include "fileOperation.h"
#include <oslib/oslib.h>

int ERRORNO = 0;
char FILE_LOCATION[13] = "/PSP/COMMON/";


static struct HTMLReplace HTMLReplaceList[]=
  {
    {"<channel", "<!-- "},
    {"<item", "<!-- "},
		{"<title>", "//--><BR><BR><font face='Arial' color='#006600'><B>"},
		{"</title>", "</B></font><BR><!--"},
		{"<link>", "//--><a href='#t'>Top</a> - <a href='"},
		{"</link>", "'>Full article...</a> <BR> <!--"},
		{"<description>", "//-->"},
			{"<![CDATA[", ""},
			{"&gt;", ">"},
			{"&lt;", "<"},
			{"<p>", ""},
			{"</p>", "<BR><BR>"},
			{"<BR><BR>]]>", ""},
			{"<BR>]]>", ""},
			{"]]>", ""},
			{"<br/><br/>", ""},
			{"&#39;", "&apos;"},
			{"&amp;", "&"},
		{"</description>", "<BR><!--"},
//		{"<pubDate>", "//--><font size=2 color=gray>"},
//		{"</pubDate>", "</font><!--"},
		{"<img", "<noimg"},
    {"</item>", "//-->"}
};



/* Get homebrew list: */
int getHBlist(struct homebrew *HBlist) {
	FILE *FileList;
	int HBfound = 0;    

	if(!(FileList = fopen(FILENAME_FEEDS, "r"))) { 
		ERRORNO = 112; sprintf(ERRORDESC, " E%i Unable to open %s", ERRORNO, FILENAME_FEEDS); goto Err_Handler;
	}

	while (fscanf(FileList, "%s %s", HBlist[HBfound].name, HBlist[HBfound].path) == 2) {		
		if (strstr(HBlist[HBfound].name, FILETYPE_RSS)) 
			HBlist[HBfound].type = TYPE_RSS;
		else if (strstr(HBlist[HBfound].name, FILETYPE_HTML)) 
			HBlist[HBfound].type = TYPE_HTML;
		else 
			HBlist[HBfound].type = 2;
		strcpy(HBlist[HBfound].size, "");
		strcpy(HBlist[HBfound].speed, "");
		strcpy(HBlist[HBfound].error, "");
		strcpy(HBlist[HBfound].displayname, HBlist[HBfound].name);
		HBlist[HBfound].download = 1;
        HBfound++;
	}
	fclose(FileList);

	return HBfound;

Err_Handler:
	if (FileList) fclose(FileList);
	return 0;
}
  

// Move HB up: 
int moveHBup(struct homebrew *HBlist, int index, int count){
	struct homebrew tmp;
	int i;
	
    tmp = HBlist[index];
	for (i = 0; i < count && index - i > 0; i++) {
        HBlist[index - i] = HBlist[index - i - 1];
	}
	HBlist[index - i] = tmp;
    return 1;
}

// Move HB down: 
int moveHBdown(struct homebrew *HBlist, int index, int count){
    struct homebrew tmp;
	int i;
	
	tmp = HBlist[index];
	for (i = 0; i < count; i++) {
		HBlist[index + i] = HBlist[index + i + 1];
	}
	HBlist[index + i] = tmp;
    return 1;
}

// Save HB list: 
int saveHBlist(struct homebrew *HBlist, int HBfound) {
	FILE *FileList;
    int i = 0;
		
	if(!(FileList = fopen(FILENAME_FEEDS, "w"))) { 
		ERRORNO = 112; sprintf(ERRORDESC, " E%i Unable to write to %s", ERRORNO, FILENAME_FEEDS); goto Err_Handler;
	}
	for (i = 0; i <= HBfound ; i++) {
		fprintf(FileList, "%s %s\n", HBlist[i].name, HBlist[i].path);
	}
	fclose(FileList);
	
	return 0;

Err_Handler:
	if (FileList) fclose(FileList);
	return -ERRORNO;
}


  
  
  
int get_link(CURL *curl, struct homebrew *HBlist, int HBselected, int HBCount) {
	FILE *FileOutput;
	CURLcode res;
	char FileNameFull[200];
	double curl_totaltime, curl_sizedownload, curl_speeddownload;
	
	sprintf(FileNameFull, "%s%s", FILE_LOCATION, HBlist[HBselected].name);
	if(!(FileOutput = fopen(FileNameFull,"w"))) {
		ERRORNO = 101; sprintf(ERRORDESC, " E%i Unable to write to file %s", ERRORNO, FileNameFull); goto Err_Handler;
	}
	// Write Header. RSS headers are written in write_html so that we don't 
	// corrupt the xml in the./xml file
	if (HBlist[HBselected].type == TYPE_HTML) {
		write_header(HEADER, FileOutput, HBlist, HBselected, HBCount);
	}
	
	curl_easy_setopt(curl, CURLOPT_URL, HBlist[HBselected].path);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)FileOutput);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		
	res = curl_easy_perform(curl);
	if (res != 0) {
		ERRORNO = res; sprintf(ERRORDESC, " C%i %s", ERRORNO, curl_easy_strerror(res)); goto Err_Handler;
	}
	// Write Footer
	if (HBlist[HBselected].type == TYPE_HTML) {
		write_header(HEADER, FileOutput, HBlist, HBselected, HBCount);
	}
	fclose(FileOutput);

	curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &curl_sizedownload);
	curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &curl_totaltime);
	curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &curl_speeddownload);
//	pspDebugScreenPrintf("(%.0fKB %.1fs %.0fKB/s)\n", curl_sizedownload/1024, curl_totaltime, curl_speeddownload/1024);
//	pspDebugScreenPrintf(" %.0fKB %.0fKB/s\n", curl_sizedownload/1024, curl_speeddownload/1024);
	sprintf(HBlist[HBselected].size, "%.0fKB", curl_sizedownload/1024);
	sprintf(HBlist[HBselected].speed, "%.0fKB/s", curl_speeddownload/1024);

	return 1;

Err_Handler:
	if (FileOutput) fclose(FileOutput);
	return -ERRORNO;
}		


// callback for curl to return data 
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int written = fwrite(ptr, size, nmemb, (FILE *)stream);
	
	return written;
}

int update_index(struct homebrew *HBlist, int selected)
{
	FILE *FileIndex;
	char FullFileName[105];
	char FileNameIndex[100];
	
	if (HBlist[selected].type == TYPE_RSS || HBlist[selected].type == TYPE_HTML) {
		sprintf(FileNameIndex,"%s%s", FILE_LOCATION, FILE_INDEX);
		if (selected == 0) {
			if(!(FileIndex = fopen(FileNameIndex,"w"))) {
				ERRORNO = 103; sprintf(ERRORDESC, " E%i Unable to write to file %s", ERRORNO, FileNameIndex); goto Err_Handler;
			}
			fprintf(FileIndex, "<font face='Arial' size=+2 color='#006600'>DownloadLinks Index</font><BR>\n");

		} else {
			if(!(FileIndex = fopen(FileNameIndex,"a"))) {
				ERRORNO = 104; sprintf(ERRORDESC, " E%i Unable to add links to file %s", ERRORNO, FileNameIndex); goto Err_Handler;
			}
		}
		strcpy(FullFileName, HBlist[selected].name);
		if (HBlist[selected].type == TYPE_RSS) { 
			strcat(FullFileName, FILETYPE_HTML); 
		}
		fprintf(FileIndex, "<a href=\"file://%s%s\">%s</a><BR>\n",FILE_LOCATION, FullFileName, HBlist[selected].displayname);					
		fclose(FileIndex);
	}
	
	return 1;
	
Err_Handler:
	if (FileIndex) fclose(FileIndex);
	return -ERRORNO;
}


int write_header(enum HeaderType header_type, FILE* FileOutput, struct homebrew *HBlist, int selected, int HBCount) 
{
	char NextFileNameFull[267];

	if (header_type == HEADER) {
		fprintf(FileOutput, "<font face=arial><a href='file://%s%s'>Index</a>", FILE_LOCATION, FILE_INDEX);
		fprintf(FileOutput, " | <a name='t'></a><a href='#b'>Bottom</a>");
	} else {
		fprintf(FileOutput, "<BR><BR><BR><font face=arial><a href='file://%s%s'>Index</a>", FILE_LOCATION, FILE_INDEX);
		fprintf(FileOutput, " | <a name='b'></a><a href='#t'>Top</a>");
	}
	if (selected < HBCount) {
		strcpy(NextFileNameFull, HBlist[selected+1].name);
		if (HBlist[selected+1].type == TYPE_RSS) { 
			strcat(NextFileNameFull, FILETYPE_HTML); 
		}
		fprintf(FileOutput, " | <a href='file://%s%s'>%s</a>", FILE_LOCATION, NextFileNameFull, HBlist[selected+1].displayname);
	}
	fprintf(FileOutput, " | %s </font><BR><BR>\n", HBlist[selected].displayname);		
	
	return 1;
}



int write_html(struct homebrew *HBlist, int HBselected, int HBTotal)
{
	FILE *FileRSS, *FileHTML;
	char FileNameFull[262], NextFileNameFull[262];
	char *buffer;
	int i = 0, found = -1;
	char HTMLLink[100];
	
	buffer = (char *) malloc(XMLLine_Length + 1);
	oslAssert(buffer);
		
	// Read RSS File
	sprintf(FileNameFull, "%s%s", FILE_LOCATION, HBlist[HBselected].name);
	if (!(FileRSS = fopen(FileNameFull,"r"))) {
		ERRORNO = 131; sprintf(ERRORDESC, " E%i Unable to open file %s", ERRORNO, FileNameFull); goto Err_Handler;
	}

	// Write HTML File
	sprintf(FileNameFull, "%s%s", FileNameFull, FILETYPE_HTML);
	if(!(FileHTML = fopen(FileNameFull,"w"))) {
		ERRORNO = 130; sprintf(ERRORDESC, " E%i Unable to write to file %s", ERRORNO, FileNameFull); goto Err_Handler;
	}					
	write_header(HEADER, FileHTML, HBlist, HBselected, HBTotal);

	if (HBselected < HBTotal) {
		strcpy(NextFileNameFull, HBlist[HBselected+1].name);
		if (HBlist[HBselected+1].type == TYPE_RSS) { 
			strcat(NextFileNameFull, FILETYPE_HTML); 
		}
		for(i=0; i < HBTotal && found == -1; i++) {
			//oslDebug("i=%i replace[i]=%s strcmp=%i",i, HTMLReplaceList[i].oldvalue, strcmp(HTMLReplaceList[i].oldvalue, "<link>"));
			if (strcmp(HTMLReplaceList[i].oldvalue, "<link>") == 0) {
				strcpy(HTMLLink, HTMLReplaceList[i].newvalue);
				sprintf(HTMLReplaceList[i].newvalue, "%sfile://%s%s'>Next %s</a> - <a href='", HTMLReplaceList[i].newvalue, FILE_LOCATION, NextFileNameFull, HBlist[HBselected+1].displayname); 
				found = i;
			}
		}
	}
	while (fgets(buffer, XMLLine_Length, FileRSS) != NULL) {
		//oslDebug("replace[%i]=%s value=%s", found, HTMLReplaceList[found].oldvalue, HTMLReplaceList[found].newvalue);
		for(i=0; HTMLReplaceList[i].oldvalue[0]; i++) {
			buffer = replace(buffer, HTMLReplaceList[i].oldvalue, HTMLReplaceList[i].newvalue);
		}
		fprintf(FileHTML,"%s",buffer);				
		free(buffer);
		buffer = (char *) malloc(XMLLine_Length);
	}
	
	if (found >= 0) 
		strcpy(HTMLReplaceList[found].newvalue, HTMLLink);
	free(buffer);
	fclose(FileRSS);

	write_header(FOOTER, FileHTML, HBlist, HBselected, HBTotal);
	fclose(FileHTML);

	return 1;
	
Err_Handler:
	free(buffer);
	if (FileRSS) fclose(FileRSS);
	if (FileHTML) fclose(FileHTML);
	return -ERRORNO;
}







int configCurl(CURL *curl) 
{
	FILE* FileConfig;
	char curl_option[100], curl_value[100];
	int count = 0;

	// Set default values
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curl, CURLOPT_MUTE, 1);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);

	if((FileConfig = fopen(FILENAME_CONFIG,"r")) == NULL) {
		ERRORNO = 140; sprintf(ERRORDESC, " E%i Unable to read config file %s. Default values used.", ERRORNO, FILENAME_CONFIG); goto Err_Handler;
	}
	while (fscanf(FileConfig, "%s=%s", curl_option, curl_value) > 1) {
		// TODO need to check that the value is a number and convert to number before applying to setopt
		//oslDebug("Config open=%s value=%s", curl_option, curl_value);
		if (strncmp(curl_option,":",1) == 0) {
			if (strcmp(curl_option,"CONNECTTIMEOUT") != 0) curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, atoi(curl_value));
			else if (strcmp(curl_option, "TIMEOUT") != 0) curl_easy_setopt(curl, CURLOPT_TIMEOUT, atoi(curl_value));
			else if (strcmp(curl_option, "USERPWD") != 0) curl_easy_setopt(curl, CURLOPT_USERPWD, curl_value);
			else if (strcmp(curl_option, "PROXY") != 0) curl_easy_setopt(curl, CURLOPT_PROXY, curl_value);
			else if (strcmp(curl_option, "PROXYUSERPWD") != 0)  curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, curl_value);
			else if (strcmp(curl_option, "PROXYPORT") != 0) curl_easy_setopt(curl, CURLOPT_PROXYPORT, atoi(curl_value));
			else if (strcmp(curl_option, "OUTPUT_DIR") != 0) strcpy(FILE_LOCATION, curl_value);
			else {
				ERRORNO = 141; sprintf(ERRORDESC, " E%i Config name %s unknown. Value %s not assigned", ERRORNO, curl_option, curl_value); goto Err_Handler;
				count--;
			}
			count++;
		}	
	}	
	fclose(FileConfig);
	return count; 
Err_Handler:
	if (FileConfig) fclose(FileConfig);
	return -ERRORNO;
}


				
char *replace(char *str, char *oldchaine, char *newchaine)
{ 
	char *wstr,*temp,*pos;
	
	if (str==NULL || strlen(oldchaine)==0) {return NULL; }
	wstr=(char *) strdup(str);
	free(str);
	while ((pos=strstr(wstr,oldchaine))!=NULL)
	{
		temp=(char*) malloc(strlen(wstr)-strlen(oldchaine)+strlen(newchaine)+1);
		oslAssert(temp);
		strncpy(temp,wstr,pos-wstr);
		temp[pos-wstr]=0;
		strcat(temp,newchaine);
		strcat(temp,pos+strlen(oldchaine));
		temp[strlen(wstr)-strlen(oldchaine)+strlen(newchaine)]=0;
		free(wstr);
		wstr=temp;
	}
	return wstr;
}
