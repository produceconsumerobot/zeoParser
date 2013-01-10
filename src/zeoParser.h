//
//  zeo.h
//
//  Created by Sean Montgomery on 12/19/12.
//  Ported from http://www.ecstaticlyrics.com/secret/zeo_raw_data_parser.tar.bz2
//

#ifndef _ZEO_PARSER
#define _ZEO_PARSER

//#include "ofMain.h" 
#include <stdio.h>
#include <math.h>
#include <algorithm> // needed for max, min, copy

using namespace std;

#define DATA(OFFSET, TYPE) *((TYPE*) (buffer + OFFSET))


class ZeoSlice {
public:
	static const int NUM_FREQS = 7;

		int number;
		int time;
		int power[NUM_FREQS];
		int impendance;
		int sqi;
		int signal;
		int stage;
		int version;

		ZeoSlice();
		~ZeoSlice();
		void copyTo(ZeoSlice * slice);
};

class ZeoParser {

	
public:
	static const int RAW_DATA_LEN = 128;
	static const int NUM_STAGES = 5;
	static const int NUM_FREQS = 7;
	//const static char * labels[ZEO_NUM_LABELS];
	//const static char * stage[ZEO_NUM_STAGES];
	static char * labels[NUM_FREQS];
	static char * stage[NUM_STAGES];

	//static string labels = {" 2-4 ", " 4-8 ", " 8-13", "13-18", "18-21", "11-14", "30-50"};
	ZeoParser();
	~ZeoParser();

	int parsePacket(char *buffer, int available, bool * spliceDataReady, bool * rawDataReady); 
	void getSlice(ZeoSlice * data);
	void getRawData(float * data);
	void getFiltAlignedRawData(float * data);
	void getFilteredData(float * data);
	void getPowerData(int * data);

private:
	ZeoSlice slice;
	float rawData[RAW_DATA_LEN];
	float rawBuffer[RAW_DATA_LEN*2];
	float filtAlignedRawData[RAW_DATA_LEN];
	float filteredData[RAW_DATA_LEN];
	int filtBufferLevel;

	void process_slice();
	void process_waveform(char *buffer);
	void parse_inner_packet(char *buffer, int count, bool * spliceDataReady, bool * rawDataReady);
	void filterData();
	void Filter50Hz();
};

#endif