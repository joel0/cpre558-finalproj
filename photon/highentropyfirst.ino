#include "math.h"



void setup() {
    double entropytest[5] = {2.0, 1.0, 3.0, 0.5, 5.0};
    int taskNumbers[5] = {0,1,2,3,4};
    
    //sortHighestEntropy(entropytest, taskNumbers, 5, 5);
    double taskEntro = calculateTaskEntropy(5, 10, 3);
    double normalized = calculateNormalizedEntropy(taskEntro, 3);
    Serial.begin(9600);
    
    while(!Serial.isConnected()) Particle.process();
   
    Serial.printf("CalculateTaskEntropy: %f \n\r Normalized: %f",taskEntro, normalized);
    //Serial.printf("[0] = %f [1] = %f [2] = %f [3] = %f [4] = %f", entropytest[0], entropytest[1], entropytest[2], entropytest[3], entropytest[4]);
    //Serial.printf("\n\r[0] = %d [1] = %d [2] = %d [3] = %d [4] = %d", taskNumbers[0], taskNumbers[1], taskNumbers[2], taskNumbers[3], taskNumbers[4]);
    //Serial.println();
}

void loop() {

}


//Functions


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