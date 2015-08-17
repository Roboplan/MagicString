#define TRIGGER_PIN		9  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN		8  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define Speaker_PIN		6  
#define Button_PIN		4

#define SamplesPerMeasurement 10
#define NumOfZeroCancelMeasurment 4

int samples[SamplesPerMeasurement];
int NumOfZero = 0;
int sampleAvg = 0;
int max1,max2;
int min1,min2;
void setup() {
	pinMode(TRIGGER_PIN, OUTPUT);
	pinMode(Speaker_PIN, OUTPUT);
	pinMode(ECHO_PIN, INPUT);
	pinMode(Button_PIN,INPUT_PULLUP);
	
	Serial.begin(9600); // Open serial monitor at 115200 baud to see ping results.
	Serial.println("Start3!");
}

const int Notes[] =  //C3 to B7 and zero(noTone)
{//C  D,  E,  F, G,     A,    B
	262, 294, 330, 349, 392, 440, 494, 
	523, 587, 659, 698, 784, 880, 988, 
	1047}; //49 - 50
// C   C#    D    D#   E     F    F#   G   G#    A   A#    B

void loop()
{
	int dis = getFilteredDistance();
	
	if(dis > 0 && dis < 2000)
	{
		int note = map(dis,0,2000,0,14);
		if(digitalRead(Button_PIN) == 0)
			tone(Speaker_PIN,Notes[note]);
		else
		{
			noTone(Speaker_PIN);
			digitalWrite(Speaker_PIN,LOW);
		}
		Serial.println(note);
	}
	else
	{
		noTone(Speaker_PIN);
		digitalWrite(Speaker_PIN,LOW);
		Serial.println("-1");
	}
	delay(5);
}
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