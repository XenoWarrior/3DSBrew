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
	Result returnCode = 0;			// The result returned from HTTP requests
	u8* frameBufferTop;				// The buffer for rendering smea's image on top-screen
	u32 statusCode = 0;				// HTTP status code returned from httpcGetResponseStatusCode
	u32 rawSize=0, contentSize=0;	// Sets the rawSize (dimensions of file) and contentSize (total bytes)
	char *ufSize = "";				// User friendly size conversion
	char *sizeType = "KB";			// Set size to KB
	u32 convertedSize = 0;			// User friendly size conversion (actual value)
	u8 *buf;						// Uses malloc to allocate space to the memory for image

	returnCode = httpcBeginRequest(httpContext);							// Start the HTTP request
	if(returnCode != 0)														// Check if we have any other error code
		return returnCode;													// Return error code if not 0

	returnCode = httpcGetResponseStatusCode(httpContext, &statusCode, 0);	// Check the response from HTTP server
	if(returnCode != 0)														// Check if we have any other error code
		return returnCode;													// Return error code if not 0
	if(statusCode != 200)													// Same as above, but for the server status
		return -2;

	returnCode=httpcGetDownloadSizeState(httpContext, NULL, &contentSize);	// Assuming this gets the file size?
	if(returnCode != 0)														// Same as above to return errors
		return returnCode;

	// UNFINISHED!
	// TODO: CONVERT SIZE TO STRING AND APPEND TO STRING FOR USER FRIENDLY OUTPUT!
	convertedSize = contentSize / 100;
	ufSize = malloc(strlen(convertedSize) + strlen(sizeType));
	strcpy(ufSize, convertedSize);
	strcpy(ufSize, sizeType);

	printf("[<--] Size: %s\n", ufSize);										// Output size to screen
	gfxFlushBuffers();														// Flush graphics buffer

	buf = (u8*)malloc(contentSize);											// Allocate memory for the image

	if(buf == NULL)															// If there was no space to allocate, then the download was 0?
		return -1;															// Return error code as -1 (check this in main())

	memset(buf, 0, contentSize);

	returnCode = httpcDownloadData(httpContext, buf, contentSize, NULL);	// If all went well, actually download the file (returns value)

	if(returnCode != 0)														// Basically checks if the OK code was not returned
	{
		free(buf);															// Frees the memory
		return returnCode;													// Returns the error code back to main()
	}

	rawSize = contentSize;
	if(rawSize > (240*400*3*2))												// Check if the size is more than screen
		rawSize = 240*400*3*2;												// Force the size to fit screen

	frameBufferTop = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);		// Set our top screen buffer to TOP and LEFT of screen
	memcpy(frameBufferTop, buf, rawSize);									// Copy the memory from buf to the frame buffer stating size

	gfxFlushBuffers();														// Flush buffer
	gfxSwapBuffers();														// Swap buffer

	frameBufferTop = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);		// Does this twice to stop flickering?
	memcpy(frameBufferTop, buf, rawSize);

	// Flush the buffer and swap it
	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();

	// Free the memory
	free(buf);
	
	// Return the OK status message
	return 0;
}


/*****************************************************************
| main execution code for application
|	0 = Exit application
|
|	Will comment more of this code soon™
*****************************************************************/
int main()
{
	bool use3D = false;

	gfxInitDefault();
	gfxSet3D(use3D);
	httpcInit();

	consoleInit(GFX_BOTTOM, NULL);
	printf("[CMD] Started console on bottom screen.\n This testing application implements a few features such as:\n  Downloading files (Press A)\n   Starting up 3D mode (Press Select)\n   Rendering graphics (Press B)\n   Closing application (Press Start)\n");

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();
		u32 kDown = hidKeysDown();

		if(kDown & KEY_B)
		{
			// Example rendering code that displays a white pixel
			// Please note that the 3DS screens are sideways (thus 240x400 and 240x320)
			u8* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
			memset(fb, 0, 240*400*3);
			fb[3*(10+10*240)] = 0xFF;
			fb[3*(10+10*240)+1] = 0xFF;
			fb[3*(10+10*240)+2] = 0xFF;
		}

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
				printf("[!!!] Connected!\n[<--] Downloading files...\n");
				
				retResult = httpDownload(&httpContext);

				if(retResult == 0)
				{
					printf("[DBG] retResult value is: %"PRId32"\n[-X>] Closing web request...\n", retResult);
					httpcCloseContext(&httpContext);

					printf("[!!!] Done!\n");
				}
				else
				{
					printf("[!!!] Status code returned: %"PRId32, retResult);
				}
			}
			else
			{
				printf("[!!!] Error: was not able to connect to server!\n");
			}
			gfxFlushBuffers();
		}

		if(kDown & KEY_SELECT)
		{
			if(use3D)
				use3D = false;
			else
				use3D = true;

			printf("[3D ] Setting 3D feature to %i\n", use3D);
			gfxSet3D(use3D);
		}

		if (kDown & KEY_START)
		{
			printf("[!!!] Exiting program...\n");
			break;
		}

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	gfxExit();
	return 0;
}
