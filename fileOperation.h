#define MAX_HB 500
#define FILETYPE_RSS ".rss"
#define FILETYPE_HTML ".html"
#define FILENAME_FEEDS "feeds.txt"
#define FILENAME_CONFIG "config.ini"
#define FILE_INDEX "index.html"
#define XMLLine_Length 8192

#define TYPE_RSS   0
#define TYPE_HTML  1
#define TYPE_OTHER 2


enum HeaderType{HEADER, FOOTER};
char ERRORDESC[50];

// HTML Convert
struct HTMLReplace{
    char oldvalue[20];   
    char newvalue[200]; 
};


struct homebrew{
    char name[50];
	char displayname[50];
    char path[200];
	short int type;
	char size[20];
	char speed[20];
	char error[250];
	int download;
};


int moveHBup(struct homebrew *HBlist, int index, int count );
int moveHBdown(struct homebrew *HBlist, int index, int count);
int saveHBlist(struct homebrew *HBlist, int HBcount);
int getHBlist(struct homebrew *HBlist);

int get_link(CURL *curl, struct homebrew *HBlist, int selected, int HBCount);
int update_index(struct homebrew *HBlist, int selected);
int write_header(enum HeaderType header_type, FILE* FileOutput, struct homebrew *HBlist, int selected, int HBCount);
int write_html(struct homebrew *HBlist, int selected, int HBCount);
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
char *replace(char *str, char *oldchaine, char *newchaine);
int configCurl(CURL *curl);



