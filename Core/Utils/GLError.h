#ifndef _GLERROR_H
#define _GLERROR_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

using namespace std;

class GLError
{
public:

	static void clearError();
	
	/**
	 * 	@brief Retrieving the last error set in the openGL context error flag
	 * 	@param userMarkUp	simple string to help identify the error position (defined by the dev)
	 *	@return return the message if something is found, else return the userMarkUp string
	 */
	static string getLastErrorBase(string userMarkUp);
	
	static GLenum getLastErrorType();
	static void sendDebugNotification(string message, bool testToSend);

	// for simple logging
	static void displayErrorLog();

	// Callbacks
	static void enableCallback();
	static void disableCallback();


};

#endif
