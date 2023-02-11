#include "audio.h"
#include "../address.h"
#include "../gameboy.h"

#include "../util/bitwise.h"
#include "../util/log.h"

using bitwise::check_bit;

Audio::Audio(Gameboy &inGb) : gb(inGb) {
    audio_ram = std::vector<u8>(AUDIO_RAM_SIZE);

    /* Channel 1 period */
    square_one_cycle_timer_period = (2048 - AUDIO_SWEEP_RATE) * 4;
}

void Audio::tick(Cycles cycles) {

    frame_seq_cycle_timer += cycles.cycles;
    vol_env_cycle_timer += cycles.cycles;
    len_cycle_timer += cycles.cycles;
    square_one_cycle_timer += cycles.cycles;

    /* Did frame sequencer clock? */
    if (frame_seq_cycle_timer >= AUDIO_FRAME_SEQUENCER_CYCLES) {
        frame_seq_cycle_timer =
            frame_seq_cycle_timer % AUDIO_FRAME_SEQUENCER_CYCLES;
        frame_seq_ctr++;
    }

    /* Did volume envelope timer clock? */
    if (vol_env_cycle_timer >= AUDIO_VOL_ENVELOPE_CYCLES) {
        vol_env_cycle_timer = vol_env_cycle_timer % AUDIO_VOL_ENVELOPE_CYCLES;

        // Do nothing if envelope period is 0
        // Do nothing except decrement envelop period timer if decrement does
        // not result in 0, else:
        // Increase/decrease volume dependng on mode
        // If new volume is not between 0 and 15 ignore and disable volume env
        handle_vol_env_ctr();
    }

    /* Did sweep timer clock? */
    if (square_one_cycle_timer >= AUDIO_SWEEP_CYCLES) {
        square_one_cycle_timer = square_one_cycle_timer % AUDIO_SWEEP_CYCLES;

        // Decrement
        // When ctr is zero, shifts left or right depending on mode
        // if frequency is in range 0 to 2047, write new frequency to memory
        // else, disable this channel and disable sweep
    }

    /* Did length counter timer clock? */
    if (len_cycle_timer >= AUDIO_LEN_CTR_CYCLES) {
        len_cycle_timer = len_cycle_timer % AUDIO_LEN_CTR_CYCLES;

        handle_length_ctr();
    }
    if (frame_seq_ctr == 0) {
        frame_seq_ctr = 8;

    } else if (frame_seq_ctr >= 8) {
        frame_seq_ctr = 0;
    }
}

u8 Audio::read(const Address &address) { return audio_ram.at(address.value()); }

void Audio::write(const Address &address, u8 value) {
    audio_ram.at(address.value()) = value;
}

