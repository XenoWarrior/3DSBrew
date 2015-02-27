#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <3ds.h>

Result http_download(httpcContext *context)
{
	Result ret=0;
	u8* framebuf_top;
	u32 statuscode=0;
	u32 size=0, contentsize=0;
	u8 *buf;

	ret = httpcBeginRequest(context);
	if(ret!=0)return ret;

	ret = httpcGetResponseStatusCode(context, &statuscode, 0);
	if(ret!=0)return ret;

	if(statuscode!=200)return -2;

	ret=httpcGetDownloadSizeState(context, NULL, &contentsize);
	if(ret!=0)return ret;

	printf("size: %"PRId32"\n",contentsize);
	gfxFlushBuffers();

	buf = (u8*)malloc(contentsize);
	if(buf==NULL)return -1;
	memset(buf, 0, contentsize);


	ret = httpcDownloadData(context, buf, contentsize, NULL);
	if(ret!=0)
	{
		free(buf);
		return ret;
	}

	size = contentsize;
	if(size>(240*400*3*2))size = 240*400*3*2;

	framebuf_top = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	memcpy(framebuf_top, buf, size);

	gfxFlushBuffers();
	gfxSwapBuffers();

	framebuf_top = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	memcpy(framebuf_top, buf, size);

	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();

	free(buf);
	free(framebuf_top);
	
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

			printf("Attempting to initiate download...\n");
			retResult = httpcOpenContext(&httpContext, webUrl , 0);
			gfxFlushBuffers();

			printf("Opened HTTP request!\nConnecting to:\n>>%s\n", webUrl);
			printf("#DEBUG retResult value is: %"PRId32"\n", retResult);
			gfxFlushBuffers();

				gfxFlushBuffers();
				printf("Success! Running final download process...\n");
				gfxFlushBuffers();
				
				retResult = http_download(&httpContext);
				gfxFlushBuffers();

				printf("#DEBUG retResult value is: %"PRId32"\n",retResult);
				gfxFlushBuffers();
				
				printf("Closing web request...\n");
				gfxFlushBuffers();

				httpcCloseContext(&httpContext);
				gfxFlushBuffers();

				printf("Done!\n");
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
