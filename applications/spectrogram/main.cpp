#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <iostream>
#include "spectrogram.h"
#include "../mystream.h"
#include <time.h>
#include <math.h>
#include <vector>  //for std::vector
#include "../../libraries/bitmap/bitmap_image.hpp"

#ifdef _WIN32
	#include <windows.h> //need output solution for unix
#endif

#include "../../source/analyser.h"

unsigned int channel_count;
MySampleType *buffer;
MySampleType** multibuffer;
ESAnalyser *analyser1;
ESAnalyser *analyser2;
double *magArray;
double *magArray2;

unsigned static const int SAMPLE_RATE = 2048;

sf::VertexArray aVertex(sf::LinesStrip, SAMPLE_RATE/2);
std::vector<sf::VertexArray> vertex_array;

std::vector<uint8_t*> point_array;
sf::VertexArray vertexPoints(sf::LinesStrip, SAMPLE_RATE/2);
sf::VertexArray vertexPoints2(sf::LinesStrip, SAMPLE_RATE/2);

//////////////////////////////////////////
//These should be atomic values, 
//to prevent race conditions when updated in the gui
static MySampleType levelValue=0;
static MySampleType peakLevelValue=0;
static MySampleType rmsLevelValue=0;
static MySampleType threePoleAverage=0;

static MySampleType levelValue2=0;
static MySampleType peakLevelValue2=0;
static MySampleType rmsLevelValue2=0;
static MySampleType threePoleAverage2=0;
//////////////////////////////////////////
int globalCount = 0;

void bitmap()
{
	/*std::string file_name("image.bmp");

  	bitmap_image image(file_name);

  	if (!image)
  	{
    	printf("test01() - Error - Failed to open '%s'\n",file_name.c_str());
    	return;
  	}

  	image.save_image("test01_saved.bmp");*/
  	
  	/*double *bitmap_array;
	bitmap_array = new double[1024];
	
	bitmap_array = ESAnalyserReturnSpectrogram(analyser1);
	for (int i = 0; i < 1024; ++i)
	{
		std::cout << bitmap_array[i] << "\n";
	}*/
	
	uint8_t* bitmap_array = (uint8_t *)malloc((1024)*sizeof(uint8_t));
	
    int y_axis = 1000;
    int x_axis = ESAnalyserSpectrogramCount(analyser1);
    //int x_axis = 30;

    std::cout << "\nCOUNT: " << ESAnalyserSpectrogramCount(analyser1) << "\n"; 

   	bitmap_image image(x_axis, y_axis);

   	for (unsigned int x = 0; x < x_axis; ++x)
   	{
   		bitmap_array = ESAnalyserGetSpectrogramArray(analyser1, x);
      	for (unsigned int y = 0; y < y_axis; ++y)
      	{
      		int val = 255*2*bitmap_array[y];

      		if (val > 255)
      		{
      			val = 255;
      		}
      		int plot = 255-val;
  			rgb_store col = {plot, plot, plot};
  			//rgb_store col = jet_colormap[(bitmap_array[y]*1000];

  			image.set_pixel(x,y_axis-y,col.red,col.green,col.blue);
        }
   	}

   	image.save_image("test.bmp");

   	free(bitmap_array);
}

void ClientCallback(uint8_t *array, int current_array, int total_arrays)
{
	/*point_array.push_back(array);
	if (total_arrays > point_array.size())
	{
		point_array.push_back(array);
		sf::VertexArray vertex_point(sf::LinesStrip, SAMPLE_RATE/2);
		for (unsigned counter = 0; counter < 1024; counter++)
		{
			vertex_point[counter].position = sf::Vector2f((float)array[counter], counter+total_arrays);
		}
		vertex_array.push_back(vertex_point);
	}

	globalCount = total_arrays;*/


	for (unsigned counter = 0; counter < SAMPLE_RATE/2; counter++)
	{
		vertexPoints[counter].position = sf::Vector2f(counter, ((200)-log(array[counter])*50));
	}

	/*std::cout << (point_array.size()) << "\n";
	uint8_t* an_array = point_array.at(current_array);

	for (unsigned counter = 0; counter < 1024; counter++)
	{
		vertex_array[current_array][counter].position = sf::Vector2f((float)an_array[counter], counter);
		//vertexPoints[counter].position = sf::Vector2f((float)an_array[counter], counter);
	}*/
}

