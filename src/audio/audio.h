#pragma once
#include "../address.h"
#include "../register.h"
#include <vector>

class Gameboy;

class Audio {

public:
    Audio(Gameboy &inGb);
    u8 read(const Address &address);
    void write(const Address &address, u8 byte);

private:
    Gameboy &gb;

    /* Registers */
    ByteRegister mode_one_sweep;    /* NR10 */
    ByteRegister mode_one_duty;     /* NR11 */
    ByteRegister mode_one_envelope; /* NR12 */
    ByteRegister mode_one_freq_lo;  /* NR13 */
    ByteRegister mode_one_freq_hi;  /* NR14 */

    ByteRegister mode_two_duty;     /* NR21 */
    ByteRegister mode_two_envelope; /* NR22 */
    ByteRegister mode_two_freq_lo;  /* NR23 */
    ByteRegister mode_two_freq_hi;  /* NR24 */

    ByteRegister mode_three_switch;  /* NR30 */
    ByteRegister mode_three_length;  /* NR31 */
    ByteRegister mode_three_level;   /* NR32 */
    ByteRegister mode_three_freq_lo; /* NR33 */
    ByteRegister mode_three_freq_hi; /* NR34 */

    ByteRegister mode_four_length;   /* NR41 */
    ByteRegister mode_four_envelope; /* NR42 */
    ByteRegister mode_four_poly_ctr; /* NR43 */
    ByteRegister mode_four_init;     /* NR44 */

    ByteRegister channel_ctrl;    /* NR50 */
    ByteRegister select_terminal; /* NR51 */
    ByteRegister sound_switch;    /* NR52 */

    std::vector<u8> audio_ram;
};

const uint AUDIO_RAM_SIZE = 0xf;