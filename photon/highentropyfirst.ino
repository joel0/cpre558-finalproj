#include "math.h"
#include <inttypes.h>

#define htonl(A) ((((uint32_t)(A) & 0xff000000) >> 24) | \
(((uint32_t)(A) & 0x00ff0000) >> 8) | \
(((uint32_t)(A) & 0x0000ff00) << 8) | \
(((uint32_t)(A) & 0x000000ff) << 24))

const int CLIENT_COUNT = 1;

unsigned int totalCompTime; //TODO - Needs to be initialized to 0 in setup()
unsigned int scheduleRound; //TODO - Needs to be initialized to 0 in setup()

typedef struct {
    long sourceIp;
    long destIp;
    uint16_t period;
    uint16_t size;
} header;

typedef struct {
    unsigned int hPeriod;
    unsigned long startTime;
    unsigned int taskOrder[CLIENT_COUNT];
    double taskEntropy[CLIENT_COUNT];
    double normalizedTaskEntropy[CLIENT_COUNT];
    unsigned long deadline[CLIENT_COUNT];
    unsigned int finalOrder[CLIENT_COUNT];
} schedule;

//Server Creation
TCPServer server = TCPServer(1337);
TCPClient client;


//*******************************************PROTOTYPES***************************************
bool processPacketFromSource(long source, long wallClockDeadline);
bool readFully(uint8_t* buf, size_t len);
header bytesToHeader(uint8_t* b);
bool headersContainSource(header* headers, size_t headersLen, long source);
header* readAllHeaders();
void sendPacket(header h, uint8_t* payload);
void scheduleInit(schedule* scheduleStruct);
double calculateNormalizedEntropy(double taskEntropy, unsigned short compTime);
double calculateTaskEntropy(unsigned short period, unsigned short hperiod, unsigned short compTime);
unsigned long checkSchedule(unsigned long startTime, unsigned short period, unsigned short compTime);
//********************************************************************************************


void setup() {
    //double entropytest[5] = {2.0, 1.0, 3.0, 0.5, 5.0};
    //int taskNumbers[5] = {0,1,2,3,4};
    
    //sortHighestEntropy(entropytest, taskNumbers, 5, 5);
    //double taskEntro = calculateTaskEntropy(5, 10, 3);
    //double normalized = calculateNormalizedEntropy(taskEntro, 3);
    
    
    
    Serial.begin(9600);
    
    while(!Serial.isConnected()) Particle.process();
   
    Serial.printf("Serial connected!\n\r");
    //create schedule & init values;
    scheduleRound = 1;
    totalCompTime = 0;
    schedule hSchedule;
    scheduleInit(&hSchedule);
    Serial.printf("Schedule initialized...\n\r");
    server.begin();
    Serial.printf("Begin. Server started...\n\r");
    header* packetHeaders = readAllHeaders();
    Serial.printf("Returned\n\r");
    Serial.printf("Period: %d\n\r", packetHeaders[0].period);
    //Serial.printf("[0] = %f [1] = %f [2] = %f [3] = %f [4] = %f", entropytest[0], entropytest[1], entropytest[2], entropytest[3], entropytest[4]);
    //Serial.printf("\n\r[0] = %d [1] = %d [2] = %d [3] = %d [4] = %d", taskNumbers[0], taskNumbers[1], taskNumbers[2], taskNumbers[3], taskNumbers[4]);
    //Serial.println();
    long src = packetHeaders[0].sourceIp;
    long startTime = millis();
    unsigned long deadln = getWallClockDeadline(10000, packetHeaders[0].period, startTime);
    //server.begin();
    Serial.printf("Begin. Server started...\n\r");
    Serial.printf("Preparing to processPacketFromSource\n\r");
    bool sent = processPacketFromSource(src, deadln);
    //packetHeaders = readAllHeaders();
    
    //bool sent = false;
    Serial.printf("\n\rDeadlineWallClk: %ld\n\r", deadln);
    if(sent){
        Serial.printf("Sent.");    
    }
    else{
        Serial.printf("Sent == false.");
    }
    
    
}

void loop() {

}


//Functions

