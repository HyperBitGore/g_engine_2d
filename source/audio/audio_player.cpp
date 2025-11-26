#include "audio.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <sstream>
#include <cstring>
#include <iostream>
#include <stdexcept>

#define SwapFourBytes(data)   \
( (((data) >> 24) & 0x000000FF) | (((data) >>  8) & 0x0000FF00) | \
  (((data) <<  8) & 0x00FF0000) | (((data) << 24) & 0xFF000000) ) 
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }


uint32_t getByteSize(gore::WavBytes org_bytes) {

     switch (org_bytes) {
        case gore::WavBytes::BYTE24:
        case gore::WavBytes::FLOAT:
            return 4;
        case gore::WavBytes::BYTE24PACKED:
            return 3;
        default:
            return (size_t)org_bytes;
    }
}                

gore::audio gore::audioplayer::loadWavFile(std::string file) {
    std::ifstream f;
    f.open(file, std::ios::binary);
    std::stringstream str_st;
    str_st << f.rdbuf();
    std::string str = str_st.str();
    char* c = (char*)str.c_str();
    f.close();
    //skip first four bytes because they are just RIFF
    c += 4;
    int* it = (int*)c;
    int size = *it;
    it += 2;//skipping WAVE part
    //now its format chunk marker
    c = (char*)it;
    //read the four bytes and add to string
    std::string format;
    for (int i = 0; i < 4; i++) {
        format.push_back(*c);
        c++;
    }
    //now length of format data
    it = (int*)c;
    int flength = *it;
    it++;
    short* st = (short*)it;
    short typef = *st; //type of format(1 is pcm)
    st++;
    short num_channels = *st;
    st++;
    it = (int*)st;
    //reading the sample rate and related now
    int samplerate = *it;
    it++;
    int byterate = *it; //bytespersample
    it++;
    st = (short*)it;
    short blockalign = *st;
    st++;
    short bitspps = *st;
    st++;
    //find the data identifier and read the data chunk after it
    size_t pos = str.find("data"); //finding data so we can read
    c = (char*)str.c_str();
    c += pos + 4;
    it = (int*)c;
    int datasize = *it; //size of data section
    it++;
    //now we are at the pcm data
    c = (char*)it;
    gore::WavBytes bitformat = (gore::WavBytes)(bitspps / 8);
    if (typef == 3) {
        bitformat = gore::WavBytes::FLOAT;
    } else if (bitspps / 8 == 3) {
        bitformat = gore::WavBytes::BYTE24PACKED;
    }
    audio ad = new sound(file, num_channels, byterate, blockalign, datasize, bitformat);
    if (ad->data) {
        memcpy(ad->data, c, ad->size);
    }
    else {
        std::cout << "Failed to allocate enough space for audio data\n";
    }
    return ad;
}



//https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-initialize
//https://learn.microsoft.com/en-us/windows/win32/coreaudio/rendering-a-stream
//https://learn.microsoft.com/en-us/windows/win32/coreaudio/exclusive-mode-streams
//https://hero.handmade.network/forums/code-discussion/t/8433-correct_implementation_of_wasapi

//add class which will convert the data being played into the needed format to be output to the audio device
//going down form 32->24->16 audio is just narrowing conversion clamp into those lower bands
//going up from 16->24->32 is widening conversion just convert the data into 32 bit readable data so wasapi can actually play it, wave will still be the same


//adds data to stream and checks if already playing, if already playing start again? if not playsound
void gore::audioplayer::playFile(audio file, size_t stream) {
    std::lock_guard<std::mutex> guard(mtx);
    if (stream < streams.size()) {
        PAudio sp;
        sp.aud = file;
        sp.stream = stream;
        sound_files.push_back(sp);
    }
}

void gore::audioplayer::playFile(std::string path, size_t stream) {
    std::lock_guard<std::mutex> guard(mtx);
    if (stream < streams.size()) {
        FStream fp;
        fp.file = path;
        fp.stream = stream;
        stream_files.push_back(fp);
   }
}

