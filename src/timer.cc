#include "timer.h"
#include "gameboy.h"

Timer::Timer(Gameboy &inGb) : gb(inGb) {}

void Timer::tick(Cycles cycles) {
    u8 new_divider = static_cast<u8>(divider.value() + cycles.cycles);
    divider.set(new_divider);

    cycle_counter += cycles.cycles;

    // Get input clock select bits
    TimerInputClkSelectMode t_timer_clk_sel_mode =
        static_cast<TimerInputClkSelectMode>(timer_control.value() & 0x3);

    switch (t_timer_clk_sel_mode) {
    case TimerInputClkSelectMode::CYCLES_4096:
        if (cycle_counter >= CLOCKS_PER_TIMER_MODE_0) {
            cycle_counter = cycle_counter % CLOCKS_PER_TIMER_MODE_0;
            inc_timer_counter();
        }
        break;
    case TimerInputClkSelectMode::CYCLES_262144:
        if (cycle_counter >= CLOCKS_PER_TIMER_MODE_1) {
            cycle_counter = cycle_counter % CLOCKS_PER_TIMER_MODE_1;
            inc_timer_counter();
        }
        break;

    case TimerInputClkSelectMode::CYCLES_65536:
        if (cycle_counter >= CLOCKS_PER_TIMER_MODE_2) {
            cycle_counter = cycle_counter % CLOCKS_PER_TIMER_MODE_2;
            inc_timer_counter();
        }
        break;

    case TimerInputClkSelectMode::CYCLES_16384:
        if (cycle_counter >= CLOCKS_PER_TIMER_MODE_3) {
            cycle_counter = cycle_counter % CLOCKS_PER_TIMER_MODE_3;
            inc_timer_counter();
        }
        break;
    }

    /* Check timer counter */
    // if timer has overflow bit set
    // set interrupt enable bit
}

auto Timer::get_divider() const -> u8 { return divider.value(); }

auto Timer::get_timer() const -> u8 { return timer_counter.value(); }

auto Timer::get_timer_modulo() const -> u8 { return timer_modulo.value(); }

auto Timer::get_timer_control() const -> u8 { return timer_control.value(); }

auto Timer::get_timer_counter() const -> u8 { return timer_counter.value(); }

void Timer::reset_divider() { divider.set(0x0); }

void Timer::set_timer_modulo(u8 value) { timer_modulo.set(value); }

void Timer::set_timer_control(u8 value) { timer_control.set(value); }

void Timer::inc_timer_counter() {
    auto curr_timer_ctr_val = timer_counter.value();
    auto new_timer_ctr = curr_timer_ctr_val + 1;
    timer_counter.set(new_timer_ctr);

    /* check for timer counter overflow */
    if (new_timer_ctr < curr_timer_ctr_val) {

        // Set overflow bit for interrupt flag timer overflow
        // 2 is the position of the bit
        gb.cpu.interrupt_flag.set_bit_to(2, true);
    }
}