void MyStream::load(const sf::SoundBuffer& buffer)
{
    // extract the audio samples from the sound buffer to our own container
    m_samples.assign(buffer.getSamples(), buffer.getSamples() + buffer.getSampleCount());

    // reset the current playing position
    m_currentSample = 0;

    // initialize the base class
    initialize(buffer.getChannelCount(), buffer.getSampleRate());
}

// Create a hamming window of windowLength samples in buffer
void hamming(double *buffer, int window_length)
{
	for(int i = 0; i < window_length; i++)
	{
		buffer[i] = 0.54 - (0.46 * cos(2 * M_PI * (i / ((window_length - 1) * 1.0))));
	}
}

double windowHamming(int index, int transform_size)
{
	return (0.54 - (0.46 * cos(2 * M_PI * (index / ((transform_size - 1) * 1.0)))));
	//return (double)1;
}

double windowBlock(int index, int transform_size)
{
	//returns 0 (for the first 512), 1 (for the middle) and 0 (for the last 512)
	if (index < 512)
	{
		return 0;
	}else if (transform_size - index < 512)
	{
		return 0;
	}else
	{
		return 1;
	}
}

bool MyStream::onGetData(Chunk& data)
    {
    	//set to 512*channel count (512 samples per channel, ie 2014 for 2 channels). Future considerations; will this work for 5.1?
        const int samples_to_stream = (int)SAMPLE_RATE*(int)channel_count;

        // set the pointer to the next audio samples to be played
        data.samples = &m_samples[m_currentSample];

        // have we reached the end of the sound?
        if (m_currentSample + samples_to_stream <= m_samples.size())
        {
            // end not reached: stream the samples and continue
            data.sampleCount = samples_to_stream;
            m_currentSample += samples_to_stream;
			unsigned int total_sample_count = samples_to_stream;

			//create an object for each channel that exists. 
			//when you call a function, the output can then depend on the input as the average for that channel is remembered
			
			for (unsigned i=0; i< total_sample_count; i++) 
			{
				buffer[i] = (MySampleType)data.samples[i]/(MySampleType)0xffff; //normalise //UINT16_MAX ?
			}
		
			unsigned int sample_count = total_sample_count / channel_count; //number of samples per channel

			//FILE *f = fopen("spectrogram.txt", "a+"); //used to output information in textfile
			
			unsigned int count = 0;
			for (unsigned s = 0; s < sample_count; s++) //for each channel
			{
				multibuffer[0][s] = buffer[count];
				count++;
				multibuffer[1][s] = buffer[count];
				count++;
			}
			
			ESAnalyserSpectrogram(analyser1, multibuffer[0], sample_count);

            return true;
        }
        else
        {
            // end of stream reached: stream the remaining samples and stop playback
            data.sampleCount = m_samples.size() - m_currentSample;
            m_currentSample = m_samples.size();
            return false;
        }
    }

void MyStream::onSeek(sf::Time timeOffset)
    {
        // compute the corresponding sample index according to the sample rate and channel count
        m_currentSample = static_cast<std::size_t>(timeOffset.asSeconds() * getSampleRate() * getChannelCount());
    }

void initialisation(unsigned int total_sample_count)
{
	buffer = new MySampleType[total_sample_count];
	unsigned sample_count = total_sample_count / channel_count;
	multibuffer = new MySampleType*[channel_count];
	for(unsigned int i = 0; i < channel_count; ++i)
	{
		multibuffer[i] = new MySampleType[sample_count];
	}
	magArray = (double *)malloc((SAMPLE_RATE/2)*sizeof(double));
	magArray2 = (double *)malloc((SAMPLE_RATE/2)*sizeof(double));
}

void createBitmap()
{
	/*double *bitmap_array;
	bitmap_array = new double[1024];
	bitmap_array = ESAnalyserReturnSpectrogram(analyser1);*/
	bitmap();
}

/*void BitmapCreate(string name, int h2, int w2)  {
         h = h2; 
         w = w2;
  filename = name;
  
  Header header = {19778, sizeof(Color)*w*h , 0, 0, 54};
  Info   info   = {sizeof(Info), w, h, 1, sizeof(Color)*8, 0, ( sizeof(Color) *(w*h) ), 1, 1, 0, 0};
  Color  color  = {0,255,90,0};

  vector<Color>data;
  data.assign(w*h, color);
  fstream pic;
  pic.open(filename.c_str(), ios::out|ios::binary);

  pic.write(reinterpret_cast<char*>(&header), sizeof(header));
  pic.write(reinterpret_cast<char*>(&info), sizeof(info));
  
  for (unsigned int f = 0; f < data.size(); ++f)  {
    Color pixel = data.at(f);
    pic.write((char*)&pixel,sizeof(Color));
   }
}*/