void gore::audioplayer::start(size_t stream) {
    std::lock_guard<std::mutex> guard(mtx);
    commands.push_back({ 1, stream });
}

void gore::audioplayer::clear(size_t stream) {
    std::lock_guard<std::mutex> guard(mtx);
    commands.push_back({ 2, stream });
}
void gore::audioplayer::pause(size_t stream) {
    std::lock_guard<std::mutex> guard(mtx);
    commands.push_back({ 0, stream });
}

void gore::audioplayer::end() {
    std::lock_guard<std::mutex> guard(mtx);
    run = false;
}

float sgn(float x) {
    if (x < 0.f) return -1.f;
    if (x > 0.f) return 1.f;
    return 0.f;
}

//generates sin wave, based on length given in milliseconds
gore::audio gore::audioplayer::generateSin(size_t length, float freq, size_t sample_rate) {
    audio a = new sound("sine", 2, (sample_rate * 32 * 2) / 8, 8, (length * (((sample_rate * 32 * 2) / 8) / 1000)), gore::WavBytes::FLOAT);
    float* ff = (float*)a->data;
    size_t sample_size = a->size / 4;
    for (size_t i = 0; i < sample_size; i++) {
        float f = sinf((2.0f * (float)M_PI * freq) / sample_rate * i);
        *(ff + i) = f;
    }
	return a;
}
//generates square wave, based on length given in milliseconds
gore::audio gore::audioplayer::generateSquare(size_t length, float freq, size_t sample_rate) {
    audio a = new sound("square", 2, (sample_rate * 32 * 2) / 8, 8, (length * (((sample_rate * 32 * 2) / 8) / 1000)), gore::WavBytes::FLOAT);
    float* ff = (float*)a->data;
    size_t sample_size = a->size / 4;
    for (size_t i = 0; i < sample_size; i++) {
        float f = sgn(sinf((2.0f * (float)M_PI * freq) / sample_rate * i));
        *(ff + i) = f;
    }
    return a;
}
//generates triangle wave, based on length given in milliseconds
gore::audio gore::audioplayer::generateTriangle(size_t length, float freq, size_t sample_rate) {
    audio a = new sound("triangle", 2, (sample_rate * 32 * 2) / 8, 8, (length * (((sample_rate * 32 * 2) / 8) / 1000)), gore::WavBytes::FLOAT);
    float* ff = (float*)a->data;
    size_t sample_size = a->size / 4;
    for (size_t i = 0; i < sample_size; i++) {
        float f = (float)(2/M_PI) * asinf(sinf((2.0f * (float)M_PI * freq) / sample_rate * i));
        *(ff + i) = f;
    }
    return a;
}

float frac(float x) {
    return (x - (long)x);
}

gore::audio gore::audioplayer::generateSawtooth(size_t length, float freq, size_t sample_rate) {
    audio a = new sound("sawtooth", 2, (sample_rate * 32 * 2) / 8, 8, (length * (((sample_rate * 32 * 2) / 8) / 1000)), gore::WavBytes::FLOAT);
    float* ff = (float*)a->data;
    size_t sample_size = a->size / 4;
    for (size_t i = 0; i < sample_size; i++) {
        float f = frac(freq / sample_rate * i);
        *(ff + i) = f;
    }
    return a;
}

void gore::audioplayer::_RenderThread() {
    while (run) {
        std::lock_guard<std::mutex> guard(mtx);
        for (auto& i : sound_files) {
            streams[i.stream]->playFile(i.aud);
            streams[i.stream]->start();
        }
        sound_files.clear();
        for(auto& i : stream_files) {
            streams[i.stream]->streamFile(i.file);
            streams[i.stream]->start();
        }
        stream_files.clear();

        for (auto& i : commands) {
            switch (i.type) {
            case 0:
                streams[i.stream]->pause();
                break;
            case 1:
                streams[i.stream]->start();
                break;
            case 2:
                streams[i.stream]->reset();
                break;
            }

        }
        commands.clear();

        for (auto& i : streams) {
            i->playStream();
        }
    }
}

