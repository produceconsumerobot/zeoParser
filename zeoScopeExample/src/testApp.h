#pragma once

#include "ofMain.h"
#include "zeoParser.h"
#include "ofxOscilloscope.h"

class testApp : public ofBaseApp{
	
	public:
		void setup();
		void update();
		void draw();
		void exit();
		void newRawData(bool & ready);
		void newSliceData(bool & ready);

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		ofTrueTypeFont		font;

		bool		bSendSerialMessage;			// a flag for sending serial
		char		bytesRead[3];				// data from serial, we will be trying to read 3
		char		bytesReadString[4];			// a string needs a null terminator, so we need 3 + 1 bytes
		int			nBytesRead;					// how much did we read?
		int			nTimesRead;					// how many times did we read?
		float		readTime;					// when did we last read?				
		
		ofSerial	serial;
		int			serialIterator;
		ZeoParser	zeo;
		ofxMultiScope scopeWin;

		// Oscilloscope data arrays	
		std::vector<std::vector<float>>  rawData;
		std::vector<std::vector<float>>  powerData;
		std::vector<std::vector<float>>  filtData;
		std::vector<std::vector<float>>	 sliceData;
};