//Fills in various arrays for the schedule struct
void createSchedule(header* allPacketHeaders, schedule* HEFschedule){
    //get all tasks' periods
    //compute hperiod
    int size = CLIENT_COUNT;
    unsigned short periods[CLIENT_COUNT];
    for(int i = 0; i < size; i++){
        periods[i] = allPacketHeaders[i].period;
    }
    HEFschedule->hPeriod = leastCommonMultiple(periods, CLIENT_COUNT);
    
    for(int i = 0; i < size; i++){
        HEFschedule->taskEntropy[i] = calculateTaskEntropy(allPacketHeaders[i].period, HEFschedule->hPeriod, allPacketHeaders[i].size);
        HEFschedule->normalizedTaskEntropy[i] = calculateNormalizedEntropy(HEFschedule->taskEntropy[i], allPacketHeaders[i].size);
    }
    //sorts in acending order
    sortEntropy(HEFschedule->normalizedTaskEntropy, HEFschedule->taskOrder, CLIENT_COUNT, CLIENT_COUNT);
    
    
    //go in reverse order becuase it is "highest entropy first"
    //check if schedule/taskOrder is feasible
    unsigned long currentTime = millis();
    int j =0;
    for(int i = size -1; i >= 0; i--){
        header task = allPacketHeaders[HEFschedule->taskOrder[i]];
        long deadline = checkSchedule(currentTime, task.period, task.size);
        if(deadline != 0){
            HEFschedule->finalOrder[j] = HEFschedule->taskOrder[i];
            HEFschedule->deadline[j++] = deadline;
        }
        else{
            //skip
        }
    }
    //Should have a finalorder array -> holds index of header in allPacketHeaders array
    //deadline holds deadline of the header at the index of final order
}

//Returns deadline if schedulable, 0 if not.
unsigned long checkSchedule(unsigned long startTime, unsigned short period, unsigned short compTime){
    unsigned long deadline = startTime + period;
    totalCompTime += compTime;
    
    if(totalCompTime > deadline){
        totalCompTime -= compTime; //Subract off the total comp time because packet will be discarded
        return 0;
    }
    else{
        return deadline;
    }
}


//gets the wall clock (time since device has started) and returns the computed relative deadline
unsigned long getWallClockDeadline(unsigned int scheduleLength, int deadline, unsigned long startTime){
    
    // i * ScheduleTime + deadline
    unsigned long wallClock = (scheduleRound * scheduleLength) + deadline + startTime;
    return wallClock;
}

//Takes LOGbase2 of hperiod (lcm of all periods)  and multiplies it by the quotient of comptime/period
double calculateTaskEntropy(unsigned short period, unsigned short hperiod, unsigned short compTime){
    double entropy = (log2(hperiod)) * ((double)compTime / (double)period);
    return entropy;
}

//divides taskEntropy by compTime
double calculateNormalizedEntropy(double taskEntropy, unsigned short compTime){
    double normalizedEntropy = taskEntropy / compTime;
    return normalizedEntropy;
}

//Takes LCM and calculates Hsu
double calculateSingleTimeUnit(int hperiod){
    double valLog = log2(hperiod);
    double result = valLog / hperiod;
    return result;
}