gore::audioplayer::audioplayer(size_t n_streams) {
    for (size_t i = 0; i < n_streams; i++) {
        audiostream* as = new audiostream;
        streams.push_back(as);
    }

    rend_thread = std::thread(&gore::audioplayer::_RenderThread, this);
}

gore::audioplayer::~audioplayer() {
    rend_thread.join();
    run = false;
    for (size_t i = 0; i < streams.size(); i++) {
        audiostream* as = streams[i];
        streams.erase(streams.begin() + i);
        delete as;
    }

}

void gore::audiostream::playStream() {
    if (play) {
        #if defined(_WIN32)
        int32_t res = WaitForSingleObject(bufReady, 0);

        if (res == WAIT_OBJECT_0) {

            uint32_t filled;
            client->GetCurrentPadding(&filled);
            uint32_t free = buffer_size - filled;
            if (free > 0) {
                uint8_t* data;
                render->GetBuffer(free, &data);
                for (size_t i = 0; i < stream_files.size();) {
                    if (!stream_files[i]->writeData((uint8_t*)data, free, this->buffer_size, this->format)) {
                        stream_files.erase(stream_files.begin() + i);
                    }
                    else {
                        i++;
                    }
                }
                for (size_t i = 0; i < sound_files.size();) {
                    if (!sound_files[i].writeData((uint8_t*)data, free, this->buffer_size, this->format)) {
                        sound_files.erase(sound_files.begin() + i);
                    }
                    else {
                        i++;
                    }
                }
                render->ReleaseBuffer(free, 0);
                if (sound_files.size() <= 0 && stream_files.size() <= 0) {
                    play = false;
                }
            }
        }
        #endif
        #if defined (__unix__)
        snd_pcm_state_t state = snd_pcm_state(pcm_handle);
        if (state == SND_PCM_STATE_XRUN) {
            int err = snd_pcm_prepare(pcm_handle);
            if (err < 0) {
                std::stringstream ss;
                ss << "Failed to prepare device: " << snd_strerror(err) << "\n";
                throw std::runtime_error(ss.str());
            }
            err = snd_pcm_start(pcm_handle);
            if (err < 0) {
                // handle error
                std::stringstream ss;
                ss << "Failed to start device: " << snd_strerror(err) << "\n";
                throw std::runtime_error(ss.str());
            }
        }
        snd_pcm_sframes_t avail = snd_pcm_avail_update(pcm_handle);
        if (avail > 0) {
            // clamp to frame capacity
            if ((size_t)avail > buffer_size / (2 * getByteSize(this->format))) {
                avail = buffer_size / (2 * getByteSize(this->format));
            }
            // prevent underflow
            if (avail <= 0 || (size_t)avail > buffer_size / (2 * getByteSize(this->format))) {
                return;
            }
            for (size_t i = 0; i < stream_files.size();) {
                if (!stream_files[i]->writeData((uint8_t*)buffer, avail, this->buffer_size, format)) {
                    stream_files.erase(stream_files.begin() + i);
                }
                else {
                    i++;
                }
            }
            for (size_t i = 0; i < sound_files.size();) {
                if (!sound_files[i].writeData((uint8_t*)buffer, avail, this->buffer_size, format)) {
                    sound_files.erase(sound_files.begin() + i);
                }
                else {
                    i++;
                }
            }
            snd_pcm_sframes_t written = snd_pcm_writei(pcm_handle, buffer, avail);
            if (written < 0) {
                if (written == -EPIPE) {
                    snd_pcm_prepare(pcm_handle); // buffer underrun
                } else {
                    std::cerr << "ALSA write error: " << snd_strerror(written) << "\n";
                }
            }
            if (sound_files.size() <= 0 && stream_files.size() <= 0) {
                play = false;
            }
        }
        #endif
    }
}

