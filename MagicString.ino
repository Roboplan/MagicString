#include <SPI.h>

#define TRIGGER_PIN		9  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN		8  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define Speaker_PIN		6  
#define Button_PIN		4

#define SamplesPerMeasurement 10
#define NumOfZeroCancelMeasurment 4

const int Notes[] =  //C3 to B7 and zero(noTone)
{//C  D,  E,  F, G,     A,    B
	262, 294, 330, 349, 392, 440, 494,
	523, 587, 659, 698, 784, 880, 988,
1047}; //49 - 50
// C   C#    D    D#   E     F    F#   G   G#    A   A#    B


int samples[SamplesPerMeasurement];
int NumOfZero = 0;
int sampleAvg = 0;
int max1,max2;
int min1,min2;

char POWER_CTL = 0x2D;    //Power Control Register
char DATA_FORMAT = 0x31;
char DATAX0 = 0x32;   //X-Axis Data 0
char DATAX1 = 0x33;   //X-Axis Data 1
char DATAY0 = 0x34;   //Y-Axis Data 0
char DATAY1 = 0x35;   //Y-Axis Data 1
char DATAZ0 = 0x36;   //Z-Axis Data 0
char DATAZ1 = 0x37;   //Z-Axis Data 1

int x,y,z;
//This buffer will hold values read from the ADXL345 registers.
char values[10];
int CS=10;
int avg = 0;

void setup() {
	pinMode(TRIGGER_PIN, OUTPUT);
	pinMode(Speaker_PIN, OUTPUT);
	pinMode(ECHO_PIN, INPUT);
	pinMode(Button_PIN,INPUT_PULLUP);
	pinMode(CS, OUTPUT);
	
	Serial.begin(9600); // Open serial monitor at 115200 baud to see ping results.
	Serial.println("Welcome to Magic string terminal");
	
	
	SPI.begin();
	SPI.setDataMode(SPI_MODE3);
	digitalWrite(CS, HIGH);
	//Put the ADXL345 into +/- 4G range by writing the value 0x01 to the DATA_FORMAT register.
	writeRegister(DATA_FORMAT, 0x01);
	//Put the ADXL345 into Measurement Mode by writing 0x08 to the POWER_CTL register.
	writeRegister(POWER_CTL, 0x08);  //Measurement mode
	Serial.println("start avg process");
	long sum = 0;
	for(int i =0; i<100;i++)
	{
		readRegister(DATAX0, 6, values);

		//The ADXL345 gives 10-bit acceleration values, but they are stored as bytes (8-bits). To get the full value, two bytes must be combined for each axis.
		//The X value is stored in values[0] and values[1].
		x = ((int)values[1]<<8)|(int)values[0];
		//The Y value is stored in values[2] and values[3].
		y = ((int)values[3]<<8)|(int)values[2];
		//The Z value is stored in values[4] and values[5].
		z = ((int)values[5]<<8)|(int)values[4];
		sum += abs(x) + abs(y) + abs(z);
		Serial.println(sum);
	}
	avg = (int)(sum/100);
	Serial.print("process end. avg - ");
	Serial.println(avg);
}



void loop()
{
		
		
	int dis = getFilteredDistance();
	
	if(dis > 0 && dis < 2000)
	{
		int note = map(dis,0,2000,0,14);
		readRegister(DATAX0, 6, values);

		//The ADXL345 gives 10-bit acceleration values, but they are stored as bytes (8-bits). To get the full value, two bytes must be combined for each axis.
		//The X value is stored in values[0] and values[1].
		x = ((int)values[1]<<8)|(int)values[0];
		//The Y value is stored in values[2] and values[3].
		y = ((int)values[3]<<8)|(int)values[2];
		//The Z value is stored in values[4] and values[5].
		z = ((int)values[5]<<8)|(int)values[4];
		
		//Print the results to the terminal.
		long sum = abs(x) + abs(y) + abs(z) - avg;
		Serial.println(sum);
		if(sum > 100)
			tone(Speaker_PIN,Notes[note]);
		else
		{
			noTone(Speaker_PIN);
			digitalWrite(Speaker_PIN,LOW);
		}
		//Serial.println(note);
	}
	else
	{
		noTone(Speaker_PIN);
		digitalWrite(Speaker_PIN,LOW);
		Serial.println("-1");
	}
	delay(5);
}
	
