#include "audio.h"
#include "../address.h"
#include "../gameboy.h"

#include "../util/bitwise.h"
#include "../util/log.h"

using bitwise::check_bit;

Audio::Audio(Gameboy &inGb) : gb(inGb) {
    audio_ram = std::vector<u8>(AUDIO_RAM_SIZE);
}

void Audio::tick(Cycles cycles) {
    frame_seq_cycle_ctr += cycles.cycles;
    env_cycle_ctr += cycles.cycles;
    len_cycle_ctr += cycles.cycles;
    sweep_cycle_ctr += cycles.cycles;

    /* Did volume envelope timer clock? */
    if (env_cycle_ctr >= AUDIO_VOL_ENVELOPE_CYCLES) {
        env_cycle_ctr = env_cycle_ctr % AUDIO_VOL_ENVELOPE_CYCLES;
    }

    /* Did sweep timer clock? */
    if (sweep_cycle_ctr >= AUDIO_SWEEP_CYCLES) {
        sweep_cycle_ctr = sweep_cycle_ctr % AUDIO_SWEEP_CYCLES;
    }

    /* Did length counter timer clock? */
    if (len_cycle_ctr >= AUDIO_LEN_CTR_CYCLES) {
        len_cycle_ctr = len_cycle_ctr % AUDIO_LEN_CTR_CYCLES;
    }

    /* Did frame sequencer clock? */
    if (frame_seq_cycle_ctr >= AUDIO_FRAME_SEQUENCER_CYCLES) {
        frame_seq_cycle_ctr =
            frame_seq_cycle_ctr % AUDIO_FRAME_SEQUENCER_CYCLES;
    }
}

u8 Audio::read(const Address &address) { return audio_ram.at(address.value()); }

void Audio::write(const Address &address, u8 value) {
    audio_ram.at(address.value()) = value;
}