gore::audiostream::audiostream() {
    #if defined(_WIN32)
    HRESULT hr = CoInitializeEx(NULL, 0);
    if (FAILED(hr)) {
        return;
    }
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&penum);
    if (FAILED(hr)) {
        return;
    }

    hr = penum->GetDefaultAudioEndpoint(eRender, eConsole, &pdevice);
    if (FAILED(hr)) {
        return;
    }
    hr = pdevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&client);
    if (FAILED(hr)) {
        return;
    }


    hr = client->GetMixFormat(&formatex);
    switch(formatex->wFormatTag) {
        case WAVE_FORMAT_PCM:
            switch (formatex->wBitsPerSample) {
                case 8:
                    this->format = gore::WavBytes::BYTE8;
                break;
                case 16:
                    this->format = gore::WavBytes::BYTE16;
                break;
                case 24:
                    this->format = gore::WavBytes::BYTE24PACKED;
                break;
                case 32:
                    this->format = gore::WavBytes::BYTE32;
                break;
            }    
        break;
        case WAVE_FORMAT_IEEE_FLOAT:
            this->format = gore::WavBytes::FLOAT;
        break;
        case WAVE_FORMAT_EXTENSIBLE:
            {
                WAVEFORMATEXTENSIBLE* ex = (WAVEFORMATEXTENSIBLE*)formatex;
                if (ex->SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
                    if (ex->Samples.wValidBitsPerSample != formatex->wBitsPerSample) {
                        if (ex->Samples.wValidBitsPerSample == 24 && formatex->wBitsPerSample == 32) {
                            this->format = gore::WavBytes::BYTE24;
                        } else {
                            throw std::runtime_error("Unsupported sound hardware!");
                        }
                    } else {
                        this->format = (gore::WavBytes)(formatex->wBitsPerSample/8);
                    }
                } else if (ex->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
                     this->format = gore::WavBytes::FLOAT;
                }

            }
        break;
    }

    if (FAILED(hr)) {
        return;
    }
    int buffer_length_msec = 10;
    REFERENCE_TIME dur = buffer_length_msec * 1000 * 10;
    hr = client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY, dur, dur, formatex, NULL);
    if (FAILED(hr)) {
        return;
    }
    hr = client->GetBufferSize(&buffer_size);
    if (FAILED(hr)) {
        return;
    }
    hr = client->GetService(__uuidof(IAudioRenderClient), (void**)&render);
    if (FAILED(hr)) {
        return;
    }
    bufReady = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    hr = client->SetEventHandle(bufReady);
    if (FAILED(hr)) {
        return;
    }
    if (bufReady == NULL) {
        return;
    }
    shutdown = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (shutdown == NULL) {
        return;
    }
    paused = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (paused == NULL) {
        return;
    }


    BYTE* dat1;
    render->GetBuffer(buffer_size, &dat1);

    render->ReleaseBuffer(buffer_size, 0);
    client->Start();
    #endif
    #if defined(__unix__)
    int err = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        std::cout << "Failed to open alsa device\n";
        return;
    }
    snd_pcm_sw_params_malloc(&sw_params);
	snd_pcm_sw_params_current(pcm_handle, sw_params);

	snd_pcm_hw_params_alloca(&hw_params);
    /* Setup HW params for all possible parameters */
	if (snd_pcm_hw_params_any(pcm_handle, hw_params) < 0) {
		std::cout << "Failed to retrieve HW params\n";
		return;
	}
    //set hw params
    snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_format_t avail[] = { SND_PCM_FORMAT_U8, SND_PCM_FORMAT_S16_LE, SND_PCM_FORMAT_S24_LE, SND_PCM_FORMAT_FLOAT_LE};
    snd_pcm_format_t format = avail[0];
    for (size_t i = 0; i < 4; i++) {
        if (snd_pcm_hw_params_test_format(pcm_handle, hw_params, format) == 0) {
            format = avail[i];
        }
    }
    snd_pcm_hw_params_set_format(pcm_handle, hw_params, format);
    snd_pcm_hw_params_set_channels(pcm_handle, hw_params, 2);
    snd_pcm_hw_params_set_rate(pcm_handle, hw_params, 44100, 0);
    snd_pcm_hw_params_set_period_size(pcm_handle, hw_params, 32, 0);
    //apply params
    snd_pcm_hw_params(pcm_handle, hw_params);

    int dir;
    snd_pcm_hw_params_get_period_size(hw_params, &frames, &dir);
    // setting up the buffer, since alsa requires we make our own
    // rework this to be setttable by user
    switch (format) {
        break;
        case SND_PCM_FORMAT_S16_LE:
            this->format = gore::WavBytes::BYTE16;
            buffer = (uint8_t*)new char[frames * 2 * getByteSize(this->format)];
            buffer_size = frames * 2 * getByteSize(this->format);
        break;
        case SND_PCM_FORMAT_S24_LE:
            // s24 is padded
            this->format = gore::WavBytes::BYTE24;
            buffer = (uint8_t*)new char[frames * 2 * getByteSize(this->format)];
            buffer_size = frames * 2 * getByteSize(this->format);
        break;
        case SND_PCM_FORMAT_FLOAT_LE:
            this->format = gore::WavBytes::FLOAT;
            buffer = (uint8_t*)new char[frames * 2 * getByteSize(this->format)];
            buffer_size = frames * 2 * getByteSize(this->format);
        break;
        case SND_PCM_FORMAT_S32_LE:
            this->format = gore::WavBytes::BYTE32;
            buffer = (uint8_t*)new char[frames * 2 * getByteSize(this->format)];
            buffer_size = frames * 2 * getByteSize(this->format);
        break;
        default:
            this->format = gore::WavBytes::BYTE8;
            buffer = (uint8_t*)new char[frames * 2 * getByteSize(this->format)];
            buffer_size = frames * 2 * getByteSize(this->format);
        break;
    }
    if (!buffer) {
        std::cerr << "Audio stream memory allocate error\n";
        return;
    }
    play = false;
    #endif
}
gore::audiostream::~audiostream() {
    #if defined(_WIN32)
    CoTaskMemFree(formatex);
    SAFE_RELEASE(penum);
    SAFE_RELEASE(pdevice);
    SAFE_RELEASE(client);
    SAFE_RELEASE(render);
    #endif
    #if defined(__unix__)
    snd_pcm_close(pcm_handle);
    if (buffer) {
        delete[] buffer;
    }
    #endif
}