void Audio::write_io_register(const Address &address, u8 byte) {

    switch (address.value()) {

        /* TODO: Audio - Channel 1: Tone & Sweep */
    case 0xFF10:
        channel_one_sweep.set(byte);
        return;

    case 0xFF11:
        channel_one_duty.set(byte);
        return;
    case 0xFF12:
        channel_one_envelope.set(byte);
        return;
    case 0xFF13:
        channel_one_freq_lo.set(byte);
        return;
    case 0xFF14:

    {
        bool channel_one_sound_init = channel_one_freq_hi.check_bit(7);
        channel_one_freq_hi.set(byte);

        /* Check if volume trigger should happen */
        /* If the init sound bit was flipped to enabled, we need
         * to adjust the period and period timer for this channel
         */
        if ((channel_one_freq_hi.check_bit(7) == true) &&
            (channel_one_sound_init == false)) {
            process_init_sound_trigger(&channel_one_envelope,
                                       &channel_one_vol_env_period_timer);
        }

        process_channel_1_trigger();
    }
        return;

    /* TODO: Audio - Channel 2: Tone */
    case 0xFF16:
        channel_two_duty.set(byte);
        return;
    case 0xFF17:
        channel_two_envelope.set(byte);
        return;
    case 0xFF18:
        channel_two_freq_lo.set(byte);
        return;
    case 0xFF19:

    {
        bool channel_two_sound_init = channel_two_freq_hi.check_bit(7);
        channel_two_freq_hi.set(byte);

        /* Check if volume trigger should happen */
        /* If the init sound bit was flipped to enabled, we need
         * to adjust the period and period timer for this channel
         */
        if ((channel_two_freq_hi.check_bit(7) == true) &&
            (channel_two_sound_init == false)) {
            process_init_sound_trigger(&channel_two_envelope,
                                       &channel_two_vol_env_period_timer);
        }
    }
        /* Check if trigger should happen */
        return;

    /* TODO: Audio - Channel 3: Wave Output */
    case 0xFF1A:
        channel_three_switch.set(byte);
        return;
    case 0xFF1B:
        channel_three_length.set(byte);
        return;
    case 0xFF1C:
        channel_three_level.set(byte);
        return;
    case 0xFF1D:
        channel_three_freq_lo.set(byte);
        return;
    case 0xFF1E:
        channel_three_freq_hi.set(byte);
        return;

        /* TODO: Audio - Channel 4: Noise */
    case 0xFF20:
        channel_four_length.set(byte);
        return;
    case 0xFF21:
        channel_four_envelope.set(byte);
        return;
    case 0xFF22:
        channel_four_poly_ctr.set(byte);
        return;
    case 0xFF23:

    {
        bool channel_four_sound_init = channel_four_init.check_bit(7);
        channel_four_init.set(byte);

        /* Check if volume trigger should happen */
        /* If the init sound bit was flipped to enabled, we need
         * to adjust the period and period timer for this channel
         */
        if ((channel_four_init.check_bit(7) == true) &&
            (channel_four_sound_init == false)) {
            process_init_sound_trigger(&channel_four_envelope,
                                       &channel_four_vol_env_period_timer);
        }

        /* Check if trigger should happen */
    }

        return;

    /* TODO: Audio - Channel control/ON-OFF/Volume */
    case 0xFF24:
        channel_ctrl.set(byte);
        return;

    /* TODO: Audio - Selection of sound output terminal */
    case 0xFF25:
        select_terminal.set(byte);
        return;

    /* TODO: Audio - Sound on/off */
    case 0xFF26:
        log_unimplemented("Wrote to sound on/off address 0x%x - 0x%x",
                          address.value(), byte);
        sound_switch.set(byte);
        return;
    }
}

uint16_t Audio::calc_frequency(uint8_t sweep_shift) {

    auto is_freq_subtract = channel_one_sweep.check_bit(3);

    /* Calculate new frequency and perform overflow check */
    auto new_frequency = channel_one_freq_shadow >> sweep_shift;

    if (is_freq_subtract) {
        new_frequency = channel_one_freq_shadow - new_frequency;
    } else {
        new_frequency = channel_one_freq_shadow + new_frequency;
    }
}

