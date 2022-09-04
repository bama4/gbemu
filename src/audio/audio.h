#pragma once
#include "../address.h"
#include "../register.h"
#include <vector>

class Gameboy;

class Audio {

public:
    Audio(Gameboy &inGb);
    void tick(Cycles cycles);
    u8 read(const Address &address);
    void write(const Address &address, u8 byte);

    /* Registers */
    ByteRegister channel_one_sweep;    /* NR10 */
    ByteRegister channel_one_duty;     /* NR11 */
    ByteRegister channel_one_envelope; /* NR12 */
    ByteRegister channel_one_freq_lo;  /* NR13 */
    ByteRegister channel_one_freq_hi;  /* NR14 */

    ByteRegister channel_two_duty;     /* NR21 */
    ByteRegister channel_two_envelope; /* NR22 */
    ByteRegister channel_two_freq_lo;  /* NR23 */
    ByteRegister channel_two_freq_hi;  /* NR24 */

    ByteRegister channel_three_switch;  /* NR30 */
    ByteRegister channel_three_length;  /* NR31 */
    ByteRegister channel_three_level;   /* NR32 */
    ByteRegister channel_three_freq_lo; /* NR33 */
    ByteRegister channel_three_freq_hi; /* NR34 */

    ByteRegister channel_four_length;   /* NR41 */
    ByteRegister channel_four_envelope; /* NR42 */
    ByteRegister channel_four_poly_ctr; /* NR43 */
    ByteRegister channel_four_init;     /* NR44 */

    ByteRegister channel_ctrl;    /* NR50 */
    ByteRegister select_terminal; /* NR51 */
    ByteRegister sound_switch;    /* NR52 */

private:
    Gameboy &gb;

    std::vector<u8> audio_ram;

    uint frame_seq_cycle_ctr = 0;
    uint env_cycle_ctr = 0;
    uint len_cycle_ctr = 0;
    uint sweep_cycle_ctr = 0;
};

const uint AUDIO_RAM_SIZE = 0xf;

/* https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Frame_Sequencer */
const uint AUDIO_LEN_CTR_CYCLES = 256;
const uint AUDIO_VOL_ENVELOPE_CYCLES = 64;
const uint AUDIO_SWEEP_CYCLES = 128;
const uint AUDIO_FRAME_SEQUENCER_CYCLES = 512;