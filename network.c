#include <pspkernel.h>
#include <pspdisplay.h>
#include <math.h>
#include <psputility.h>
#include <pspgu.h>
#include <pspgum.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#include <pspsdk.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>

#include "network.h"



int initNetwork() {
	int result = 0;
	
	LoadModules();
	sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
	netInit();
	setupGu();
	result = netDialog();
	if (result != 0) {
		netTerm();
		UnloadModules();	
	}
	return result;
}
				
int LoadModules()
{
	int result = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	if (result < 0)
	{
		printf("Error 0x%08X loading pspnet.prx.\n", result);
		return result;
	}

	result = sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
	if (result < 0)
	{
		printf("Error 0x%08X loading pspnet_inet.prx.\n", result);
		return result;
	}

	return 0;
}


int UnloadModules()
{
	int result = sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
	if (result < 0)
	{
		printf("Error 0x%08X unloading pspnet.prx.\n", result);
		return result;
	}

	result = sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
	if (result < 0)
	{
		printf("Error 0x%08X unloading pspnet_inet.prx.\n", result);
		return result;
	}

	return 0;
}



static void setupGu()
{
		sceGuInit();

    	sceGuStart(GU_DIRECT,list);
    	sceGuDrawBuffer(GU_PSM_8888,(void*)0,BUF_WIDTH);
    	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)0x88000,BUF_WIDTH);
    	sceGuDepthBuffer((void*)0x110000,BUF_WIDTH);
    	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
    	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
    	sceGuDepthRange(0xc350,0x2710);
    	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
    	sceGuEnable(GU_SCISSOR_TEST);
    	sceGuDepthFunc(GU_GEQUAL);
    	sceGuEnable(GU_DEPTH_TEST);
    	sceGuFrontFace(GU_CW);
    	sceGuShadeModel(GU_SMOOTH);
    	sceGuEnable(GU_CULL_FACE);
    	sceGuEnable(GU_CLIP_PLANES);
    	sceGuFinish();
    	sceGuSync(0,0);

    	sceDisplayWaitVblankStart();
    	sceGuDisplay(GU_TRUE);
}

static void drawStuff(void)
{
	static int val = 0;

	sceGuStart(GU_DIRECT, list);

	sceGuClearColor(0xff000000);
	
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective(75.0f,16.0f/9.0f,0.5f,1000.0f);
	   
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	    
	sceGuFinish();
	sceGuSync(0,0);
	
	val++;
}

int netDialog()
{
	int running = 1;

   	pspUtilityNetconfData data;

	memset(&data, 0, sizeof(data));
	data.base.size = sizeof(data);
	data.base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
	data.base.buttonSwap = PSP_UTILITY_ACCEPT_CROSS;
	data.base.graphicsThread = 17;
	data.base.accessThread = 19;
	data.base.fontThread = 18;
	data.base.soundThread = 16;
	data.action = PSP_NETCONF_ACTION_CONNECTAP;
	
	struct pspUtilityNetconfAdhoc adhocparam;
	memset(&adhocparam, 0, sizeof(adhocparam));
	data.adhocparam = &adhocparam;

	sceUtilityNetconfInitStart(&data);
	
	while(running)
	{
		drawStuff();

		switch(sceUtilityNetconfGetStatus())
		{
			case PSP_UTILITY_DIALOG_NONE:
				break;

			case PSP_UTILITY_DIALOG_VISIBLE:
				sceUtilityNetconfUpdate(1);
				break;

			case PSP_UTILITY_DIALOG_QUIT:
				sceUtilityNetconfShutdownStart();
				running =  -1; //sceKernelExitGame();
				break;
				
			case PSP_UTILITY_DIALOG_FINISHED:
				running = 0;
				break;

			default:
				break;
		}		
		sceDisplayWaitVblankStart();
		sceGuSwapBuffers();		
	}
	
	return data.base.result; // 0 for successfully connected, 1 if user cancelled and an error code if there was some error.
	//return running;
}

void netInit(void)
{
	sceNetInit(128*1024, 42, 4*1024, 42, 4*1024);
	sceNetInetInit();
	sceNetApctlInit(0x8000, 48);
}

void netTerm(void)
{
	sceNetApctlTerm();
	sceNetInetTerm();	
	sceNetTerm();
}
