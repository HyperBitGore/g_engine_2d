// Audio test: exercises audioplayer waveform generation, wav loading,
// playback, pause/resume, and streaming. No window required.
// Press Enter to advance through each step.
#include "../include/g_engine/audio/audio.hpp"
#include <chrono>
#include <iostream>
#include <thread>

static void waitMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

static void pressEnter(const char* msg) {
    std::cout << msg << " [Enter]\n";
    std::cin.ignore();
}

int main() {
    gore::audioplayer ap(4, gore::LogType::NONE);

    // --- generated waveforms ---
    gore::audio sin_aud  = ap.generateSin     (500, 440.0f, 44100);
    gore::audio sq_aud   = ap.generateSquare  (500, 440.0f, 44100);
    gore::audio tri_aud  = ap.generateTriangle(500, 440.0f, 44100);
    gore::audio saw_aud  = ap.generateSawtooth(500, 440.0f, 44100);

    pressEnter("Playing 440Hz sine wave (500ms)");
    ap.playFile(sin_aud, 0);
    waitMs(700);

    pressEnter("Playing 440Hz square wave (500ms)");
    ap.playFile(sq_aud, 0);
    waitMs(700);

    pressEnter("Playing 440Hz triangle wave (500ms)");
    ap.playFile(tri_aud, 0);
    waitMs(700);

    pressEnter("Playing 440Hz sawtooth wave (500ms)");
    ap.playFile(saw_aud, 0);
    waitMs(700);

    // --- pause / resume ---
    pressEnter("Playing sine (2s) — will pause after 500ms then resume");
    gore::audio long_sin = ap.generateSin(2000, 330.0f, 44100);
    ap.playFile(long_sin, 1);
    waitMs(500);
    ap.pause(1);
    std::cout << "  paused\n";
    waitMs(500);
    ap.start(1);
    std::cout << "  resumed\n";
    waitMs(1800);

    // --- wav file streaming ---
    pressEnter("Streaming resources/dungeonsynth5.wav on stream 2 (3s)");
    ap.playFile(std::string("resources/dungeonsynth5.wav"), 2);
    waitMs(3000);
    ap.clear(2);

    // --- wav loaded into memory ---
    pressEnter("Loading resources/sound_32.wav and playing on stream 3");
    gore::audio wav = ap.loadWavFile("resources/sound_32.wav");
    ap.playFile(wav, 3);
    waitMs(2000);

    std::cout << "Audio test complete.\n";
    ap.end();

    delete sin_aud;
    delete sq_aud;
    delete tri_aud;
    delete saw_aud;
    delete long_sin;
    delete wav;

    return 0;
}
