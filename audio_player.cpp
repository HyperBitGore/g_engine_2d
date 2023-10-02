#include "g_engine_2d.h"

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

//generates 1 second basic synth
Audio AudioPlayer::generateSound() {
	return nullptr;
}

void AudioPlayer::_RenderThread() {
    while (run) {
        mtx.lock();
        for (auto& i : sound_files) {
            streams[i.stream]->playFile(i.aud);
        }
        sound_files.clear();
        for(auto& i : stream_files) {
            streams[i.stream]->streamFile(i.file);
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
                        stream_files[i]->~FileStream();
                        FileStream* fp = stream_files[i];
                        stream_files.erase(stream_files.begin() + i);
                        delete fp;
                    }
                    else {
                        i++;
                    }
                }
                for (size_t i = 0; i < sound_files.size();) {
                    if (!sound_files[i].writeData(data, free)) {
                        sound_files.erase(sound_files.begin() + i);
                    }
                    else {
                        i++;
                    }
                }
                render->ReleaseBuffer(free, 0);
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

//Translator
//https://stackoverflow.com/questions/74596138/microsoft-wasapi-do-different-audio-formats-need-different-data-in-the-buffer
//https://github.com/adamstark/AudioFile/blob/master/AudioFile.h
//https://gist.github.com/endolith/e8597a58bcd11a6462f33fa8eb75c43d
//https://ccrma.stanford.edu/courses/422-winter-2014/projects/WaveFormat/
int AudioStream::Translator::convertRange(int n, int min, int max, int n_min, int n_max) {
    int orange = (n - min);
    int nrange = (n_max - n_min);
    int val = (((n - min) * nrange) / orange) + n_min;
    return val;
}
float AudioStream::Translator::convertRange(float n, float min, float max, float n_min, float n_max) {
    float orange = (n - min);
    float nrange = (n_max - n_min);
    float val = (((n - min) * nrange) / orange) + n_min;
    return val;
}
short AudioStream::Translator::convertRange(short n, short min, short max, short n_min, short n_max) {
    short orange = (n - min);
    short nrange = (n_max - n_min);
    short val = (((n - min) * nrange) / orange) + n_min;
    return val;
}
//redone
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
//redone
void AudioStream::Translator::convertToFloat(short* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size / 2;
    float* f_mem = (float*)n_mem;
    float div = powf(2, (15)) - 1;
    for (size_t i = 0, j = 0; i < s; i++, j++) {
        *(f_mem + j) = ((float)*(mem + i)) / (float)(div);
    }
}
//redo
//https://stackoverflow.com/questions/9896589/how-do-you-read-in-a-3-byte-size-value-as-an-integer-in-c
void AudioStream::Translator::convert24ToFloat(char* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size;
    float* f_mem = (float*)n_mem;
    uint8_t* m = (uint8_t*)mem;
    float div = powf(2, 23) - 1;
    for (size_t i = 0, j = 0; i < s; i+=3, j++) {
        int sam = m[i] << 16 | m[i + 1] << 8 | m[i + 2];
        sam = clamp<int>(sam, -8388607, 8388607);
        float o = ((float)sam) / div;
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
//redo this
void AudioStream::Translator::convertTo24bit(uint8_t* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size;
    uint8_t* m = mem;
    char* f_mem = (char*)n_mem;
    for (size_t i = 0, j = 0; i < s; i++, j+=3) {
        int p = convertRange(m[i], 0, 255, -8388608, 8388607);
        //*(f_mem + j) = 
        //*(f_mem + j) = ((int24)(short)*(mem + i));
    }
}
void AudioStream::Translator::convertTo24bit(short* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size;
    char* f_mem = (char*)n_mem;
    for (size_t i = 0, j = 0; i < s; i++, j++) {
        *(f_mem + j) = ((char) * (mem + i));
    }
}
void AudioStream::Translator::convertTo24bit(float* mem, size_t size, void* n_mem, size_t n_size) {
    size_t s = size;
    char* f_mem = (char*)n_mem;
    for (size_t i = 0, j = 0; i < s; i++, j++) {
        *(f_mem + j) = ((char)(int)*(mem + i));
    }
}