/*
void loop(){
	//Reading 6 bytes of data starting at register DATAX0 will retrieve the x,y and z acceleration values from the ADXL345.
	//The results of the read operation will get stored to the values[] buffer.
	readRegister(DATAX0, 6, values);

	//The ADXL345 gives 10-bit acceleration values, but they are stored as bytes (8-bits). To get the full value, two bytes must be combined for each axis.
	//The X value is stored in values[0] and values[1].
	x = ((int)values[1]<<8)|(int)values[0];
	//The Y value is stored in values[2] and values[3].
	y = ((int)values[3]<<8)|(int)values[2];
	//The Z value is stored in values[4] and values[5].
	z = ((int)values[5]<<8)|(int)values[4];
		
	//Print the results to the terminal.
	int sum = abs(x) + abs(y) + abs(z) - avg;
	/*Serial.print(x, DEC);
	Serial.print(',');
	Serial.print(y, DEC);
	Serial.print(',');
	Serial.println(sum);
	delay(10);
}*/

int getFilteredDistance()
{
	min1 = 10000;
	min2 = 10000;
	max1 = 0;
	max2 = 0;
	NumOfZero = 0;
	sampleAvg = 0;
	for(int i=0;i<SamplesPerMeasurement;i++)
	{
		samples[i] = readDistance();
		//Serial.println(samples[i]);
		if(samples[i] <= 0)
		{
			NumOfZero++;
			samples[i] = 0;
		}
		//find 2
		if(samples[i] > max1)
		{
			max1 = samples[i];
			max2 = max1;
		}
		else if(samples[i] > max2)
		max2 = samples[i];
		
		if(samples[i] < min1)
		{
			min1 = samples[i];
			min2 = min1;
		}
		else if(samples[i]<min2)
		min2 = samples[i];
		sampleAvg += samples[i];
		if(NumOfZero>=NumOfZeroCancelMeasurment)
			return 0;
		delay(20);
	}
	//sampleAvg = sampleAvg / (SamplesPerMeasurement - NumOfZero)
	sampleAvg -= (min1 + min2 + max1 + max2);
	
	return max(0,sampleAvg / (SamplesPerMeasurement - NumOfZero - 4));
	
}
int readDistance()
{
	digitalWrite(TRIGGER_PIN, HIGH);
	delay(1);
	digitalWrite(TRIGGER_PIN, LOW);
	return pulseIn(ECHO_PIN, HIGH, 5700);
}

//This function will write a value to a register on the ADXL345.
//Parameters:
//  char registerAddress - The register to write a value to
//  char value - The value to be written to the specified register.
void writeRegister(char registerAddress, char value){
	//Set Chip Select pin low to signal the beginning of an SPI packet.
	digitalWrite(CS, LOW);
	//Transfer the register address over SPI.
	SPI.transfer(registerAddress);
	//Transfer the desired register value over SPI.
	SPI.transfer(value);
	//Set the Chip Select pin high to signal the end of an SPI packet.
	digitalWrite(CS, HIGH);
}

//This function will read a certain number of registers starting from a specified address and store their values in a buffer.
//Parameters:
//  char registerAddress - The register addresse to start the read sequence from.
//  int numBytes - The number of registers that should be read.
//  char * values - A pointer to a buffer where the results of the operation should be stored.
void readRegister(char registerAddress, int numBytes, char * values){
	//Since we're performing a read operation, the most significant bit of the register address should be set.
	char address = 0x80 | registerAddress;
	//If we're doing a multi-byte read, bit 6 needs to be set as well.
	if(numBytes > 1)address = address | 0x40;

	//Set the Chip select pin low to start an SPI packet.
	digitalWrite(CS, LOW);
	//Transfer the starting register address that needs to be read.
	SPI.transfer(address);
	//Continue to read registers until we've read the number specified, storing the results to the input buffer.
	for(int i=0; i<numBytes; i++)
		values[i] = SPI.transfer(0x00);
	
	//Set the Chips Select pin high to end the SPI packet.
	digitalWrite(CS, HIGH);
}