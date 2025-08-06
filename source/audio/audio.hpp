#pragma once
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#if defined (_WIN32)
#include <Windows.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <sstream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <endpointvolume.h>
#endif
#if defined(__unix__)
#include <alsa/asoundlib.h>
#include <alloca.h>
#endif

//PCM data
struct Sound {
public:
	std::string name; //file name
	uint8_t samplebits; //number of bits per sample
	uint8_t channels; //number of channels
	int framesize; //frame size
	int blockalign; //block align for samples
	size_t size; //in bytes
	char* data = nullptr; //actual wave form data
	Sound (std::string name, uint8_t samplebits, uint8_t channels, int framesize, int blockalign, size_t size) {
		this->name = name;
		this->samplebits = samplebits;
		this->channels = channels;
		this->framesize = framesize;
		this->blockalign = blockalign;
		this->size = size;
		this->data = new char[this->size];
	}
	~Sound() {
		delete[] data;
	}
	// copy constructor
	Sound(const Sound& s) {
		this->name = s.name;
		this->samplebits = s.samplebits;
		this->channels = s.channels;
		this->framesize = s.framesize;
		this->blockalign = s.blockalign;
		this->size = s.size;
		this->data = new char[this->size];
		std::memcpy(this->data, s.data, this->size);
	}
	// move constructor
	Sound(const Sound&& s) {
		this->name = s.name;
		this->samplebits = s.samplebits;
		this->channels = s.channels;
		this->framesize = s.framesize;
		this->blockalign = s.blockalign;
		this->size = s.size;
		this->data = std::move(s.data);
	}
	Sound& operator=(const Sound& s) {
		return *this = Sound(s);
	}
	Sound& operator=(const Sound&& s) {
		return *this = Sound(s);
	}
};
typedef Sound* Audio;

enum class WavBytes {
	BYTE8 = 1, BYTE16 = 2, BYTE24 = 3, BYTE32 = 4
};

class AudioStream {
private:
	struct SoundP {
	private:
		size_t pos = 0;
		bool n_write = false;
	public:
		int blockalign;
		size_t size; //in bytes
		char* data; //actual wave form data
		int bytesp = 1; //bytes per sample

		bool writeData(uint8_t* dat, size_t n, WavBytes bits);
	};

	class FileStream {
		bool n_write = false;
		size_t pos = 0;
		std::string file;
		int bytesp = 1;
		int blockalign = 0;
		std::ifstream fi;
	public:
		FileStream(std::string file);
		~FileStream();
		bool writeData(uint8_t* dat, size_t n, WavBytes bits);
		bool strMatch(std::string str);
	};
	//https://stackoverflow.com/questions/74596138/microsoft-wasapi-do-different-audio-formats-need-different-data-in-the-buffer
	//translates to whatever format u need from input data
	class Translator {
	private:
		static int convertRange(int n, int min, int max, int n_min, int n_max);
		static float convertRange(float n, float min, float max, float n_min, float n_max);
		static short convertRange(short n, short min, short max, short n_min, short n_max);
		//redone
		static void convertToFloat(uint8_t* mem, size_t size, void* n_mem, size_t n_size);
		//redone
		static void convertToFloat(short* mem, size_t size, void* n_mem, size_t n_size);
		//redone
		static void convert24ToFloat(uint8_t* mem, size_t size, void* n_mem, size_t n_size);
		//redone
		static void convertTo16bit(char* mem, size_t size, void* n_mem, size_t n_size);
		//redone
		static void convertTo16bit(float* mem, size_t size, void* n_mem, size_t n_size);
		//redo 24 bit
		static void convert24To16bit(char* mem, size_t size, void* n_mem, size_t n_size);
		//redone
		static void convertTo8bit(short* mem, size_t size, void* n_mem, size_t n_size);
		//redo 24 bit
		static void convert24To8bit(char* mem, size_t size, void* n_mem, size_t n_size);
		//redone
		static void convertTo8bit(float* mem, size_t size, void* n_mem, size_t n_size);
		//redo this
		static void convertTo24bit(uint8_t* mem, size_t size, void* n_mem, size_t n_size);
		static void convertTo24bit(short* mem, size_t size, void* n_mem, size_t n_size);
		static void convertTo24bit(float* mem, size_t size, void* n_mem, size_t n_size);

	public:
		//make sure to free the memory returned here
		static void* translate(void* mem, size_t size, size_t* n_size, WavBytes org_bytes, WavBytes new_bytes);
	};

	#if defined(_WIN32)
	WAVEFORMATEX* format = nullptr;
	IAudioClient* client = nullptr;
	IAudioRenderClient* render = nullptr;
	ISimpleAudioVolume* volume = nullptr;
	IMMDevice* pdevice = nullptr;
	IMMDeviceEnumerator* penum = nullptr;
	HANDLE bufReady;
	HANDLE shutdown; //add later
	HANDLE paused;
	#endif
	#if defined(__unix__)
	snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
    snd_pcm_uframes_t frames;
	uint16_t* buffer = nullptr;
	WavBytes format = WavBytes::BYTE16;

	#endif
	uint32_t buffer_size = 0;
	std::vector<SoundP> sound_files;
	std::vector<std::shared_ptr<FileStream>> stream_files;

	bool play = true;
	bool fs = false;
public:
	~AudioStream();
	AudioStream();

	void playStream();
	void playFile(Audio file);
	void streamFile(std::string file);
	void pause();
	void start();
	void reset();
};


//https://habr.com/en/articles/663352/#windows-and-wasapi
//https://www.sounddevices.com/32-bit-float-files-explained/
//https://stackoverflow.com/questions/44759526/how-winapi-handle-iaudioclient-seteventhandle-works
//https://gist.github.com/Liastre/ff201f37bc62f6dc0b7f5541923565ab
//https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/multimedia/audio/RenderExclusiveEventDriven/WASAPIRenderer.cpp

// need to fix the audio player with 8 bit data
//	-remove all mallocs??
class AudioPlayer {
private:
	struct PAudio {
		Audio aud;
		size_t stream;
	};
	struct FStream {
		std::string file;
		size_t stream;
	};

	struct AudioCommand {
		size_t type;
		size_t stream;
	};



	std::vector<PAudio> sound_files;
	std::vector<FStream> stream_files;
	std::vector<AudioStream*> streams;
	std::vector<AudioCommand> commands;

	std::thread rend_thread;
	std::mutex mtx;
	bool run = true;
	void _RenderThread();
public:
	AudioPlayer() = delete;
	AudioPlayer(size_t n_streams);
	~AudioPlayer();
	// copy
	AudioPlayer(const AudioPlayer& a) = delete;
	AudioPlayer& operator=(const AudioPlayer& a) = delete;
	//move
	AudioPlayer(const AudioPlayer&& a) = delete;
	AudioPlayer& operator=(const AudioPlayer&& a) = delete;


	Audio loadWavFile(std::string file);

	void playFile(std::string path, size_t stream);
	void playFile(Audio file, size_t stream);
	void pause(size_t stream);
	void start(size_t stream);
	void clear(size_t stream);
	void end();

	//done
	Audio generateSin(size_t length, float freq, size_t sample_rate);
	Audio generateSquare(size_t length, float freq, size_t sample_rate);
	Audio generateTriangle(size_t length, float freq, size_t sample_rate);
	Audio generateSawtooth(size_t length, float freq, size_t sample_rate);
};