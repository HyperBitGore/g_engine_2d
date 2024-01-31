#include "audio.hpp"
#define SwapFourBytes(data)   \
( (((data) >> 24) & 0x000000FF) | (((data) >>  8) & 0x0000FF00) | \
  (((data) <<  8) & 0x00FF0000) | (((data) << 24) & 0xFF000000) ) 
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }


Audio AudioPlayer::loadWavFile(std::string file) {
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
    Audio ad = new Sound;
    ad->name = file;
    ad->size = datasize;
    ad->channels = (BYTE)num_channels;
    ad->samplebits = (BYTE)bitspps;
    ad->framesize = byterate;
    ad->blockalign = blockalign;
    ad->data = (char*)std::malloc(ad->size);
    if (ad->data) {
        std::memcpy(ad->data, c, ad->size);
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
void AudioPlayer::playFile(Audio file, size_t stream) {
    if (stream < streams.size()) {
        PAudio sp;
        sp.aud = file;
        sp.stream = stream;
        sound_files.push_back(sp);
    }
}

void AudioPlayer::playFile(std::string path, size_t stream) {
    if (stream < streams.size()) {
        FStream fp;
        fp.file = path;
        fp.stream = stream;
        stream_files.push_back(fp);
   }
}

void AudioPlayer::start(size_t stream) {
    commands.push_back({ 1, stream });
}

void AudioPlayer::clear(size_t stream) {
    commands.push_back({ 2, stream });
}
void AudioPlayer::pause(size_t stream) {
    commands.push_back({ 0, stream });
}

void AudioPlayer::end() {
    mtx.lock();
    run = false;
    mtx.unlock();
}

float sgn(float x) {
    if (x < 0.f) return -1.f;
    if (x > 0.f) return 1.f;
    return 0.f;
}

//generates sin wave, based on length given in milliseconds
Audio AudioPlayer::generateSin(size_t length, float freq, size_t sample_rate) {
    Audio a = new Sound;
    a->blockalign = 8;
    a->channels = 2;
    a->samplebits = 32;
    a->framesize = (sample_rate * 32 * 2) / 8;
    a->size = (length * (a->framesize / 1000));
    a->data = (char*)std::malloc(a->size);
    float* ff = (float*)a->data;
    size_t sample_size = a->size / 4;
    for (size_t i = 0; i < sample_size; i++) {
        float f = sinf((2.0f * (float)M_PI * freq) / sample_rate * i);
        *(ff + i) = f;
    }
	return a;
}
//generates square wave, based on length given in milliseconds
Audio AudioPlayer::generateSquare(size_t length, float freq, size_t sample_rate) {
    Audio a = new Sound;
    a->blockalign = 8;
    a->channels = 2;
    a->samplebits = 32;
    a->framesize = (sample_rate * 32 * 2) / 8;
    a->size = (length * (a->framesize / 1000));
    a->data = (char*)std::malloc(a->size);
    float* ff = (float*)a->data;
    size_t sample_size = a->size / 4;
    for (size_t i = 0; i < sample_size; i++) {
        float f = sgn(sinf((2.0f * (float)M_PI * freq) / sample_rate * i));
        *(ff + i) = f;
    }
    return a;
}
//generates triangle wave, based on length given in milliseconds
Audio AudioPlayer::generateTriangle(size_t length, float freq, size_t sample_rate) {
    Audio a = new Sound;
    a->blockalign = 8;
    a->channels = 2;
    a->samplebits = 32;
    a->framesize = (sample_rate * 32 * 2) / 8;
    a->size = (length * (a->framesize / 1000));
    a->data = (char*)std::malloc(a->size);
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

Audio AudioPlayer::generateSawtooth(size_t length, float freq, size_t sample_rate) {
    Audio a = new Sound;
    a->blockalign = 8;
    a->channels = 2;
    a->samplebits = 32;
    a->framesize = (sample_rate * 32 * 2) / 8;
    a->size = (length * (a->framesize / 1000));
    a->data = (char*)std::malloc(a->size);
    float* ff = (float*)a->data;
    size_t sample_size = a->size / 4;
    for (size_t i = 0; i < sample_size; i++) {
        float f = frac(freq / sample_rate * i);
        *(ff + i) = f;
    }
    return a;
}

void AudioPlayer::_RenderThread() {
    while (run) {
        mtx.lock();
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
        mtx.unlock();
    }
}

AudioPlayer::AudioPlayer(size_t n_streams) {
    for (size_t i = 0; i < n_streams; i++) {
        AudioStream* as = new AudioStream;
        streams.push_back(as);
    }

    rend_thread = std::thread(&AudioPlayer::_RenderThread, this);
}

AudioPlayer::~AudioPlayer() {
    rend_thread.join();
    run = false;
    for (size_t i = 0; i < streams.size(); i++) {
        AudioStream* as = streams[i];
        streams.erase(streams.begin() + i);
        delete as;
    }

}

void AudioStream::playStream() {
    if (play) {
        DWORD res = WaitForSingleObject(bufReady, 0);

        if (res == WAIT_OBJECT_0) {

            UINT32 filled;
            client->GetCurrentPadding(&filled);
            UINT32 free = buffer_size - filled;
            if (free > 0) {
                WavBytes bits = (WavBytes)((this->format->wBitsPerSample)/8);
                BYTE* data;
                render->GetBuffer(free, &data);
                for (size_t i = 0; i < stream_files.size();) {
                    if (!stream_files[i]->writeData(data, free, bits)) {
                        FileStream* fp = stream_files[i];
                        stream_files.erase(stream_files.begin() + i);
                        delete fp; //dont need to call destructor since delete does that for us
                    }
                    else {
                        i++;
                    }
                }
                for (size_t i = 0; i < sound_files.size();) {
                    if (!sound_files[i].writeData(data, free, bits)) {
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
    }
}

AudioStream::AudioStream() {
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


    hr = client->GetMixFormat(&format);

    if (FAILED(hr)) {
        return;
    }
    int buffer_length_msec = 10;
    REFERENCE_TIME dur = buffer_length_msec * 1000 * 10;
    hr = client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY, dur, dur, format, NULL);
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
}
AudioStream::~AudioStream() {
    CoTaskMemFree(format);
    SAFE_RELEASE(penum);
    SAFE_RELEASE(pdevice);
    SAFE_RELEASE(client);
    SAFE_RELEASE(render);
}

void AudioStream::playFile(Audio file) {
    SoundP sp;
    sp.blockalign = file->blockalign;
    sp.data = file->data;
    sp.size = file->size;
    sp.bytesp = (file->samplebits / 8);
    sound_files.push_back(sp);
}
void AudioStream::streamFile(std::string file) {
    FileStream* fs = new FileStream(file);
    stream_files.push_back(fs);
}
void AudioStream::pause() {
    play = false;
    client->Stop();
}
void AudioStream::start() {
    play = true;
    client->Start();
}
void AudioStream::reset() {
    client->Reset();
}


//Translator
//https://stackoverflow.com/questions/74596138/microsoft-wasapi-do-different-audio-formats-need-different-data-in-the-buffer
//https://github.com/adamstark/AudioFile/blob/master/AudioFile.h
//https://gist.github.com/endolith/e8597a58bcd11a6462f33fa8eb75c43d
//https://ccrma.stanford.edu/courses/422-winter-2014/projects/WaveFormat/
int AudioStream::Translator::convertRange(int n, int OldMin, int OldMax, int NewMin, int NewMax) {
    int OldRange = (OldMax - OldMin);
    int NewRange = (NewMax - NewMin);
    int NewValue = (((n - OldMin) * NewRange) / OldRange) + NewMin;
    return NewValue;
}
float AudioStream::Translator::convertRange(float n, float OldMin, float OldMax, float NewMin, float NewMax) {
    float OldRange = (OldMax - OldMin);
    float NewRange = (NewMax - NewMin);
    float NewValue = (((n - OldMin) * NewRange) / OldRange) + NewMin;
    return NewValue;
}
short AudioStream::Translator::convertRange(short n, short OldMin, short OldMax, short NewMin, short NewMax) {
    short OldRange = (OldMax - OldMin);
    short NewRange = (NewMax - NewMin);
    short NewValue = (((n - OldMin) * NewRange) / OldRange) + NewMin;
    return NewValue;
}
//fixed
void AudioStream::Translator::convertToFloat(uint8_t* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size;
    float* f_mem = (float*)n_mem;
    float div = powf(2, 7);
    for (size_t i = 0, j = 0; i < s; i++, j++) {
        float p = (float)*(mem + i) - div;
        p = p / (div - 1);
        *(f_mem + j) = p;
    }
}
//fixed
void AudioStream::Translator::convertToFloat(short* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size / 2;
    float* f_mem = (float*)n_mem;
    float pow = (1.0f / 32768.0f);
    for (size_t i = 0, j = 0; i < s && j < size; i++, j++) {
        float o = (mem[i]) * pow;
        f_mem[j] = o;
    }
}
//fixed
//https://stackoverflow.com/questions/9896589/how-do-you-read-in-a-3-byte-size-value-as-an-integer-in-c
void AudioStream::Translator::convert24ToFloat(uint8_t* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size;
    float* f_mem = (float*)n_mem;
    float pow = (1.0f / 8388607.0f);
    for (size_t i = 0, j = 0; i < s && j < n_size; i+=3, j++) {
        int sam = mem[i] | (mem[i + 1] << 8) | ((int8_t)mem[i + 2] << 16);
        //sam = clamp<int>(sam, -8388607, 8388607);
        float o = (float)sam * pow;
        *(f_mem + j) = o;
    }
}
//redone
void AudioStream::Translator::convertTo16bit(char* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size;
    short* f_mem = (short*)n_mem;
    for (size_t i = 0, j = 0; i < s; i++, j++) {
        *(f_mem + j) = convertRange((short)*(mem + i), (short)0, (short)255, (short)-32768, (short)32768);
    }
}
//redone
void AudioStream::Translator::convertTo16bit(float* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size / 4;
    short* f_mem = (short*)n_mem;
    for (size_t i = 0, j = 0; i < s; i++, j++) {
        *(f_mem + j) = (short)convertRange(*(mem + i), -1.0f, 1.0f, -32768.0f, 32768.0f);
    }
}
//redone
void AudioStream::Translator::convert24To16bit(char* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size;
    short* f_mem = (short*)n_mem;
    for (size_t i = 0, j = 0; i < s; i+=3, j++) {
        int p = mem[i] << 16 | mem[i + 1] << 8 | mem[i + 2];
        p = convertRange(p, -8388608, 8388607, -32768, 32767);
        *(f_mem + j) = ((short)p);
    }
}
//redone
void AudioStream::Translator::convertTo8bit(short* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size / 2;
    char* f_mem = (char*)n_mem;
    for (size_t i = 0, j = 0; i < s; i++, j++) {
        *(f_mem + j) = ((char)convertRange(*(mem + i), (short)-32768, (short)32768, (short)0, (short)255));
    }
}
//redone
void AudioStream::Translator::convert24To8bit(char* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size;
    char* f_mem = (char*)n_mem;
    for (size_t i = 0, j = 0; i < s; i+=3, j++) {
        int p = mem[i] << 16 | mem[i + 1] << 8 | mem[i + 2];
        p = convertRange(p, -8388608, 8388607, 0, 255);
        *(f_mem + j) = ((char)p);
    }
}
//redone
void AudioStream::Translator::convertTo8bit(float* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size / 4;
    char* f_mem = (char*)n_mem;
    for (size_t i = 0, j = 0; i < s; i++, j++) {
        *(f_mem + j) = ((char)convertRange(*(mem + i), -1.0f, 1.0f, 0.0f, 255.0f));
    }
}
//redone
void AudioStream::Translator::convertTo24bit(uint8_t* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size;
    uint8_t* m = mem;
    uint8_t* f_mem = (uint8_t*)n_mem;
    for (size_t i = 0, j = 0; i < s; i++, j+=3) {
        int p = convertRange(m[i], 0, 255, -8388608, 8388607);
        f_mem[j] = (p) & 0xff;
        f_mem[j + 1] = (p >> 8) & 0xff;
        f_mem[j + 2] = ((int8_t)p >> 16) & 0xff;
    }
}
//redone
void AudioStream::Translator::convertTo24bit(short* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size / 2;
    uint8_t* f_mem = (uint8_t*)n_mem;
    for (size_t i = 0, j = 0; i < s; i++, j+=3) {
        int p = convertRange((int)mem[i], -32768, 32768, -8388608, 8388607);
        f_mem[j] = (p) & 0xff;
        f_mem[j + 1] = (p >> 8) & 0xff;
        f_mem[j + 2] = ((int8_t)p >> 16) & 0xff;
    }
}
//redone
void AudioStream::Translator::convertTo24bit(float* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size;
    char* f_mem = (char*)n_mem;
    for (size_t i = 0, j = 0; i < s; i++, j++) {
        int p = (int)convertRange((float)mem[i], -1.0f, 1.0f, -8388608.0f, 8388607.0f);
        f_mem[j] = (p) & 0xff;
        f_mem[j + 1] = (p >> 8) & 0xff;
        f_mem[j + 2] = ((int8_t)p >> 16) & 0xff;
    }
}

void* AudioStream::Translator::translate(void* mem, size_t size, size_t* n_size, WavBytes org_bytes, WavBytes new_bytes) {
    //return nullptr;
    void* mem2;
    if (org_bytes == new_bytes) {
        return nullptr;
    }
    else if (org_bytes > new_bytes) {
        *n_size = size / (size_t)new_bytes;
        mem2 = std::malloc(*n_size);
    }
    else {
        *n_size = (size / (size_t)org_bytes) * ((size_t)new_bytes);
        mem2 = std::malloc(*n_size);
    }
    if (!mem2) {
        return nullptr;
    }
    switch (new_bytes) {
    case WavBytes::BYTE8:
        switch (org_bytes) {
        case WavBytes::BYTE16:
            convertTo8bit((short*)mem, size, mem2, *n_size);
            break;
        case WavBytes::BYTE24:
            convert24To8bit((char*)mem, size, mem2, *n_size);
            break;
        case WavBytes::BYTE32:
            convertTo8bit((float*)mem, size, mem2, *n_size);
            break;
        }
        break;
    case WavBytes::BYTE16:
        switch (org_bytes) {
        case WavBytes::BYTE8:
            convertTo16bit((char*)mem, size, mem2, *n_size);
            break;
        case WavBytes::BYTE24:
            convert24To16bit((char*)mem, size, mem2, *n_size);
            break;
        case WavBytes::BYTE32:
            convertTo16bit((float*)mem, size, mem2, *n_size);
            break;
        }
        break;
    case WavBytes::BYTE24:
        switch (org_bytes) {
        case WavBytes::BYTE8:
            convertTo24bit((uint8_t*)mem, size, mem2, *n_size);
            break;
        case WavBytes::BYTE16:
            convertTo24bit((short*)mem, size, mem2, *n_size);
            break;
        case WavBytes::BYTE32:
            convertTo24bit((float*)mem, size, mem2, *n_size);
            break;
        }
        break;
    case WavBytes::BYTE32:
        switch (org_bytes) {
        case WavBytes::BYTE8:
            convertToFloat((uint8_t*)mem, size, mem2, *n_size);
            break;
        case WavBytes::BYTE24:
            convert24ToFloat((uint8_t*)mem, size, mem2, *n_size);
            break;
        case WavBytes::BYTE16:
            convertToFloat((short*)mem, size, mem2, *n_size);
            break;
        }
        break;
    }

    return mem2;
}




AudioStream::FileStream::FileStream(std::string file) {
    fi.open(file, std::ios::binary);
    if (fi) {
        this->file = file;
        char c[40];
        fi.read(c, 32);
        pos += 32;
        short bl;
        fi.read((char*)&bl, 2);
        blockalign = (int)bl;
        pos += 2;
        fi.read((char*)&bl, 2);
        bytesp = (bl) / 8;
        pos += 2;
        //now find the data section
        strMatch("data");
        //skip the four bytes of data size
        fi.read(c, 4);
        pos += 4;
    }
}
AudioStream::FileStream::~FileStream() {
    fi.close();
}
bool AudioStream::FileStream::writeData(BYTE* dat, size_t n, WavBytes bits) {
    if (n_write) {
        return false;
    }
    void* d1 = std::malloc(n * blockalign);
    if (d1 == nullptr) {
        return false;
    }
    fi.read((char*)d1, n * blockalign);
    size_t tt;
    WavBytes w1 = (WavBytes)(bytesp);
    void* da1 = Translator::translate(d1, n * blockalign, &tt, w1, bits);
    if (da1 == nullptr) {
        std::memcpy(dat, d1, n * blockalign);
    }
    else {
        std::memcpy(dat, da1, tt);
        std::free(da1);
    }
    std::free(d1);
    if (!fi) {
        n_write = true;
    }
    return true;
}
bool AudioStream::FileStream::strMatch(std::string str) {
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

bool AudioStream::SoundP::writeData(BYTE* dat, size_t n, WavBytes bits) {
    if (n_write) {
        return false;
    }
    else if ((pos + (n * blockalign) >= size)) {
        size_t nt = (size - pos);
        size_t tt;
        WavBytes w1 = (WavBytes)(bytesp);
        void* da1 = Translator::translate(data + pos, n * blockalign, &tt, w1, bits);
        if (da1 == nullptr) {
            std::memcpy(dat, data + pos, nt - 1);
        }
        else {
            std::memcpy(dat, da1, tt);
            std::free(da1);
        }
        n_write = true;
        return false;
    }
    size_t tt;
    WavBytes w1 = (WavBytes)(bytesp);
    void* da1 = Translator::translate(data + pos, n * blockalign, &tt, w1, bits);
    if (da1 == nullptr) {
        std::memcpy(dat, data + pos, n * blockalign);
    }
    else {
        std::memcpy(dat, da1, tt);
        std::free(da1);
    }
    pos += (n * (blockalign));
    return true;
}


