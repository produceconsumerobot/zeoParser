#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup(){
	ofSetVerticalSync(true);
	
	bSendSerialMessage = false;
	ofBackground(255);	
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	// Setup oscilloscopes
	int nScopes = 3;
	ofPoint min = ofPoint(0., 10.);
	ofPoint max = ofPoint(ofGetWindowSize().x, ofGetWindowSize().y-10);
	scopeWin = ofxMultiScope(nScopes, min, max);
	int rawTimeWindow = 15;
	int powerTimeWindow = 300;
	{
		const int nVariables = 1;
		ofColor colors[nVariables] = {ofColor(0,200,0)};
		string names[nVariables] = {"Filt EEG"};
		scopeWin.scopes[0].setup(rawTimeWindow, zeo.RAW_DATA_LEN, names, colors, nVariables, 7., 0.);
	}
	{
		const int nVariables = zeo.NUM_FREQS;
		ofColor colors[nVariables] = {ofColor(200,0,0), ofColor(0,200,0), ofColor(0,0,200), 
			ofColor(0,200,200), ofColor(200,0,200), ofColor(200,200,0), ofColor(100,100,100)};

		string names[nVariables];
		for (int i=0; i<nVariables; i++) {
			names[i] = zeo.labels[i];
		}
		scopeWin.scopes[1].setup(powerTimeWindow, 1, names, colors, nVariables, 0.07, -350.);
	}
	{
		const int nVariables = 3;
		ofColor colors[nVariables] = {ofColor(200,0,0), ofColor(0,200,0), ofColor(0,0,200)};
		string names[nVariables] = {"Impedance", "SQI", "Signal"};
		scopeWin.scopes[2].setup(powerTimeWindow, 1, names, colors, nVariables, 0.5, -350.);
	}
	
	// Initialize Oscilloscope data arrays	
	rawData = new float*[1];
	rawData[0] = new float[zeo.RAW_DATA_LEN];
	powerData = new float*[zeo.NUM_FREQS];
	for (int i=0; i<zeo.NUM_FREQS; i++) {
		powerData[i] = new float[1];
	}
	sliceData = new float*[3];
	for (int i=0; i<3; i++) {
		sliceData[i] = new float[1];
	}

	// Setup Zeo
	serial.listDevices();
	serial.setup("\\\\.\\COM26", 38400);
	//zeo.setupSerial("\\\\.\\COM26", 38400);
	//ofAddListener(zeo.rawDataReady,this,&testApp::newRawData);
	//ofAddListener(zeo.sliceDataReady,this,&testApp::newSliceData);
	//zeo.startThread(true, false);
}

//--------------------------------------------------------------
void testApp::newSliceData(bool & ready){
	printf("SLICE READY\n");

	/*
	int zeoData[ZEO_NUM_LABELS];
	zeo.getPowerData(zeoData);

	for (int i=0; i<ZEO_NUM_LABELS; i++) {
		powerData[i][0] = (float) zeoData[i];
		printf("    %s: %f, %i\n", zeo.labels[i], powerData[i], zeoData[i]);
	}
	scopeWin.scopes[1].updateData(powerData, 1);
	*/

	ZeoSlice zeoSlice;
	zeo.getSlice(&zeoSlice);
	for (int i=0; i<zeo.NUM_FREQS; i++) {
		powerData[i][0] = (float) zeoSlice.power[i];
		printf("    %s: %f, %i\n", zeo.labels[i], powerData[i][0], zeoSlice.power[i]);
	}
	scopeWin.scopes[1].updateData(powerData, 1);

	sliceData[0][0] = zeoSlice.impendance;
	sliceData[1][0] = zeoSlice.sqi*40; // multiply for display
	sliceData[2][0] = zeoSlice.signal*1500; // multiply for display
	scopeWin.scopes[2].updateData(sliceData, 1);
}

//--------------------------------------------------------------
void testApp::newRawData(bool & ready){
	printf("RAW READY\n");
	
	float zeoData[zeo.RAW_DATA_LEN];

	zeo.getFilteredData(zeoData);

	for (int i=0; i<zeo.RAW_DATA_LEN; i++) {
		rawData[0][i] = zeoData[i];
	}
	printf("    Raw Data:");
	for (int i=0; i<6; i++) {
		printf(" %f, %f;", rawData[0][i], zeoData[i]);
	}
	printf("\n");
	scopeWin.scopes[0].updateData(rawData, zeo.RAW_DATA_LEN);
}

//--------------------------------------------------------------
void testApp::update() {
	const int BUFFER_SIZE = 4096;
	static char buffer[BUFFER_SIZE];
	static int available = 0;

	bool spliceDataReady = false;
	bool rawDataReady = false;

	int count = serial.readBytes((unsigned char *) buffer + available, BUFFER_SIZE - available);

	if (count < 0) {
		fprintf(stderr, "Error reading data!\n");
		exit();
	};
	if (count > 0) {
		available += count;

		int remaining = zeo.parsePacket(buffer, available, &spliceDataReady, &rawDataReady);
		
		memmove(buffer, buffer + available - remaining, remaining);
		available = remaining;
	}

	if (spliceDataReady) newSliceData(spliceDataReady);
	if (rawDataReady) newRawData(rawDataReady);

}

//--------------------------------------------------------------
void testApp::draw(){
	printf("draw()\n");
	scopeWin.plot();
}

//--------------------------------------------------------------
void testApp::exit(){
	//exit();
	for (int i=0; i<zeo.NUM_FREQS; i++) {
		delete powerData[i];
	}
	delete powerData;

	for (int i=0; i<3; i++) {
		delete sliceData[i];
	}
	delete sliceData;

	//delete rawData[0];
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){ 
	
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){ 
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
	
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 
	
}