void gore::audiostream::playFile(audio file) {
    SoundP sp;
    sp.blockalign = file->blockalign;
    sp.data = file->data;
    sp.size = file->size;
    sp.byteFormat = file->byteFormat;
    sound_files.push_back(sp);
}
void gore::audiostream::streamFile(std::string file) {
    std::shared_ptr<FileStream> fs = std::make_shared<FileStream>(file);
    stream_files.push_back(fs);
}
void gore::audiostream::pause() {
    play = false;
    #if defined(__unix__)
    snd_pcm_drain(pcm_handle);
    #endif
    #if defined(_WIN32)
    client->Stop();
    #endif
}
void gore::audiostream::start() {
    play = true;
    #if defined(__unix__)
    snd_pcm_start(pcm_handle);
    #endif
    #if defined(_WIN32)
    client->Start();
    #endif
}
void gore::audiostream::reset() {
    #if defined(__unix__)
    snd_pcm_drop(pcm_handle);
    snd_pcm_prepare(pcm_handle);
    #endif
    #if defined(_WIN32)
    client->Reset();
    #endif
}


//Translator
//https://stackoverflow.com/questions/74596138/microsoft-wasapi-do-different-audio-formats-need-different-data-in-the-buffer
//https://github.com/adamstark/AudioFile/blob/master/AudioFile.h
//https://gist.github.com/endolith/e8597a58bcd11a6462f33fa8eb75c43d
//https://ccrma.stanford.edu/courses/422-winter-2014/projects/WaveFormat/