void Audio::handle_vol_env_ctr(void) {

    /* Applies to square and noise channels */
    /* get values for each for the channels */
    bool do_amplify[3] = {false, false, false};
    ByteRegister *channel_envelopes[3] = {
        &channel_one_envelope, &channel_two_envelope, &channel_four_envelope};
    uint8_t *envelope_period_timers[3] = {&channel_one_vol_env_period_timer,
                                          &channel_two_vol_env_period_timer,
                                          &channel_four_vol_env_period_timer};

    /* Amplify if the envelope up/down bit is set,
     * else attenuate. Do nothing if the resulting amplify or attenuate
     * would cause the volume value to fall outside of the range of
     * AUDIO_MIN_VOL - AUDIO_MAX_VOL
     */
    do_amplify[0] = channel_one_envelope.check_bit(3);
    do_amplify[1] = channel_two_envelope.check_bit(3);
    do_amplify[2] = channel_four_envelope.check_bit(3);

    uint8_t channel_vol_val = 0;
    ByteRegister *channel_envelope = nullptr;
    uint8_t envelope_period = 0;
    uint8_t *envelope_period_timer = nullptr;

    for (auto ctr = 0; ctr < 3; ctr++) {

        envelope_period = channel_envelope->value() & 0x07;
        envelope_period_timer = envelope_period_timers[ctr];

        /* Do not perform any changes to volume envelope if the period is 0
         * for a given channel
         */
        if (envelope_period == 0 || envelope_period == 8) {
            continue;
        }

        /* Do not perform any changes to the volume envelope if the period
         * timer has not gone off after decrementing
         */
        *envelope_period_timer = *envelope_period_timer - 1;
        if ((*envelope_period_timer) != 0) {
            continue;
        }

        /* Reload period timer with period */
        *envelope_period_timer = envelope_period;

        /* Get bits 4 - 7 (the volume value) */
        channel_vol_val = (channel_envelope->value() >> 4);
        channel_envelope = channel_envelopes[ctr];

        /* increment volume value by one if amplify true */
        if (do_amplify[ctr]) {
            if (channel_vol_val < AUDIO_MAX_VOL) {
                channel_vol_val++;
                channel_envelope->set((channel_envelope->value() & 0x0f) |
                                      (channel_vol_val << 4));
            }

            /* decrement volume value by one if amplify false (attenuate) */
        } else {
            if (channel_vol_val > AUDIO_MIN_VOL) {
                channel_vol_val--;
                channel_envelope->set((channel_envelope->value() & 0x0f) |
                                      (channel_vol_val << 4));
            }
        }
    }
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

void Audio::process_init_sound_trigger(ByteRegister *channel_envelope_reg,
                                       uint8_t *period_timer) {

    /* For envelope
     * Load envelope period into period timer
     */
    uint8_t envelope_period = channel_envelope_reg->value() & 0x07;
    *period_timer = envelope_period;
}

void Audio::process_sweep_trigger(void) {
    uint8_t sweep_period = 0;
    uint16_t frequency, new_frequency = 0;
    uint8_t sweep_shift = 0;
    bool is_freq_subtract = false;

    if (channel_one_sweep_period_timer > 0) {

        channel_one_sweep_period_timer--;

        /* Only move forward if timer went off */
        if (channel_one_sweep_period_timer == 0) {
            sweep_shift = (channel_one_sweep.value() & 0x07);

            /* Copy frequency into shadow register */
            frequency = (channel_one_freq_hi.value() & 0x07);

            /* Shift high bits of frequency in place */
            frequency = frequency << 8;

            /* copy frequency lower bits into place */
            frequency |= channel_one_freq_lo.value();

            /* Place frequency into shadow register */
            channel_one_freq_shadow = frequency;

            /* Get sweep period (bits 6 - 4) */
            sweep_period = (channel_one_sweep.value() & 0x70) >> 4;
            channel_one_sweep_period_timer =
                (sweep_period == 0) ? 8 : sweep_period;

            /* Set sweep enable flag if either the sweep period or shift are
             * non-zero
             */
            channel_one_sweep_internal_enabled =
                ((sweep_period != 0) || (sweep_shift != 0)) ? true : false;

            /* check if we need to calculate a new frequency */
            if ((channel_one_sweep_internal_enabled == true) &&
                sweep_period != 0) {
                new_frequency = calc_frequency(sweep_shift);

                /* Overflow check */
                if ((new_frequency <= 2047) && (sweep_shift != 0)) {
                    /* Update shadow frequency and channel 1 frequency with this
                     * new value */
                    channel_one_freq_shadow = new_frequency;

                    /* Set lower 8 frequency bits */
                    channel_one_freq_lo.set(new_frequency & 0xff);

                    /* Set higher 3 frequency bits */
                    /* Clear high bits first (clear first 3 bits) */
                    channel_one_freq_hi.set(channel_one_freq_hi.value() & 0xf8);

                    /* Set new high bits */
                    channel_one_freq_hi.set(channel_one_freq_hi.value() |
                                            ((new_frequency & 0x700) >> 8));

                    /* Disable channel sweep if new freq overflow */
                } else if (new_frequency > 2047) {
                    channel_one_sweep_internal_enabled = false;
                }

                /* Calculate frequency again and perform overflow check */
                new_frequency = calc_frequency(sweep_shift);

                if (new_frequency > 2047) {
                    channel_one_sweep_internal_enabled = false;
                }
            }
        }
    }
}

void Audio::process_channel_1_trigger(void) {
    // enable channel length
    channel_one_freq_hi.set_bit_to(6, 1);

    // if length counter is 0, set to 63 (256 for wave channel)
    auto nr_11_len_ctr = (channel_one_duty.value() & 0x3f);
    if (nr_11_len_ctr == 0) {
        channel_one_duty.set(channel_one_duty.value() | 0x3f);
    }

    // reload frequency timer with period

    // channel volume is reloaded from NRx2

    // wave channel position is 0

    // square 1 sweep  does things

    // if channels DAC is off, channel will then be disabled again
}

void Audio::process_channel_2_trigger(void) {
    channel_two_freq_hi.set_bit_to(6, 1);
}

void Audio::process_channel_3_trigger(void) {
    channel_three_freq_hi.set_bit_to(6, 1);
}

void Audio::process_channel_4_trigger(void) {

    channel_four_init.set_bit_to(6, 1);
}
