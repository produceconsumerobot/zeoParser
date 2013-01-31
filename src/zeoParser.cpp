//
//  zeo.h
//
//  Created by Sean Montgomery on 12/19/12.
//  Ported from http://www.ecstaticlyrics.com/secret/zeo_raw_data_parser.tar.bz2
//

#include "zeoParser.h"

ZeoSlice::ZeoSlice() {
		number = -1;
		power.assign(NUM_FREQS, -1);
		sqi = -1;
		time = -1;
		impendance = -1;
		signal = -1;
		stage = -1;
		version = -1;

}

ZeoSlice::~ZeoSlice() {
}

const string ZeoParser::labels[] = {" 2-4 ", " 4-8 ", " 8-13", "13-18", "18-21", "11-14", "30-50"};
const string ZeoParser::stage[] = {"Undefined", "Awake", "R.E.M.", "Light", "Deep"};

ZeoParser::ZeoParser() {

	printData = false;

	rawData.assign(RAW_DATA_LEN, 0.);
	rawBuffer.assign(RAW_DATA_LEN*2, 0.);
	filtAlignedRawData.assign(RAW_DATA_LEN, 0.);
	filteredData.assign(RAW_DATA_LEN, 0.);
}

ZeoParser::~ZeoParser() {
}

ZeoSlice ZeoParser::getSlice() {
	return slice;
}

std::vector<float> ZeoParser::getRawData(){
	return rawData;
}

std::vector<float> ZeoParser::getFiltAlignedRawData(){
	return filtAlignedRawData;
}

std::vector<float> ZeoParser::getFilteredData(){
#ifdef DEBUG_PRINT
	printf("ZeoParser::getFilteredData\n"); 
#endif
	return filteredData;
}


std::vector<int> ZeoParser::getPowerData(){
	return slice.power;
}

void ZeoParser::process_slice() {
	int i;
	if (printData) {
		printf("slice number %d:\n", slice.number);
		for (i = 0; i < NUM_FREQS; i++) {
			if (slice.power.at(i) >= 0) printf("    %s: %d\n", labels[i].c_str(), slice.power.at(i));
		};
		if (slice.sqi >= 0) printf("    SQI: %d\n", slice.sqi);
		//printf("    Date: %s", asctime(gmtime((time_t*) &slice.time)));
		if (slice.impendance >= 0) printf("    Impendance: %d\n", slice.impendance);
		//if (slice.signal >= 0) printf("    Signal: %s\n", slice.sqi ? "Good" : "Bad");
		if (slice.signal >= 0) printf("    Signal: %u\n", slice.signal);
		if (slice.stage >= 0 && slice.stage <= 4) printf("    Sleep Stage: %s\n", stage[slice.stage].c_str());
		if (slice.time > 0) printf("    RTC Time: %d\n", slice.time);
	}
}

void ZeoParser::process_waveform(char * buffer) {
	if (printData) {
		printf("We have waveform data: ");
	}
	for (int i = 0; i < RAW_DATA_LEN; i++) {
		//int raw = *((short*) (buffer + 2*i)); // may be necessary for xcode
		int raw = *((_int16*) (buffer + 2*i));
		//value = float(value*315)/0x8000    # convert to uV value FIX
		rawData.at(i) = ((float) raw * 315) / ((float) 0x8000);
		if (printData) {
			if (i<6) printf("%i, %f;", raw, rawData.at(i));
		}
	};
	if (printData) {
		printf("... Yay!\n");
	}

	filterData();
}

