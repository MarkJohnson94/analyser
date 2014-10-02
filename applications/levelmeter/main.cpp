#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include "levelmeter.h"
#include "../mystream.h"
#include <time.h>

#ifdef _WIN32
	#include <windows.h> //need output solution for unix
#endif

#include "../../source/analyser.h"

unsigned channel_count;
MySampleType *buffer;
MySampleType** multibuffer;
ESAnalyser *analyser1;
ESAnalyser *analyser2;

unsigned static const int SAMPLE_RATE = 1024;

MySampleType a1=0;
MySampleType b1=0;
MySampleType c1=0;
MySampleType d1=0;

//////////////////////////////////////////
//These should be atomic values, 
//to prevent race conditions when updated in the gui
MySampleType rmsLevelValue1=0;
MySampleType rmsLevelValue2=0;
MySampleType rmsLevelValue3=0;
MySampleType rmsLevelValue4=0;
/////////////////////////////////////////

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
		//FILE *f = fopen("textfile.txt", "a+"); //used to output information in textfile
		
        //set to 512 (512 samples per channel). Future considerations; will this work for 5.1?
        const int samplesToStream = SAMPLE_RATE*channel_count;

        // set the pointer to the next audio samples to be played
        data.samples = &m_samples[m_currentSample];

        // have we reached the end of the sound?
        if (m_currentSample + samplesToStream <= m_samples.size())
        {
            // end not reached: stream the samples and continue
            data.sampleCount = samplesToStream;
            m_currentSample += samplesToStream;
			unsigned int total_sample_count = samplesToStream;

			//create an object for each channel that exists. 
			//when you call a function, the output can then depend on the input as the average for that channel is remembered
			
			for (unsigned i=0; i< total_sample_count; i++) 
			{
				buffer[i] = (MySampleType)data.samples[i]/(MySampleType)0xffff; //normalise
			}
		
			unsigned sampleCount = total_sample_count / channel_count; //number of samples per channel
			
			unsigned int count = 0;
			for (unsigned s = 0; s < sampleCount; s++) //for each channel
			{
				multibuffer[0][s] = buffer[count];
				count++;
				multibuffer[1][s] = buffer[count];
				count++;
			}
			
			a1 = ESAnalyserRMS(multibuffer[0], sampleCount);
			b1 = ESAnalyserRMS(multibuffer[1], sampleCount);
			c1 = ESAnalyserPeak(analyser1, multibuffer[0], sampleCount);
			d1 = ESAnalyserPeak(analyser2, multibuffer[1], sampleCount);
			
			
			rmsLevelValue1 = a1*500;
			rmsLevelValue2 = b1*500;
			rmsLevelValue3 = c1*500;
			rmsLevelValue4 = d1*500;
			
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
	unsigned sampleCount = total_sample_count / channel_count;
	multibuffer = new MySampleType*[channel_count];
	for(unsigned int i = 0; i < channel_count; ++i)
	{
		multibuffer[i] = new MySampleType[sampleCount];
	}
}

void levelMeter(const char* filePath)
{
	std::string aFilePath(filePath, strlen(filePath));
	
	unsigned int windowX = 300;
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

	ESAnalyserOptions options1;

	options1.levelmeter.active = true;
	options1.levelmeter.smoothing_coefficient = 0.1f;
	options1.levelmeter.peak_drop_off = 0.98f;
	options1.histogram.active = false;
	options1.waveform.active = false;

	ESAnalyserCreate(&analyser1, &options1, sample_rate);
	ESAnalyserCreate(&analyser2, &options1, sample_rate);

	unsigned int view1x = windowX;
	unsigned int view1y = windowY;
	unsigned int view2x = windowX/2;
	unsigned int view2y = windowY;
	
	sf::Font font;
	if (!font.loadFromFile("arial.ttf")) 
	{
		std::cout << ("Error loading font");
	}
	sf::Text framerate;
	framerate.setFont(font); // font is a sf::Font
	framerate.setCharacterSize(12); // in pixels, not points!
	
	sf::View view;
	view.reset(sf::FloatRect(0, 0, windowX/2, windowY));
	view.setViewport(sf::FloatRect(0, 0, 1.0f, 1.0f));

	initialisation(stream.getSampleRate()); //initialise buffers here before use, so that we aren't initialising them each time we use them

	sf::RectangleShape levelMeter1(sf::Vector2f(windowX/2, 1000));
	levelMeter1.setPosition(0, 350);
	levelMeter1.setFillColor(sf::Color::Yellow);
	
	sf::RectangleShape levelMeter2(sf::Vector2f(windowX/2, 1000));
	levelMeter2.setPosition(windowX/2, 350);
	levelMeter2.setFillColor(sf::Color::Yellow);

	sf::RectangleShape levelMeter3(sf::Vector2f(windowX/2, 1000));
	levelMeter1.setPosition(0, 350);
	levelMeter1.setFillColor(sf::Color::Red);
	
	sf::RectangleShape levelMeter4(sf::Vector2f(windowX/2, 1000));
	levelMeter2.setPosition(windowX/2, 350);
	levelMeter2.setFillColor(sf::Color::Red);
	
	stream.play();

    //window.setVerticalSyncEnabled(true);
	window.setActive(true);
    while (window.isOpen())
    {
        clock.restart();
        sf::Event event;
        while (window.pollEvent(event))
        {
			switch(event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Numpad8 || event.key.code == sf::Keyboard::W)
				{
					view.setRotation(0);
					view.reset(sf::FloatRect(0, 0, windowX, windowY));
					view.setViewport(sf::FloatRect(0, 0, 1.0f, 1.0f));
					break;
				}
				if (event.key.code == sf::Keyboard::Numpad4 || event.key.code == sf::Keyboard::A)
				{
					view.setRotation(90);
					view.setCenter(sf::Vector2f(windowY, 100));
					view.setSize(sf::Vector2f(windowX, windowY));
					break;
				}
				if (event.key.code == sf::Keyboard::Numpad6 || event.key.code == sf::Keyboard::D)
				{
					view.setRotation(270);
					view.setCenter(sf::Vector2f(windowY, 100));
					view.setSize(sf::Vector2f(windowX, windowY));
					break;
				}
			}
        }
		
		window.clear();
		
		levelMeter1.setPosition(0, windowY-rmsLevelValue1);
		levelMeter2.setPosition(windowX/2, windowY-rmsLevelValue2);
		levelMeter3.setPosition(0, windowY-rmsLevelValue3);
		levelMeter4.setPosition(windowX/2, windowY-rmsLevelValue4);

		window.draw(levelMeter3);	
		window.draw(levelMeter4);
		window.draw(levelMeter1);	
		window.draw(levelMeter2);
		

        window.display();
    }
}

int main(int argc, char **argv)
{	
	sf::Thread thread(&levelMeter, "../resources/audio.wav");
	
    thread.launch();
	thread.wait(); //keep focus until thread has closed
	
    return 0;
}