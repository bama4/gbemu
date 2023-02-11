#pragma once
#include "../address.h"
#include "../definitions.h"
#include "../register.h"
#include <vector>

class Gameboy;

class Audio {

public:
    Audio(Gameboy &inGb);
    void tick(Cycles cycles);
    u8 read(const Address &address);
    void write(const Address &address, u8 byte);
    void write_io_register(const Address &address, u8 byte);

    /* Registers */
    /* Square 1  channel */
    ByteRegister channel_one_sweep;    /* NR10 */
    ByteRegister channel_one_duty;     /* NR11 */
    ByteRegister channel_one_envelope; /* NR12 */
    ByteRegister channel_one_freq_lo;  /* NR13 */
    ByteRegister channel_one_freq_hi;  /* NR14 */

    /* Square 2 channel */
    ByteRegister channel_two_duty;     /* NR21 */
    ByteRegister channel_two_envelope; /* NR22 */
    ByteRegister channel_two_freq_lo;  /* NR23 */
    ByteRegister channel_two_freq_hi;  /* NR24 */

    /* Wave Channel */
    ByteRegister channel_three_switch;  /* NR30 */
    ByteRegister channel_three_length;  /* NR31 */
    ByteRegister channel_three_level;   /* NR32 */
    ByteRegister channel_three_freq_lo; /* NR33 */
    ByteRegister channel_three_freq_hi; /* NR34 */

    /* Noise Channel */
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

    /* Timer vars */
    uint frame_seq_cycle_timer = 0;
    uint frame_seq_ctr = 0;

    /* Volume envelope timer */
    uint vol_env_cycle_timer = 0;

    uint8_t channel_one_vol_env_period_timer = 0;
    uint8_t channel_two_vol_env_period_timer = 0;
    uint8_t channel_four_vol_env_period_timer = 0;

    /* Length cycle timer */
    uint len_cycle_timer = 0;

    /* square 1 channel timer */
    uint square_one_cycle_timer = 0;
    uint square_one_cycle_timer_period = 0;

    /* square 2 channel timer */
    uint square_two_cycle_timer = 0;
    uint square_two_cycle_timer_period = 0;

    /* sweep timer */
    uint8_t channel_one_sweep_period_timer = 0;
    bool channel_one_sweep_internal_enabled = false;
    uint channel_one_freq_shadow = 0;

    uint16_t calc_frequency(uint8_t sweep_shift);
    void handle_length_ctr(void);
    void handle_vol_env_ctr(void);
    void process_init_sound_trigger(ByteRegister *channel_envelope_reg,
                                    uint8_t *period_timer);
    void process_sweep_trigger(void);
    void process_channel_1_trigger(void);
    void process_channel_2_trigger(void);
    void process_channel_3_trigger(void);
    void process_channel_4_trigger(void);
};

const uint AUDIO_RAM_SIZE = 0xf;
const uint AUDIO_MAX_VOL = 15;
const uint AUDIO_MIN_VOL = 0;

/* https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Frame_Sequencer */
const uint AUDIO_FRAME_SEQUENCER_RATE = 512;
const uint AUDIO_LEN_CTR_RATE = 256;
const uint AUDIO_VOL_ENVELOPE_RATE = 64;
const uint AUDIO_SWEEP_RATE = 128;

const uint AUDIO_FRAME_SEQUENCER_CYCLES =
    CLOCK_RATE / AUDIO_FRAME_SEQUENCER_RATE;
const uint AUDIO_LEN_CTR_CYCLES = CLOCK_RATE / AUDIO_LEN_CTR_RATE;
const uint AUDIO_VOL_ENVELOPE_CYCLES = CLOCK_RATE / AUDIO_VOL_ENVELOPE_RATE;
const uint AUDIO_SWEEP_CYCLES = CLOCK_RATE / AUDIO_SWEEP_RATE;