void clearMemory()
{
	ESAnalyserDestroy(analyser1);
	ESAnalyserDestroy(analyser2);
}

void freeMemory()
{
	free(buffer);
	free(multibuffer);
	free(magArray);
	free(magArray2);
}


void waveformViewer(const char* filePath)
{
	std::string aFilePath(filePath, strlen(filePath)); //only works for current file path
	unsigned int windowX = 1024;
	unsigned int windowY = 512;

	//create window of a given size
	sf::RenderWindow window;
	window.create(sf::VideoMode(windowX, windowY), "EarSoft Audio Analyser!");
	sf::Clock clock;

	//load an audio buffer from a sound file, and load buffer into the stream
    sf::SoundBuffer buffer;
    buffer.loadFromFile(aFilePath);
    MyStream stream;
    stream.load(buffer);
	stream.setVolume(0);

	channel_count = stream.getChannelCount();
	unsigned int sample_rate = stream.getSampleRate();
	unsigned int chunk_size = SAMPLE_RATE;
  
  	unsigned int num_of_points = SAMPLE_RATE/2;
	unsigned int num_of_seconds = 1;
	unsigned int num_of_intervals = 4;
	
	ESAnalyserOptions options1;

	options1.spectrogram.active = true;
	options1.spectrogram.overlap = (double)0.5;
	options1.histogram.active = false;
	options1.levelmeter.active = false;
	options1.waveform.active = false;
	options1.lib.callback = &ClientCallback;

	ESAnalyserCreate(&analyser1, &options1, sample_rate); //set to store 4 samples from the audio data
	ESAnalyserCreate(&analyser2, &options1, sample_rate); //set to store 4 samples from the audio data*/

	sf::View view;
	view.reset(sf::FloatRect(0, 0, windowX, windowY));
	view.setViewport(sf::FloatRect(0, 0, 1.0f, 1.0f));
	initialisation(stream.getSampleRate()); //initialise buffers here before use, so that we aren't initialising them each time we use them
	
	sf::Font font;
	if (!font.loadFromFile("arial.ttf"))
	{
		std::cout << ("Error loading font");
	}

	sf::Text framerate;
	framerate.setFont(font); // font is a sf::Font
	framerate.setCharacterSize(18); // in pixels, not points!

	MySampleType *nextValueBuffer;
	nextValueBuffer = new MySampleType[num_of_points];
	MySampleType *nextValueBuffer2;
	nextValueBuffer2 = new MySampleType[num_of_points];
	double *magArray;
	magArray = new double[num_of_points];

	stream.play();

    window.setVerticalSyncEnabled(true); //if this is enabledm display refresh rate will be synchronized with the monitor refresh rate
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
			switch(event.type)
			{
			case sf::Event::Closed:
				//createBitmap();
				printTheLinkedList(analyser1, SAMPLE_RATE);
				clearMemory();
				freeMemory();
				window.close();
				break;
			}
        }

		window.clear();

		/*magArray = ESAnalyserReturnSpectrogram(analyser1);
		
		for (unsigned counter = 0; counter < num_of_points; counter++)
		{
			vertexPoints[counter].position = sf::Vector2f(counter, ((2*windowY/10)-log(magArray[counter])*50));
		}*/
 
		MotionAdvance(analyser1);

		if (!(stream.getStatus() == MyStream::Playing))
		{
			clearMemory();
			freeMemory();
			window.close();
		}
		
		//framerate.setString(getFPS(clock.restart().asSeconds()));
		//window.draw(framerate);
		//window.draw(aVertex);
		window.draw(vertexPoints2);
		window.draw(vertexPoints);
		/*for (int i = 0; i < globalCount; ++i)
		{
			window.draw(vertex_array[i]);
		}*/
		
		window.display();
	}
}

int main(int argc, char **argv)
{
	//create thread for gui
	AllocConsole();
	freopen("CONIN$", "r", stdin); 
	freopen("CONOUT$", "w", stdout); 
	freopen("CONOUT$", "w", stderr); 
	sf::Thread thread(&waveformViewer, "../resources/audio.wav");
	
	thread.launch();
	thread.wait(); //keep focus until thread has closed
	
	return 0;
}