#include "audio.h"
#include "../address.h"
#include "../gameboy.h"

Audio::Audio(Gameboy &inGb) : gb(inGb) {
    audio_ram = std::vector<u8>(AUDIO_RAM_SIZE);
}

u8 Audio::read(const Address &address) { return audio_ram.at(address.value()); }

void Audio::write(const Address &address, u8 value) {
    audio_ram.at(address.value()) = value;
}
