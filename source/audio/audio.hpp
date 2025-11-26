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

namespace gore {

enum class WavBytes {
	BYTE8 = 1, BYTE16 = 2, BYTE24 = 3, BYTE32 = 4, FLOAT = 5, BYTE24PACKED = 6
};

//PCM data
struct sound {
public:
	std::string name; //file name
	uint8_t channels; //number of channels
	int framesize; //frame size
	int blockalign; //block align for samples, the size of (channels * sample size in bytes)
	size_t size; //in bytes
	WavBytes byteFormat = WavBytes::BYTE8;
	char* data = nullptr; //actual wave form data
	sound (std::string name, uint8_t channels, int framesize, int blockalign, size_t size, WavBytes byteFormat) {
		this->name = name;
		this->channels = channels;
		this->framesize = framesize;
		this->blockalign = blockalign;
		this->size = size;
		this->byteFormat = byteFormat;
		this->data = new char[this->size];
	}
	~sound() {
		delete[] data;
	}
	// copy constructor
	sound(const sound& s) {
		this->name = s.name;
		this->channels = s.channels;
		this->framesize = s.framesize;
		this->blockalign = s.blockalign;
		this->size = s.size;
		this->data = new char[this->size];
		this->byteFormat = s.byteFormat;
		std::memcpy(this->data, s.data, this->size);
	}
	// move constructor
	sound(const sound&& s) {
		this->name = s.name;
		this->channels = s.channels;
		this->framesize = s.framesize;
		this->blockalign = s.blockalign;
		this->size = s.size;
		this->byteFormat = s.byteFormat;
		this->data = std::move(s.data);
	}
	sound& operator=(const sound& s) {
		return *this = sound(s);
	}
	sound& operator=(const sound&& s) {
		return *this = sound(s);
	}
};
typedef sound* audio;

class audiostream {
private:
	struct SoundP {
	private:
		size_t pos = 0;
	public:
		int blockalign;
		size_t size; //in bytes
		char* data; //actual wave form data
		bool n_write = false;
		WavBytes byteFormat = WavBytes::BYTE8;
		bool writeData(uint8_t* dat, size_t n, size_t buffer_size, WavBytes bits);
	};

	class FileStream {
		bool n_write = false;
		size_t pos = 0;
		std::string file;
		WavBytes byteFormat = WavBytes::BYTE8;
		int blockalign = 0;
		std::ifstream fi;
	public:
		FileStream(std::string file);
		~FileStream();
		bool writeData(uint8_t* dat, size_t n, size_t buffer_size, WavBytes bits);
		bool strMatch(std::string str);
	};
	//https://stackoverflow.com/questions/74596138/microsoft-wasapi-do-different-audio-formats-need-different-data-in-the-buffer
	//translates to whatever format u need from input data
	static void* translate(void* mem, size_t size, size_t* n_size, WavBytes org_bytes, WavBytes new_bytes);

	#if defined(_WIN32)
	WAVEFORMATEX* formatex = nullptr;
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
	uint8_t* buffer = nullptr;

	#endif
	WavBytes format = WavBytes::BYTE16;
	uint32_t buffer_size = 0;
	std::vector<SoundP> sound_files;
	std::vector<std::shared_ptr<FileStream>> stream_files;

	bool play = true;
	bool fs = false;
public:
	~audiostream();
	audiostream();

	void playStream();
	void playFile(audio file);
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

// need to fix audio data converion
//	-switch to using a single function instead of big switch
class audioplayer {
private:
	struct PAudio {
		audio aud;
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
	std::vector<audiostream*> streams;
	std::vector<AudioCommand> commands;

	std::thread rend_thread;
	std::mutex mtx;
	bool run = true;
	void _RenderThread();
public:
	audioplayer() = delete;
	audioplayer(size_t n_streams);
	~audioplayer();
	// copy
	audioplayer(const audioplayer& a) = delete;
	audioplayer& operator=(const audioplayer& a) = delete;
	//move
	audioplayer(const audioplayer&& a) = delete;
	audioplayer& operator=(const audioplayer&& a) = delete;


	audio loadWavFile(std::string file);

	void playFile(std::string path, size_t stream);
	void playFile(audio file, size_t stream);
	void pause(size_t stream);
	void start(size_t stream);
	void clear(size_t stream);
	void end();

	//done
	audio generateSin(size_t length, float freq, size_t sample_rate);
	audio generateSquare(size_t length, float freq, size_t sample_rate);
	audio generateTriangle(size_t length, float freq, size_t sample_rate);
	audio generateSawtooth(size_t length, float freq, size_t sample_rate);
};

}