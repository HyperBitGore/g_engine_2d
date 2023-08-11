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

void AudioPlayer::start() {
	//client->Start();
}

void AudioPlayer::clear() {
	client->Reset();
}
void AudioPlayer::pause() {
	//client->Stop();
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