#include "math.h"
#include <inttypes.h>

const int CLIENT_COUNT = 1;

unsigned int totalCompTime; //TODO - Needs to be initialized to 0 in setup()
unsigned int scheduleRound; //TODO - Needs to be initialized to 0 in setup()

typedef struct {
    long sourceIp;
    long destIp;
    uint16_t period;
    uint16_t size;
} header;

struct schedule{
    int hperiod;
    unsigned long startTime;
    unsigned int taskOrder[];
    unsigned long deadline[];
};

bool processPacketFromSource(TCPServer server, long source, long wallClockDeadline);
bool readFully(TCPClient c, uint8_t* buf, size_t len);
header bytesToHeader(uint8_t* b);
bool headersContainSource(header* headers, size_t headersLen, long source);
header* readAllHeaders(TCPServer server);

TCPServer server = TCPServer(1337);

void setup() {
    //double entropytest[5] = {2.0, 1.0, 3.0, 0.5, 5.0};
    //int taskNumbers[5] = {0,1,2,3,4};
    
    //sortHighestEntropy(entropytest, taskNumbers, 5, 5);
    //double taskEntro = calculateTaskEntropy(5, 10, 3);
    //double normalized = calculateNormalizedEntropy(taskEntro, 3);
    
    
    
    Serial.begin(9600);
    
    while(!Serial.isConnected()) Particle.process();
   
   
    server.begin();
    header* packetHeaders = readAllHeaders(server);
    
    Serial.printf("Returned\n\r");
    Serial.printf("Period: %d", packetHeaders[0].period);
    //Serial.printf("[0] = %f [1] = %f [2] = %f [3] = %f [4] = %f", entropytest[0], entropytest[1], entropytest[2], entropytest[3], entropytest[4]);
    //Serial.printf("\n\r[0] = %d [1] = %d [2] = %d [3] = %d [4] = %d", taskNumbers[0], taskNumbers[1], taskNumbers[2], taskNumbers[3], taskNumbers[4]);
    //Serial.println();
}

void loop() {

}


//Functions

void createSchedule(header* allPacketHeaders){
    //get all tasks' periods
    //compute hperiod
    int size = CLIENT_COUNT;
    uint16_t* periods = malloc(sizeof(uint16_t) * CLIENT_COUNT);
    for(int i = 0; i < size; i++){
            periods[i] = allPacketHeaders[i].period;
    }
}

//Returns true if schedulable, false if not.
bool checkSchedule(unsigned long startTime, unsigned long period, unsigned int compTime){
    unsigned long deadline = startTime + period;
    totalCompTime += compTime;
    
    if(totalCompTime > deadline){
        return false;
    }
    else{
        return true;
    }
}


//gets the wall clock (time since device has started) and returns the computed relative deadline
unsigned long getWallClockDeadline(unsigned int scheduleLength, int deadline, unsigned long startTime){
    
    // i * ScheduleTime + deadline
    unsigned long wallClock = (scheduleRound * scheduleLength) + deadline + startTime;
    return wallClock;
}

//Takes LOGbase2 of hperiod (lcm of all periods)  and multiplies it by the quotient of comptime/period
double calculateTaskEntropy(int period, int hperiod, int compTime){
    double entropy = (log2(hperiod)) * ((double)compTime / (double)period);
    return entropy;
}

//divides taskEntropy by compTime
double calculateNormalizedEntropy(double taskEntropy, int compTime){
    double normalizedEntropy = taskEntropy / compTime;
    return normalizedEntropy;
}

//Takes LCM and calculates Hsu
double calculateSingleTimeUnit(int hperiod){
    double valLog = log2(hperiod);
    double result = valLog / hperiod;
    return result;
}

