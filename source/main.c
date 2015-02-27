#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <3ds.h>

/*****************************************************************
| httpDownload will return an result message of status
|	0 = OK
|	1 = Empty response?
|	-2 = Error 200
|	Any values not 0 are errors (need to look into this more)
*****************************************************************/
Result httpDownload(httpcContext *httpContext)
{
	Result returnCode = 0;
	u8* frameBufferTop;
	u32 statusCode=0;
	u32 rawSize=0, contentSize=0;
	u8 *buf;

	returnCode = httpcBeginRequest(httpContext);
	if(returnCode != 0)
		return returnCode;

	returnCode = httpcGetResponseStatusCode(httpContext, &statusCode, 0);
	if(returnCode != 0)
		return returnCode;

	if(statusCode != 200)
		return -2;

	returnCode=httpcGetDownloadSizeState(httpContext, NULL, &contentSize);
	if(returnCode != 0)
		return returnCode;

	printf("Size: %"PRId32"\n", contentSize);
	gfxFlushBuffers();

	buf = (u8*)malloc(contentSize);

	if(buf == NULL)
		return -1;

	memset(buf, 0, contentSize);


	returnCode = httpcDownloadData(httpContext, buf, contentSize, NULL);
	if(returnCode!=0)
	{
		free(buf);
		return returnCode;
	}

	rawSize = contentSize;
	if(rawSize > (240*400*3*2))
		rawSize = 240*400*3*2;

	frameBufferTop = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	memcpy(frameBufferTop, buf, rawSize);

	gfxFlushBuffers();
	gfxSwapBuffers();

	frameBufferTop = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	memcpy(frameBufferTop, buf, rawSize);

	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();

	free(buf);
	
	return 0;
}

int main()
{
	bool use3D = false;

	gfxInitDefault();
	gfxSet3D(use3D);
	httpcInit();

	consoleInit(GFX_BOTTOM, NULL);
	printf("Started console on bottom screen.\n");

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();
		u32 kDown = hidKeysDown();

		if(kDown & KEY_A)
		{
			char *webUrl = "http://devkitpro.org/misc/httpexample_rawimg.rgb";
			Result retResult = 0;
			httpcContext httpContext;

			gfxFlushBuffers();

			printf("[---] Attempting to initiate download...\n[-->] Connecting to:\n\n%s\n\n", webUrl);

			retResult = httpcOpenContext(&httpContext, webUrl , 0);
			gfxFlushBuffers();

			printf("[DBG] retResult value is: %"PRId32"\n\n", retResult);

			if(retResult == 0)
			{
				printf("[!!!] Connected!\n[<--]Downloading files...\n");
				
				retResult = httpDownload(&httpContext);

				printf("[DBG] retResult value is: %"PRId32"\n[-X>] Closing web request...\n", retResult);
				httpcCloseContext(&httpContext);

				printf("[!!!] Done!\n");
			}
			else
			{
				printf("[!!!] Error: was not able to connect to server!");
			}
			gfxFlushBuffers();
		}

		if(kDown & KEY_DDOWN)
		{
			consoleInit(GFX_BOTTOM, NULL);
			printf("Starting console on bottom screen!\n");
		}
		if(kDown & KEY_DUP)
		{
			consoleInit(GFX_TOP, NULL);
			printf("Starting console on top screen!\n");
		}

		if(kDown & KEY_SELECT)
		{
			if(use3D)
			{
				use3D = false;

				printf("Stopping 3D feature!\n");
				gfxSet3D(use3D);
			}
			else
			{
				use3D = true;

				printf("Starting 3D feature!\n");
				gfxSet3D(use3D);
			}
		}

		if (kDown & KEY_START)
		{
			printf("Exiting program...\n");
			break;
		}

		/*
			// Example rendering code that displays a white pixel
			// Please note that the 3DS screens are sideways (thus 240x400 and 240x320)
			u8* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
			memset(fb, 0, 240*400*3);
			fb[3*(10+10*240)] = 0xFF;
			fb[3*(10+10*240)+1] = 0xFF;
			fb[3*(10+10*240)+2] = 0xFF;
		*/

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	gfxExit();
	return 0;
}
