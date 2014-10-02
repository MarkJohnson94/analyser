#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include "waveform.h"
#include "../mystream.h"
#include <time.h>

#ifdef _WIN32
	#include <windows.h> //need output solution for unix
#endif

#include "../../source/analyser.h"

unsigned int channel_count;
MySampleType *buffer;
MySampleType** multibuffer;
MySampleType* values;
ESAnalyser *analyser1;
ESAnalyser *analyser2;
ESAnalyser **analyserArray;
MySampleType *buffering;
unsigned int sizeOfBuffer;

unsigned static const int SAMPLE_RATE = 2048;

void MyStream::load(const sf::SoundBuffer& buffer)
    {
        // extract the audio samples from the sound buffer to our own container
        m_samples.assign(buffer.getSamples(), buffer.getSamples() + buffer.getSampleCount());

        // reset the current playing position 
        m_currentSample = 0;

        // initialize the base class
        initialize(buffer.getChannelCount(), buffer.getSampleRate());
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

			unsigned int count = 0;
			for (unsigned s = 0; s < sample_count; s++) //for each channel
			{
				multibuffer[0][s] = buffer[count];
				count++;
				multibuffer[1][s] = buffer[count];
				count++;
			}
			//ESAnalyserWaveform(ESAnalyser* analyser, MySampleType* samples)
			ESAnalyserWaveform(analyser1, multibuffer[0]);
			ESAnalyserWaveform(analyser2, multibuffer[1]);



			for (unsigned c = 0; c < channel_count; c++) 
			{
				//ESTuple theTuple;
				
				//bool val = ESAnalyserTruncate(&theTuple, multibuffer[c], sample_count);
				//buffering = theTuple.buffer;
				//sizeOfBuffer = theTuple.size;
			}
			//fclose(f);
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
	values = new MySampleType[channel_count];
}

void createAnalysers(unsigned int numOfChannels, unsigned int sample_rate, unsigned int chunk_size)
{
	analyserArray = new ESAnalyser*[numOfChannels];
	for (int i = 0; i<numOfChannels; i++)
	{
		ESAnalyser* analyser;
		analyserArray[i] = analyser;
		//ESAnalyserCreate(&analyserArray[i], sample_rate, numOfChannels, chunk_size, 32, 100, 5); //set to store 32 samples from the audio data
	}
}

const sf::String getFPS(float currentTime)
{
	float fps = 1.f / currentTime;
	char c[10];
	sprintf(c, "%f", fps);
	return c;
}

void waveformViewer(const char* filePath)
{
	std::string aFilePath(filePath, strlen(filePath)); //only works for current file path
	unsigned int windowX = 1000;
	unsigned int windowY = 400;

	//create window of a given size
	sf::RenderWindow window;
	window.create(sf::VideoMode(windowX, windowY), "EarSoft Audio Analyser!");
	sf::Clock clock;

	//load an audio buffer from a sound file, and load buffer into the stream
    sf::SoundBuffer buffer;
    buffer.loadFromFile(aFilePath);
    MyStream stream;
    stream.load(buffer);
	
	channel_count = stream.getChannelCount();
	unsigned int sample_rate = stream.getSampleRate();
	unsigned int chunk_size = SAMPLE_RATE;
  
  	unsigned int num_of_points = 1000;
	unsigned int num_of_seconds = 1;
	unsigned int num_of_intervals = 4;
	
	ESAnalyserOptions options1;

	options1.waveform.active = true;
	options1.waveform.sample_rate = sample_rate;
	options1.waveform.channel_count = channel_count;
	options1.waveform.chunk_size = chunk_size;
	options1.waveform.num_of_points = num_of_points;
	options1.waveform.num_of_seconds = num_of_seconds;
	options1.histogram.active = false;
	options1.levelmeter.active = false;

	ESAnalyserCreate(&analyser1, &options1, sample_rate); //set to store 4 samples from the audio data
	ESAnalyserCreate(&analyser2, &options1, sample_rate); //set to store 4 samples from the audio data

	sf::View view;
	view.reset(sf::FloatRect(0, 0, windowX, windowY));
	view.setViewport(sf::FloatRect(0, 0, 1.0f, 1.0f));
	initialisation(stream.getSampleRate()); //initialise buffers here before use, so that we aren't initialising them each time we use them
	sf::VertexArray vertexPoints(sf::LinesStrip, num_of_points);
	sf::VertexArray vertexPoints2(sf::LinesStrip, num_of_points);
	
	sf::Font font;
	if (!font.loadFromFile("arial.ttf"))
	{
		std::cout << ("Error");
		std::cout << ("Error2");
		std::cout << ("Error") << (windowX);
	}
	sf::Text framerate;
	framerate.setFont(font); // font is a sf::Font
	framerate.setCharacterSize(18); // in pixels, not points!

	MySampleType *nextValueBuffer;
	nextValueBuffer = new MySampleType[num_of_points];
	MySampleType *nextValueBuffer2;
	nextValueBuffer2 = new MySampleType[num_of_points];


	stream.play();

    window.setVerticalSyncEnabled(true); //if this is enabledm display refresh rate will be synchronized with the monitor refresh rate
	//window.setActive(true);
	//window.setFramerateLimit(60);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
			switch(event.type)
			{
			case sf::Event::Closed:
				//ESAnalyserDestroy(analyser1); //free memory
				//ESAnalyserDestroy(analyser2);
				window.close();
				break;
			}
        }

		////////////////TRUNCATE SECTION/////////////////////////
		/*
		//fprintf(f, "Number of vertex points: %u\n", sizeOfBuffer);
		//fprintf(f, "Chunk size: %u\n", chunk_size);
		/int i = 0;
		for (i = 0; i < sizeOfBuffer; i++)
		{
			vertexPoints[i].position = sf::Vector2f(i+100, windowY/2+buffering[i]*400);
		}
		while (i<chunk_size)
		{
			vertexPoints[i].position = sf::Vector2f(i+100, windowY/2);
			i++;
		}
		*/
		/////////////////////////////////////

		window.clear();

		//nextValueBuffer = ESAnalyserNextValue(analyser1);
		nextValueBuffer = ESAnalyserReturnWaveform(analyser1);

		for (unsigned counter = 0; counter < num_of_points; counter++)
		{
			vertexPoints[counter].position = sf::Vector2f(counter, (1*windowY/3)+nextValueBuffer[counter]*200);
		}
		
		//nextValueBuffer2 = ESAnalyserNextValue(analyser2);
		nextValueBuffer2 = ESAnalyserReturnWaveform(analyser2);

		for (unsigned counter = 0; counter < num_of_points; counter++)
		{
			vertexPoints2[counter].position = sf::Vector2f(counter, (2*windowY/3)+nextValueBuffer2[counter]*200);
		}
		
        if (!(stream.getStatus() == MyStream::Playing))
        {
			//ESAnalyserDestroy(analyser1); //free memory
			//ESAnalyserDestroy(analyser2);
			window.close();
		}
		
		//framerate.setString(getFPS(clock.restart().asSeconds()));
		//window.draw(framerate);
		window.draw(vertexPoints);
		window.draw(vertexPoints2);
		
        window.display();
    }
}

int main(int argc, char **argv)
{
	sf::Thread thread(&waveformViewer, "../resources/song.wav");
	
    thread.launch();
	thread.wait(); //keep focus until thread has closed
	
    return 0;
}