//sorts the entropy array (selection sort)
//Takes in two arrays: entropy(either normalized or not) values and which task the value is associated with
void sortEntropy(double taskEntropy[], unsigned int taskNumbers[], int size1, int size2){
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

void sortPeriod(unsigned short taskPeriod[], unsigned int taskNumbers[], int size1, int size2){
    for(int i = 0; i < size1; i++){
        
        int jMin = i;
        
        for(int j = i+1; j < size1; j++){
            
            if(taskPeriod[j] < taskPeriod[jMin]){
                jMin = j;
            }
        }
        
        if(jMin != i){
            //Uses array+i because we need to pass reference to array rather than value
            swapShort(taskPeriod+i, taskPeriod+jMin);
            swapInt(taskNumbers+i, taskNumbers+jMin);
        }
    }
}




//HELPER FUNCTIONS


void scheduleInit(schedule* scheduleStruct){
    scheduleStruct->hPeriod = 0;
    scheduleStruct->startTime = 0;
    
    for(unsigned int i = 0; i < CLIENT_COUNT; i++){
        scheduleStruct->taskOrder[i] = i;
    }
}
//Least Common Multiple - from https://www.geeksforgeeks.org/lcm-of-given-array-elements/
unsigned int leastCommonMultiple(unsigned short periods[], int size){
    
    unsigned int lcm = periods[0];
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
void swapInt(unsigned int *a, unsigned int *b){
    int temp = *a;
    *a = *b;
    *b = temp;
}

//swap helper function for shorts
void swapShort(unsigned short *a, unsigned short *b){
    int temp = *a;
    *a = *b;
    *b = temp;
}

//Read all headers for setup phase
// server: A listening TCPServer
header* readAllHeaders() {
    header* headers = (header*) malloc(sizeof(header) * CLIENT_COUNT);
    int clients = 0;
    
    while (clients < CLIENT_COUNT) {
        Serial.printf("(1) Waiting for client...\n\r");
        //TCPClient c;
        do{
            client = server.available();
            //delay(100);
        }while(!client.connected());
        Serial.printf("Connected...\n\r");
        uint8_t headerBytes[12];
        if (readFully(headerBytes, sizeof(header))) {
            header thisHeader = bytesToHeader(headerBytes);
            if (headersContainSource(headers, clients, thisHeader.sourceIp)) {
                Serial.printf("Encountered header from alread seen client.\n\r");
            } else {
                Serial.printf("Header read.\n\r");
                headers[clients] = thisHeader;
                clients++;
            }
        }
        client.stop();
    }
    return headers;
}

// Reads a desired amount from a stream and will block until enough data is available.
bool readFully(uint8_t* buf, size_t len) {
    size_t read = 0;
    while (read < len) {
        int temp = client.read(&buf[read], len - read);
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
    ret.sourceIp = htonl(*((long*) &b[0]));
    ret.destIp = htonl(*((long*) &b[4]));
    ret.period = *((uint16_t*) &b[8]);
    ret.size = *((uint16_t*) &b[10]);
    return ret;
}

// Converts a header to a byte array of 12 bytes
uint8_t* headerToBytes(header h) {
    uint8_t* ret = (uint8_t*) malloc(12);
    *((long*) &ret[0]) = h.sourceIp;
    *((long*) &ret[4]) = h.destIp;
    *((uint16_t*) &ret[8]) = h.period;
    *((uint16_t*) &ret[10]) = h.size;
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
bool processPacketFromSource(long source, long wallClockDeadline) {
    while (millis() <= wallClockDeadline) {
        //TCPClient c;
        Serial.printf("(2) Attempting to connect...\n\r");
        do{
            client = server.available();   
            //delay(100);
        }while(!client.connected());
        Serial.printf("Re-connected...\n\r");
        if (millis() > wallClockDeadline) {
            // Timeout
            Serial.printf("Timeout.\n\r");
            return false;
        }
        if (!(client.remoteIP() == source)) {
            Serial.printf("Got connection from unexpected source:\n\r");
            Serial.printf("Client Address:\n\r");
            Serial.println(client.remoteIP());
            Serial.printf("Server Address: \n\r");
            Serial.println(IPAddress(source));
            Serial.printf("Closing Connection.\n\r");
            client.stop();
        } else {
            Serial.printf("Processing packet\n\r");
            readPacket();
            client.stop();
            return true;
        }
    }
    return false;
}

void readPacket() {
    uint8_t headerBytes[12];
    if (readFully(headerBytes, sizeof(header))) {
        header h = bytesToHeader(headerBytes);
        uint8_t payload[h.size];
        if (!readFully(payload, h.size)) {
            Serial.printf("Problem reading full packet payload from client\n\r");
            return;
        } else {
            sendPacket(h, payload);
        }
    }
    Serial.printf("Else.\n\r");
}

void sendPacket(header h, uint8_t* payload) {
    IPAddress destinationIP = IPAddress(h.destIp);
    TCPClient c;
    if (!c.connect(destinationIP, 1337)) {
        Serial.printf("Problem connecting to server.\n\r");
    } else {
        Serial.printf("Connected outgoing.\n\r");
        uint8_t* headerBytes = headerToBytes(h);
        c.write(headerBytes, 12);
        Serial.printf("Wrote header.\n\r");
        free(headerBytes);
        c.write(payload, h.size);
        Serial.printf("Wrote payload.\n\r");
        c.stop();
    }
}