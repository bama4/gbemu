#pragma once

#include "definitions.h"
#include "register.h"

class Gameboy;

enum class TimerInputClkSelectMode {
    CYCLES_4096 = 0x0,
    CYCLES_262144 = 0x1,
    CYCLES_65536 = 0x2,
    CYCLES_16384 = 0x3,
};

class Timer {
public:
    Timer(Gameboy &inGb);
    void tick(uint cycles);

    auto get_divider() const -> u8;
    auto get_timer() const -> u8;
    auto get_timer_modulo() const -> u8;
    auto get_timer_control() const -> u8;
    auto get_timer_counter() const -> u8;

    void reset_divider();
    void set_timer_modulo(u8 value);
    void set_timer_control(u8 value);
    void inc_timer_counter();

private:
    ByteRegister divider;
    ByteRegister timer_counter;

    ByteRegister timer_modulo;
    ByteRegister timer_control;

    Gameboy &gb;

    /* track clock cycles */
    uint cycle_counter = 0;
};

/* See 0xFF07 TAC (Input Clock Select) */
const uint CLOCKS_PER_TIMER_MODE_0 = 4096;
const uint CLOCKS_PER_TIMER_MODE_1 = 262144;
const uint CLOCKS_PER_TIMER_MODE_2 = 65536;
const uint CLOCKS_PER_TIMER_MODE_3 = 16384;
