#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4)
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2)

/* Graphics stuff, based on cube sample */
static unsigned int __attribute__((aligned(16))) list[262144];

int initNetwork();
int LoadModules();
int UnloadModules();
static void setupGu();
static void drawStuff(void);
int netDialog();
void netInit(void);
void netTerm(void);