std::pair<float, float> calculateRange (gore::WavBytes org_bytes) {
    float orignalRangeLow, orignalRangeHigh;
    size_t origBytes = getByteSize(org_bytes);
    size_t bits = ((size_t)origBytes) * 8;
    if (org_bytes == gore::WavBytes::BYTE24) {
        bits = 24;
    }
    switch (org_bytes) {
        case gore::WavBytes::BYTE8:
            orignalRangeLow = 0.0f;
            orignalRangeHigh = 255.0f;
        break;
        case gore::WavBytes::FLOAT:
            orignalRangeLow = -1.0f;
            orignalRangeHigh = 1.0f;
        break;
        default:
            orignalRangeLow = -(std::pow(2, bits - 1));
            orignalRangeHigh = (std::pow(2, bits - 1) - 1);
        break;
    }
    return std::pair<float, float>(orignalRangeLow, orignalRangeHigh);
}

// 24 bit to 16 conversion can give weird harsh sound
// maybe fix one day, seems to edge case for me to care
float convertRange(float n, float OldMin, float OldMax, float NewMin, float NewMax) {
    float OldRange = (OldMax - OldMin);
    float NewRange = (NewMax - NewMin);
    float NewValue = (((n - OldMin) * NewRange) / OldRange) + NewMin;
    return std::clamp(NewValue, NewMin, NewMax);
}


void convertBits (char* mem, size_t size, char* n_mem, size_t n_size, gore::WavBytes org_bytes, gore::WavBytes new_bytes) {
    size_t origBytes = getByteSize(org_bytes);
    size_t newBytes = getByteSize(new_bytes);
    std::pair<float, float> originalRange = calculateRange(org_bytes);
    std::pair<float, float> newRange = calculateRange(new_bytes);
    for (size_t i = 0, j = 0; i < size && j < n_size - origBytes; i += origBytes, j += newBytes) {
        int32_t orgValue = 0;
        // grab the orignal bytes
        std::memcpy(&orgValue, (mem + i), origBytes);
        // have to retain sign from smaller sizes
        if (origBytes < 4 && origBytes > 1) {
            // grab highest bit to determine if signed
            // at which point we set to signed
            uint32_t bits = (origBytes * 8) - 1;
            uint32_t sig = orgValue & (1 << bits);
            if (sig) {
                uint32_t mask = 0xff000000;
                if (origBytes == 2) {
                    mask = 0xffff0000;
                }
                orgValue |= mask;
            }
            
        }
        float tf;
        // convert the value to a float
        if (org_bytes == gore::WavBytes::FLOAT) {
            std::memcpy(&tf, &orgValue, sizeof(float));
        } else {
            tf = orgValue;
        }
        float out = convertRange(tf, originalRange.first, originalRange.second, newRange.first, newRange.second);
        int32_t castOut = roundf(out);
        if (new_bytes == gore::WavBytes::FLOAT) {
            std::memcpy(&castOut, &out, sizeof(float));
        } else if (new_bytes == gore::WavBytes::BYTE24) {
            // padded 24 bits drop the msb
            castOut = castOut & 0x00ffffff;
        }
        // put into the new data output
        std::memcpy(n_mem + j, &castOut, newBytes);
    }
}

