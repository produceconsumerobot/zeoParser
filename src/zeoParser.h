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
#include <vector>
#include <string>

using namespace std;

#define DATA(OFFSET, TYPE) *((TYPE*) (buffer + OFFSET))


class ZeoSlice {
public:
	static const int NUM_FREQS = 7;

		int number;
		int time;
		std::vector<int> power;
		int impendance;
		int sqi;
		int signal;
		int stage;
		int version;

		ZeoSlice();
		~ZeoSlice();
};



class ZeoParser {

	
public:
	static const int RAW_DATA_LEN = 128;
	static const int NUM_STAGES = 5;
	static const int NUM_FREQS = 7;
	static const string labels[NUM_FREQS];
	static const string stage[NUM_STAGES];

	ZeoParser();
	~ZeoParser();

	int parsePacket(char *buffer, int available, bool * spliceDataReady, bool * rawDataReady); 
	ZeoSlice getSlice();
	std::vector<float> getRawData();
	std::vector<float> getFiltAlignedRawData();
	std::vector<float> getFilteredData();
	std::vector<int> getPowerData();

private:
	ZeoSlice slice;
	std::vector<float> rawData;
	std::vector<float> rawBuffer;
	std::vector<float> filtAlignedRawData;
	std::vector<float> filteredData;
	int filtBufferLevel;
	bool printData;

	void process_slice();
	void process_waveform(char *buffer);
	void parse_inner_packet(char *buffer, int count, bool * spliceDataReady, bool * rawDataReady);
	void filterData();
	void Filter50Hz();
};

#endif