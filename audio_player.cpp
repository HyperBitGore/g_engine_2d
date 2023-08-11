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
    ad->channels = num_channels;
    ad->samplebits = bitspps;
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
                BYTE* data;
                render->GetBuffer(free, &data);
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
    int buffer_length_msec = 500;
    REFERENCE_TIME dur = buffer_length_msec * 1000 * 10;
    hr = client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, dur, dur, format, NULL);
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