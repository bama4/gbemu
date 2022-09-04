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

        // Decrement volume envelope
        // If zero, increase/decrease volume dependng on mode
        // If new volume is not between 0 and 15 ignore and disable volume env
    }

    /* Did sweep timer clock? */
    if (sweep_cycle_ctr >= AUDIO_SWEEP_CYCLES) {
        sweep_cycle_ctr = sweep_cycle_ctr % AUDIO_SWEEP_CYCLES;

        // Decrement
        // When ctr is zero, shifts left or right depending on mode
        // if frequency is in range 0 to 2047, write new frequency to memory
        // else, disable this channel and disable sweep
    }

    /* Did length counter timer clock? */
    if (len_cycle_ctr >= AUDIO_LEN_CTR_CYCLES) {
        len_cycle_ctr = len_cycle_ctr % AUDIO_LEN_CTR_CYCLES;

        handle_length_ctr();
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

void Audio::handle_length_ctr(void) {

    /* get values for each fo the four channels */
    bool checked_len_enable[4] = {false, false, false, false};

    // Decrement length counter if enabled
    // if counter is zero, disable channel
    checked_len_enable[0] = channel_one_freq_hi.check_bit(6);
    checked_len_enable[1] = channel_two_freq_hi.check_bit(6);
    checked_len_enable[2] = channel_three_freq_hi.check_bit(6);
    checked_len_enable[3] = channel_four_init.check_bit(6);

    /* CHANNEL 1 */
    if (checked_len_enable[0]) {
        {
            // Get the length ctr value bits 0 - 5
            auto nr_11_len_ctr = (channel_one_duty.value() & 0x3f);
            nr_11_len_ctr = nr_11_len_ctr - 1;

            // if counter is zero, disable channel
            if (nr_11_len_ctr == 0) {
                channel_one_freq_hi.set_bit_to(6, 0);
            }
        }
    }

    /* CHANNEL 2 */
    if (checked_len_enable[1]) {

        {
            // Get the length ctr value bits 0 - 5
            auto nr_21_len_ctr = (channel_two_duty.value() & 0x3f);
            nr_21_len_ctr = nr_21_len_ctr - 1;

            // if counter is zero, disable channel
            if (nr_21_len_ctr == 0) {
                channel_two_freq_hi.set_bit_to(6, 0);
            }
        }
    }

    /* CHANNEL 3 */
    if (checked_len_enable[2]) {
        {
            // Get the length ctr value bits 0 - 7
            auto nr_31_len_ctr = (channel_three_length.value());
            nr_31_len_ctr = nr_31_len_ctr - 1;

            // if counter is zero, disable channel
            if (nr_31_len_ctr == 0) {
                channel_three_freq_hi.set_bit_to(6, 0);
            }
        }
    }

    /* CHANNEL 4 */
    if (checked_len_enable[3]) {
        {
            // Get the length ctr value bits 0 - 7
            auto nr_41_len_ctr = (channel_four_length.value() & 0x3f);
            nr_41_len_ctr = nr_41_len_ctr - 1;

            // if counter is zero, disable channel
            if (nr_41_len_ctr == 0) {
                channel_four_init.set_bit_to(6, 0);
            }
        }
    }
}