//sorts the entropy array
//Takes in two arrays: entropy(either normalized or not) values and which task the value is associated with
void sortEntropy(double taskEntropy[], int taskNumbers[], int size1, int size2){
    for(int i = 0; i < size1; i++){
        
        int jMin = i;
        
        for(int j = i+1; j < size1; j++){
            
            if(taskEntropy[j] < taskEntropy[jMin]){
                jMin = j;
            }
        }
        
        if(jMin != i){
            //Uses array+i because we need to pass reference to array rather than value
            swapDouble(taskEntropy+i, taskEntropy+jMin);
            swapInt(taskNumbers+i, taskNumbers+jMin);
        }
    }
}




//HELPER FUNCTIONS

//Least Common Multiple - from https://www.geeksforgeeks.org/lcm-of-given-array-elements/
int leastCommonMultiple(int periods[], int size){
    
    int lcm = periods[0];
    for(int i = 1; i < size; i++){
        lcm = ((((periods[i] * lcm)) / (gcd(periods[i], lcm))));
    }
    return lcm;
}

//Greatest Common Denominator
int gcd(int a, int b) { 
    if (b == 0){
        return a;
    } 
    else{
        return gcd(b, (a % b));     
    }
    
}

//Swap helper function for doubles
void swapDouble(double *a, double *b){
    double temp = *a;
    *a = *b;
    *b = temp;
}

//swap helper function for ints
void swapInt(int *a, int *b){
    int temp = *a;
    *a = *b;
    *b = temp;
}



//Read all headers for setup phase
// server: A listening TCPServer
header* readAllHeaders(TCPServer server) {
    header* headers = (header*) malloc(sizeof(header) * CLIENT_COUNT);
    int clients = 0;
    
    while (clients < CLIENT_COUNT) {
        TCPClient c = server.available();
        uint8_t headerBytes[12];
        if (readFully(c, headerBytes, sizeof(header))) {
            header thisHeader = bytesToHeader(headerBytes);
            if (headersContainSource(headers, clients, thisHeader.sourceIp)) {
                Serial.printf("Encountered header from alread seen client.\n");
            } else {
                headers[clients] = thisHeader;
                clients++;
            }
        }
        c.stop();
    }
    return headers;
}

// Reads a desired amount from a stream and will block until enough data is available.
bool readFully(TCPClient c, uint8_t* buf, size_t len) {
    size_t read = 0;
    while (read < len) {
        int temp = c.read(&buf[read], len - read);
        if (temp < 0) {
            return false;
        }
        read += temp;
    }
    return true;
}

// Converts a byte array of 12 bytes to a header struct
header bytesToHeader(uint8_t* b) {
    header ret;
    ret.sourceIp = *((long*) &b[0]);
    ret.destIp = *((long*) &b[4]);
    ret.period = *((uint16_t*) &b[8]);
    ret.size = *((uint16_t*) &b[10]);
    return ret;
}

// Searches an array of header structs for one that contains the source IP.
bool headersContainSource(header* headers, size_t headersLen, long source) {
    for (int i = 0; i < headersLen; i++) {
        if (headers[i].sourceIp == source) {
            return true;
        }
    }
    return false;
}

// Takes a packet from a specific source and sends it to the destination.
// Closes any connections from clients that are not source.
// Times out and returns if wallClockDeadline is exceeded.
//  server: the TCP listener
//  source: the source IP to accept from
//  wallClockDeadline: the time according to millis() to return if the source does not connect.
// Returns: true if packet was received and sent.  False if timeout.
bool processPacketFromSource(TCPServer server, long source, long wallClockDeadline) {
    while (millis() <= wallClockDeadline) {
        TCPClient c = server.available();
        if (millis() > wallClockDeadline) {
            // Timeout
            return false;
        }
        if (!(c.remoteIP() == IPAddress(source))) {
            Serial.printf("Got connection from unexpected source: %s.  Closing connection.\n", IPAddress(source));
            c.stop();
        } else {
            Serial.printf("TODO: process packet\n");
            c.stop();
            return true;
        }
    }
    return false;
}