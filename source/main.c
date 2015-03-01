#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <3ds.h>

/*****************************************************************
| geApiParse will return an result message of status
|	0 = OK
|	1 = Empty response?
|	-2 = Error 200
|	Any values not 0 are errors (need to look into this more)
|
|	geApiParse will require use of JSON decoding eventually!
|	For now, just print out the message got from server.
|
|	API: http://projectge.com/new/api
|	Get API key by logging in with authorised account(s)!
*****************************************************************/
Result geApiParse(httpcContext *httpContext)
{
	Result returnCode = 0;			// The result returned from HTTP requests
	u32 statusCode = 0;				// HTTP status code returned from httpcGetResponseStatusCode
	u32 contentSize=0;				// Sets the contentSize (total bytes)
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

	gfxFlushBuffers();														// Flush graphics buffer

	buf = (u8*)malloc(contentSize);											// Allocate memory for the image

	if(buf == NULL)															// If there was no space to allocate, then the download was 0?
		return -1;															// Return error code as -1 (check this in main())

	memset(buf, 0, contentSize);
	
	// May need a sleep after to allow for the download to finish, partial text downloads are not showing all text unless done a few times
	returnCode = httpcDownloadData(httpContext, buf, contentSize, NULL);	// If all went well, actually download the file (returns value)

	if(returnCode != 0)														// Basically checks if the OK code was not returned
	{
		free(buf);															// Frees the memory
		return returnCode;													// Returns the error code back to main()
	}

	printf("[<--] Message: \n\n%s\n\n", buf);								// Print out the message returned from server

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
	bool use3D = false;		// Set the bool to false to disable 3D. Set in gfxSet3D();

	gfxInitDefault();		// Start graphics controller
	gfxSet3D(use3D);		// Start 3D controller
	httpcInit();			// Start HTTP controller

	// Start console on bottom screen with a message containing various features
	consoleInit(GFX_BOTTOM, NULL);
	printf("ProjectGE 3DS Alpha 0.1\nPress A to connect.\nPress Start to exit\n");

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();					// Scanning the input
		u32 kDown = hidKeysDown();		// Check if input pressed

		// Will check if B is being pressed
		if(kDown & KEY_B)
		{
			// EXAMPLE CODE TO DRAW DOT ON SCREEN
			u8* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
			memset(fb, 0, 240*400*3);
			fb[3*(10+10*240)] = 0xFF;
			fb[3*(10+10*240)+1] = 0xFF;
			fb[3*(10+10*240)+2] = 0xFF;
		}

		// Will check if A is being pressed
		if(kDown & KEY_A)
		{
			char *webUrl = "http://projectge.com/new/api";

			Result retResult = 0;												// Result returned from various calls
			httpcContext httpContext;											// The HTTP handler
			gfxFlushBuffers(); 													// Flush buffers

			printf("[-->] Connecting to:\n\n%s\n\n", webUrl);


			retResult = httpcOpenContext(&httpContext, webUrl , 0);		// Open webrequest
			gfxFlushBuffers();											// Flush buffers

			// Will check if there were any issues with download
			if(retResult == 0)
			{
				printf("[<--] Getting response message...\n");
				
				// Start download from server using the setup handler
				retResult = geApiParse(&httpContext);

				// Check for any errors with download
				if(retResult == 0)
					httpcCloseContext(&httpContext);
				else
					printf("[!!!] Unable to connect to server! (E: %"PRId32")", retResult);

				printf("[---] Done!");
			}
			else
				printf("[!!!] Unable to connect to server! (E: %"PRId32")", retResult);
			// Flush the buffers
			gfxFlushBuffers();
		}

		// Will check if start is being pressed
		if (kDown & KEY_START)
		{
			// Exit the program
			// Breaking out of the loop finally exits the services and dumps back to HB menu
			printf("[!!!] Exiting program...\n");
			break;
		}

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	httpcExit();
	gfxExit();
	return 0;
}
