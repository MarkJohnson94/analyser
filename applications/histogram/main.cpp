#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include "histogram.h"
#include "../mystream.h"
#include <time.h>
#include "../../libraries/bitmap/bitmap_image.hpp"

#ifdef _WIN32
	#include <windows.h> //need output solution for unix
#endif

#include "../../source/analyser.h"

unsigned num_of_channels;
MySampleType *singlebuffer;
MySampleType** multibuffer;
ESAnalyser *analyser1;
ESAnalyser *analyser2;

unsigned static const int SAMPLE_RATE = 2048;

void bitmap(int num_of_segments)
{
	MySampleType *a_buffer;
	a_buffer = new MySampleType[num_of_segments];
	a_buffer = ESAnalyserGetNormalisedHistogram(analyser2);

	int scaling = 1; //each point from the array will be drawn on x pixels, where x = scaling.

	const unsigned int y_axis = 100;
    unsigned int x_axis = num_of_segments*scaling;

    bitmap_image image(x_axis, y_axis);

   	for (unsigned int x = 0; x < x_axis; ++x)
   	{
      	for (unsigned int y = 0; y < y_axis; ++y)
      	{
      		//int val = 255*a_buffer[x];
      		//int val = 255;
      		//int plot = 255-val;

      		//int point = a_buffer[x];
      		int limit = y_axis*a_buffer[x/scaling];
      		int plot;
      		if (y > limit)
      		{
      			plot = 255;
      		}else
      		{
      			plot = 0;
      		}

  			rgb_store col = {plot, plot, plot};
  			image.set_pixel(x,y,col.red,col.green,col.blue);
        }
   	}

   	image.vertical_flip();
  	//image.horizontal_flip();
   	image.save_image("output.bmp");
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

bool MyStream::onGetData(Chunk& data)
    {
        const int samplesToStream = SAMPLE_RATE*num_of_channels;

        // set the pointer to the next audio samples to be played
        data.samples = &m_samples[m_currentSample];

        // have we reached the end of the sound?
        if (m_currentSample + samplesToStream <= m_samples.size())
        {
            // end not reached: stream the samples and continue
            data.sampleCount = samplesToStream;
            m_currentSample += samplesToStream;
			unsigned int total_sample_count = samplesToStream;

			for (unsigned i=0; i< total_sample_count; i++) 
			{
				singlebuffer[i] = (MySampleType)data.samples[i]/(MySampleType)0xffff; //normalise
			}
		
			unsigned sample_count = total_sample_count / num_of_channels; //number of samples per channel

			unsigned int count = 0;
			for (unsigned s = 0; s < sample_count; s++) //for each channel
			{
				multibuffer[0][s] = singlebuffer[count];
				count++;
				multibuffer[1][s] = singlebuffer[count];
				count++;
			}
			
			ESAnalyserHistogram(analyser1, multibuffer[0], sample_count);
			ESAnalyserHistogram(analyser2, multibuffer[1], sample_count);

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
	singlebuffer = new MySampleType[total_sample_count];
	unsigned sampleCount = total_sample_count / num_of_channels;
	multibuffer = new MySampleType*[num_of_channels];
	for(unsigned int i = 0; i < num_of_channels; ++i)
	{
		multibuffer[i] = new MySampleType[sampleCount];
	}
}

void clearMemory()
{
	int segments = ESAnalyserGetHistogramSegments(analyser2);
	std::cout << segments << "SEGMENTS\n";
	bitmap(segments);
	//ESAnalyserSaveHistogram(analyser1, true);
	//ESAnalyserDestroy(analyser1);
	//ESAnalyserSaveHistogram(analyser2, true);
	//ESAnalyserDestroy(analyser2);
}

void freeMemory()
{
	//free(singlebuffer);
	//free(multibuffer);
}

void waveformViewer(const char* filePath)
{
	std::string aFilePath(filePath, strlen(filePath)); //only works for current file path
	unsigned int windowX = 600;
	unsigned int windowY = 100;

	//create window of a given size
	sf::RenderWindow window;
	window.create(sf::VideoMode(windowX, windowY), "EarSoft Audio Analyser!");
	sf::Clock clock;

	//load an audio buffer from a sound file, and load buffer into the stream
    sf::SoundBuffer buffer;
    buffer.loadFromFile(aFilePath);
    MyStream stream;
    stream.load(buffer);

	num_of_channels = stream.getChannelCount();
	unsigned int sample_rate = stream.getSampleRate();
	unsigned int chunk_size = SAMPLE_RATE;
	
	unsigned int num_of_points = 1000;
	unsigned int num_of_seconds = 2;

	float high1 = 0.0f;
	float low1 = -60.0f;
	float interval1 = 1.0f;
	float val1 = (high1-low1)/interval1;
	int num_of_segments1 = (int)val1;

	float high2 = -20.0f;
	float low2 = -100.0f;
	float interval2 = 0.1f;
	float val2 = (high2-low2)/interval2;
	int num_of_segments2 = (int)val2;

	ESAnalyserOptions options1;

	options1.histogram.active = true;
	options1.histogram.high = high1;
	options1.histogram.low = low1;
	options1.histogram.interval = interval1;
	options1.histogram.period = 400.0f;
	options1.histogram.sample_rate = sample_rate;
	options1.waveform.active = false;
	options1.levelmeter.active = false;

	ESAnalyserOptions options2;

	options2.histogram.active = true;
	options2.histogram.high = high2;
	options2.histogram.low = low2;
	options2.histogram.interval = interval2;
	options2.histogram.period = 40.0f;
	options2.histogram.sample_rate = sample_rate;
	options2.waveform.active = false;
	options2.levelmeter.active = false;

	ESAnalyserCreate(&analyser1, &options1, sample_rate);
	ESAnalyserCreate(&analyser2, &options2, sample_rate);

	sf::View view;
	view.reset(sf::FloatRect(0, 0, windowX, windowY));
	view.setViewport(sf::FloatRect(0, 0, 1.0f, 1.0f));
	initialisation(stream.getSampleRate()); //initialise buffers here before use, so that we aren't initialising them each time we use them
	
	float xPosition1 = (float)windowX/(float)num_of_segments1;
	float xPosition2 = (float)windowX/(float)num_of_segments2;

	sf::VertexArray vertexPoints(sf::Lines, num_of_segments1*2);
	sf::VertexArray vertexPoints2(sf::Lines, num_of_segments2*2);
	
	sf::Font font;
	if (!font.loadFromFile("arial.ttf")) 
	{
		std::cout << ("Unabled to load font");
	}
	sf::Text framerate;
	framerate.setFont(font); // font is a sf::Font
	framerate.setCharacterSize(12); // in pixels, not points!

	MySampleType *nextValueBuffer;
	nextValueBuffer = new MySampleType[num_of_segments1];
	MySampleType *nextValueBuffer2;
	nextValueBuffer2 = new MySampleType[num_of_segments2];

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
				clearMemory();
				freeMemory();
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::R)
				{
					ESAnalyserResetHistogram(analyser1);
					ESAnalyserResetHistogram(analyser2);
					break;
				}
			}
        }

		window.clear();

		nextValueBuffer = ESAnalyserGetHistogram(analyser1);

		for (unsigned counter = 0; counter < num_of_segments1*2; counter = counter + 2)
		{	
			vertexPoints[counter].position = sf::Vector2f((counter/2)*xPosition1, (windowY/2));
			vertexPoints[counter+1].position = sf::Vector2f((counter/2)*xPosition1, (windowY/2)-nextValueBuffer[counter/2]*25);
		}
		
		nextValueBuffer2 = ESAnalyserGetHistogram(analyser2);

		for (unsigned counter = 0; counter < num_of_segments2*2; counter = counter + 2)
		{
			vertexPoints2[counter].position = sf::Vector2f((counter/2)*xPosition2, windowY);
			vertexPoints2[counter+1].position = sf::Vector2f((counter/2)*xPosition2, windowY-nextValueBuffer2[counter/2]*25);
		}
		
        if (!(stream.getStatus() == MyStream::Playing))
        {
			clearMemory();
			window.close();
		}
		
		window.draw(vertexPoints);
		window.draw(vertexPoints2);
		
        window.display();
    }
}

int main(int argc, char **argv)
{
	sf::Thread thread(&waveformViewer, "../resources/pizza.wav");
	
    thread.launch();
	thread.wait(); //keep focus until thread has closed

	clearMemory();
    return 0;
}