void* gore::audiostream::translate(void* mem, size_t size, size_t* n_size, gore::WavBytes org_bytes, gore::WavBytes new_bytes) {
    //return nullptr;
    void* mem2;
    uint32_t original_byte_size = getByteSize(org_bytes);
    uint32_t new_byte_size = getByteSize(new_bytes);
    if (org_bytes == new_bytes) {
        return nullptr;
    }
    else if (original_byte_size > new_byte_size) {
        if (new_bytes == gore::WavBytes::BYTE8) {
            *n_size = size / original_byte_size;
        } else {
            *n_size = size / new_byte_size;
        }
        mem2 = new char[*n_size];
    }
    else if (original_byte_size < new_byte_size) {
        *n_size = (size / original_byte_size) * (new_byte_size);
        mem2 = new char[*n_size];
    } else {
        *n_size = size;
        mem2 = new char[*n_size];
    }
    if (!mem2) {
        return nullptr;
    }
    convertBits((char*)mem, size, (char*)mem2, *n_size, org_bytes, new_bytes);

    return mem2;
}




gore::audiostream::FileStream::FileStream(std::string file) {
    fi.open(file, std::ios::binary);
    if (fi) {
        this->file = file;
        char c[40];
        fi.read(c, 20);
        pos += 20;
        uint16_t format;
        fi.read((char*)&format, 2);
        pos += 2;
        fi.read(c, 10);
        pos += 10;
        short bl;
        // read blockalign
        fi.read((char*)&bl, 2);
        blockalign = (int)bl;
        pos += 2;
        // read the bits per sample
        fi.read((char*)&bl, 2);
        this->byteFormat = (gore::WavBytes)(bl / 8);
        if (format == 3) {
             this->byteFormat = gore::WavBytes::FLOAT;
        } else if (bl / 8 == 3) {
             this->byteFormat = gore::WavBytes::BYTE24PACKED;
        }
        pos += 2;
        //now find the data section
        strMatch("data");
        //skip the four bytes of data size
        fi.read(c, 4);
        pos += 4;
    }
}
gore::audiostream::FileStream::~FileStream() {
    fi.close();
}
bool gore::audiostream::FileStream::writeData(uint8_t* dat, size_t n, size_t buffer_size, gore::WavBytes bits) {
    if (n_write) {
        return false;
    }
    char* d1 = new char[n * blockalign];
    if (d1 == nullptr) {
        return false;
    }
    fi.read((char*)d1, n * blockalign);
    size_t tt;
    char* da1 = (char*)translate(d1, n * blockalign, &tt, this->byteFormat, bits);
    
    if (da1 == nullptr) {
        std::memcpy(dat, d1, n * blockalign);
    }
    else {
        if (tt > buffer_size) {
            throw std::runtime_error("Trying to write data greater than audiostream buffer!");
        }
        std::memcpy(dat, da1, tt);
        delete[] da1;
    }
    delete[] d1;
    if (!fi) {
        n_write = true;
    }
    return true;
}
bool gore::audiostream::FileStream::strMatch(std::string str) {
    size_t i = 0;
    char c;
    while (true) {
        pos++;
        c = fi.get();
        if (c == str[i]) {
            i++;
            if (i >= str.size()) {
                return true;
            }
        }
        else {
            i = 0;
        }
        if (!fi) {
            break;
        }
    }
    return false;
}

bool gore::audiostream::SoundP::writeData(uint8_t* dat, size_t n, size_t buffer_size, gore::WavBytes bits) {
    if (n_write || pos >= size) {
        return false;
    }
    size_t translateSize = ((pos + (n * blockalign) >= size)) ? (size - pos) : n * blockalign;
    size_t tt;
    char* da1 = (char*)translate(data + pos, translateSize, &tt, this->byteFormat, bits);
    if (da1 == nullptr) {
        std::memcpy(dat, data + pos, n * blockalign);
    }
    else {
        if (tt > buffer_size) {
            throw std::runtime_error("Trying to write data greater than audiostream buffer!");
        }
        std::memcpy(dat, da1, tt);
        delete[] da1;
    }
    pos += (n * (blockalign));
    return true;
}