void ZeoParser::parse_inner_packet(char *buffer, int count, bool * spliceDataReady, bool * rawDataReady) {
	int type;
	int i;
	int event;
	int real;
	int imaginary;
	if (count < 1) return;


	type = DATA(0, unsigned char);
	buffer++; count--;

#define REQUIRES(X) if (count < X) { fprintf(stderr, "Inner packet type %u requires %d data bytes but only contains %d data bytes!\n", (unsigned char) buffer[-1], X, count); return; }

	if (type == 0x00) { // Events
		REQUIRES(4);
		event = DATA(0, unsigned int);
		if (printData) {
			if (event == 0x05) printf("Event: Start of night!\n");
			if (event == 0x07) printf("Event: Sleep onset!\n");
			if (event == 0x0E) printf("Event: Headband docked!\n");
			if (event == 0x0F) printf("Event: Headband undocked!\n");
			if (event == 0x10) printf("Event: Alarm off!\n");
			if (event == 0x11) printf("Event: Alarm snooze!\n");
			if (event == 0x13) printf("Event: Alarm play!\n");
			if (event == 0x15) printf("Event: End of night!\n");
			if (event == 0x24) printf("Event: New headband!\n");
		}
	} else if (type == 0x02) { // slice Boundary
		REQUIRES(4);
		if (DATA(0, unsigned int) == slice.number + 1) {
			process_slice();
			*spliceDataReady = true;
		} else {
			if (slice.number >= 0) {
				int missing = DATA(0, unsigned int) - slice.number;
				if (missing > 0) {
					fprintf(stderr, "Discarding %d slices due to missing slice boundary packets!\n", missing);
				} else {
					fprintf(stderr, "Discarding unknown number of slices due to Zeo reboot!\n");
				};
			};
		};
		slice.number = DATA(0, unsigned int);
		/*
		for (i = 0; i < NUM_FREQS; i++) {
			slice.power[i] = -1;
		};
		slice.sqi = -1;
		slice.time = -1;
		slice.impendance = -1;
		slice.signal = -1;
		slice.stage = -1;
		slice.version = -1;
		*/
	} else if (type == 0x03) { // Protocol Version
		REQUIRES(4);
		slice.version = DATA(0, unsigned int);
		// If the version is different, does it matter?
		// Depends on how similar the newer versions are.
	} else if (type == 0x80) { // Waveform Data
		REQUIRES(256);
		process_waveform(buffer);
		*rawDataReady = true;
	} else if (type == 0x83) { // Frequency Bins
		REQUIRES(14);
		for (i = 0; i < NUM_FREQS; i++) {
			slice.power.at(i) = DATA(2 * i, unsigned short);
		};
	} else if (type == 0x84) { // SQI
		REQUIRES(4);
		slice.sqi = DATA(0, unsigned int);
	} else if (type == 0x8A) { // Time
		REQUIRES(4);
		slice.time = DATA(0, unsigned int);
	} else if (type == 0x97) { // Impendance
		REQUIRES(4);
		real = DATA(0, unsigned short) - 0x8000;
		imaginary = DATA(2, unsigned short) - 0x8000;
		slice.impendance = (int) sqrt((double)(real * real + imaginary * imaginary));
	} else if (type == 0x9C) { // Good / Bad Signal
		REQUIRES(4);
		slice.signal = DATA(0, unsigned int);
	} else if (type == 0x9D) { // Sleep Stage
		REQUIRES(4);
		slice.stage = DATA(0, unsigned int);
	} else {
		fprintf(stderr, "Unknown zeo message:");
		for (i = 0; i < count; i++) {
			fprintf(stderr, " %02x", (unsigned char) buffer[i]);
		};
		fprintf(stderr, "\n");
	};

}

int ZeoParser::parsePacket(char *buffer, int available, bool * spliceDataReady, bool * rawDataReady) {

	while (available >= 11) {
		int data_length;
		int packet_size;
		int sum;
		int i;

		// First bytes must be characters 'A4'
		if ((DATA(0, unsigned char) != 'A')  || (DATA(1, unsigned char) != '4')) {
			buffer++; available--; continue;
		};

		// Check that message length matches inverted message length.

		if (DATA(3, unsigned short) != (0xFFFF & ~DATA(5, unsigned short))) {
			buffer++; available--; continue;
		};

		// Also discard unusually long messages...

		if (DATA(3, unsigned short) > 257) {
			fprintf(stderr, "Discarding unusually long inner packet (%d bytes).\n", DATA(3, unsigned short));
			buffer++; available--; continue;
		};

		data_length = DATA(3, unsigned short);
		packet_size = 11 + data_length;

		// Check that we have full message...

		if (available < packet_size) return available;

		// Verify checksum...

		sum = 0;
		for (i = 0; i < data_length; i++) {
			sum += DATA(11 + i, unsigned char);
		};
		sum %= 256;

		if (sum != DATA(2, unsigned char)) {
			fprintf(stderr, "Outer packet failed checksum, but I'm not sure I care...\n");
			buffer++; available--; continue;
		};

		// Get timestamp info
		int timestamp_lowbyte = DATA(7, unsigned char);
		float timestamp_subsec = ((float) DATA(8, unsigned short)) / 65535.0;
		int timestamp = 0;

		// Get the sequence number
		int seqnum = DATA(10, unsigned char);

		//printf("Outer packet dump:");
		//for (int i = 0; i < packet_size; i++) {
		//  printf(" %02x", (unsigned char) buffer[i]);
		//};
		//printf("\n");

		parse_inner_packet(buffer + 11, data_length, spliceDataReady, rawDataReady);

		buffer += packet_size;
		available -= packet_size;

	};

	return available;

}

void ZeoParser::filterData() {

	// shift the old data to the beginning of the filtBuffer
	// TODO This could be made faster
	for (int i=0; i<RAW_DATA_LEN; i++) {
		rawBuffer.at(i) = rawBuffer.at(i + RAW_DATA_LEN);
	}
	//copy( rawBuffer + RAW_DATA_LEN, rawBuffer + (2*RAW_DATA_LEN), rawBuffer);

	// copy the new data to the end of the filtBuffer
	//copy( rawData, rawData + ZEO_RAW_DATA_LEN, filtBuffer + ZEO_RAW_DATA_LEN);
	for (int i=0; i<RAW_DATA_LEN; i++) {
		rawBuffer.at(i + RAW_DATA_LEN) = rawData.at(i);
	}

	//Filter50Hz(rawBuffer, 2*ZEO_RAW_DATA_LEN, filteredData);
	Filter50Hz();

}

void ZeoParser::Filter50Hz() {
	// 50Hz Filter based on http://blog.myzeo.com/zeo-raw-data-library-free-your-mind/
	// Convolution math based on http://www.phys.uu.nl/~haque/computing/WPark_recipes_in_python.html

	const int filterLen = 51;
	float array_filter[filterLen] =
	{
		0.0032f, 0.0063f, -0.0088f, -0.0000f, 0.0100f, -0.0082f, -0.0047f, 0.0132f, -0.0054f, -0.0108f, 0.0151f, 0.0000f,
		-0.0177f, 0.0147f, 0.0087f, -0.0248f, 0.0105f, 0.0215f, -0.0315f, -0.0000f, 0.0411f, -0.0369f, -0.0241f, 0.0790f,
		-0.0404f, -0.1123f, 0.2939f, 0.6250f, 0.2939f, -0.1123f, -0.0404f, 0.0790f, -0.0241f, -0.0369f, 0.0411f, -0.0000f,
		-0.0315f, 0.0215f, 0.0105f, -0.0248f, 0.0087f, 0.0147f, -0.0177f, 0.0000f, 0.0151f, -0.0108f, -0.0054f, 0.0132f,
		-0.0047f, -0.0082f, 0.0100f                            
	};
	std::vector<float> filter(array_filter, array_filter + filterLen);

	//int filterLen = filter.Length;
	int len = RAW_DATA_LEN*2;
	int n = len + filterLen - 1;

	//float * tempData;
	//tempData = new float[n];
	std::vector<float> tempData(n);
	//ChannelData[] filteredSignal = new ChannelData[n];
	for (int i = 0; i < n; i++)
	{
		float t = 0;
		int lower = max(0, i - (filterLen - 1));
		int upper = min(len - 1, i);

		for (int j = lower; j <= upper; j++)
		{
			t = t + (rawBuffer.at(j) * filter.at(i - j));
		}

		//filteredSignal[i] = new ChannelData (1);
		tempData.at(i) = t;
	}

	// Copy tempData into filteredData
	// Exclude the size of the filter from the leading edge tempData to
	// avoid data that hasn't been convolved by the entire filter
	// take the most recent ZEO_RAW_DATA_LEN after that 
	// TODO This could be made faster
	for (int i=0; i<len/2; i++) {
		filteredData.at(i) = tempData.at(i + n-filterLen-(len/2));
	}
	//copy(tempData + n-filterLen-(len/2), tempData + n-filterLen, filteredData);

	// Copy rawBuffer into filtAlignedRawData that's aligned with the filteredData
	// Exclude 1/2 of the filter length from the leading edge  
	// take the most recent ZEO_RAW_DATA_LEN after that
	// TODO This could be made faster
	for (int i=0; i<len/2; i++) {
		filtAlignedRawData.at(i) = rawBuffer.at(i + len-filterLen/2-(len/2));
	}
	//copy(rawBuffer + len-filterLen/2-(len/2), rawBuffer + len-filterLen/2, filtAlignedRawData);
		
	//tempData + n-(filterLen/2)-(len/2), tempData + n-(filterLen/2), filteredData);

	//delete tempData;
	//return filteredSignal;
}

/*
void ZeoParser::Filter50Hz(float * dataBuffer, int bufferLen, float * filteredData) {
            // 50Hz Filter based on http://blog.myzeo.com/zeo-raw-data-library-free-your-mind/
            // Convolution math based on http://www.phys.uu.nl/~haque/computing/WPark_recipes_in_python.html
            
			const int filterLen = 51;
			float filter[filterLen] =
            {
                0.0032f, 0.0063f, -0.0088f, -0.0000f, 0.0100f, -0.0082f, -0.0047f, 0.0132f, -0.0054f, -0.0108f, 0.0151f, 0.0000f,
                -0.0177f, 0.0147f, 0.0087f, -0.0248f, 0.0105f, 0.0215f, -0.0315f, -0.0000f, 0.0411f, -0.0369f, -0.0241f, 0.0790f,
                -0.0404f, -0.1123f, 0.2939f, 0.6250f, 0.2939f, -0.1123f, -0.0404f, 0.0790f, -0.0241f, -0.0369f, 0.0411f, -0.0000f,
                -0.0315f, 0.0215f, 0.0105f, -0.0248f, 0.0087f, 0.0147f, -0.0177f, 0.0000f, 0.0151f, -0.0108f, -0.0054f, 0.0132f,
                -0.0047f, -0.0082f, 0.0100f                            
            };

            //int filterLen = filter.Length;
            int len = bufferLen;
            int n = len + filterLen - 1;

			float * tempData;
			tempData = new float[n];
            //ChannelData[] filteredSignal = new ChannelData[n];
            for (int i = 0; i < n; i++)
            {
                float t = 0;
                int lower = max(0, i - (filterLen - 1));
                int upper = min(len - 1, i);

                for (int j = lower; j <= upper; j++)
                {
                    t = t + (dataBuffer[j] * filter[i - j]);
                }

                //filteredSignal[i] = new ChannelData (1);
                tempData[i] = t;
            }

			copy(tempData + n-(filterLen/2)-(bufferLen/2), tempData + n-(filterLen/2), filteredData);

			delete tempData;
            //return filteredSignal;
        }
		*/