#include "main_init.h"
#include "stepper.h"
#include "thread/timer.h"
#include "configuration.h"

#define ISR_CODE //!< Used by some drivers to force a function to always stay in RAM to improve performance.
#define ISR_FUNC(fn) fn
#define ADAPTIVE_MULTI_AXIS_STEP_SMOOTHING 1
#define SEGMENT_BUFFER_SIZE 10

// Stepper ISR data struct. Contains the running data for the main stepper ISR.
static stepper_t st;

static axes_signals_t next_step_outbits;
static uint32_t pulse_length, pulse_delay, aux_irq = 0;

int32_t position[N_AXIS];               //!< Real-time machine (aka home) position vector in steps.

#ifdef ADAPTIVE_MULTI_AXIS_STEP_SMOOTHING
typedef struct {
    uint32_t level_1;
    uint32_t level_2;
    uint32_t level_3;
} amass_t;

static amass_t amass;
#endif

// Holds the planner block Bresenham algorithm execution data for the segments in the segment
// buffer. Normally, this buffer is partially in-use, but, for the worst case scenario, it will
// never exceed the number of accessible stepper buffer segments (SEGMENT_BUFFER_SIZE-1).
// NOTE: This data is copied from the prepped planner blocks so that the planner blocks may be
// discarded when entirely consumed and completed by the segment buffer. Also, AMASS alters this
// data for its own use.
static st_block_t st_block_buffer[SEGMENT_BUFFER_SIZE - 1];

// Primary stepper segment ring buffer. Contains small, short line segments for the stepper
// algorithm to execute, which are "checked-out" incrementally from the first block in the
// planner buffer. Once "checked-out", the steps in the segments buffer cannot be modified by
// the planner, where the remaining planner block steps still can.
static segment_t segment_buffer[SEGMENT_BUFFER_SIZE];

// Step segment ring buffer pointers
static volatile segment_t *segment_buffer_tail;
static segment_t *segment_buffer_head, *segment_next_head;

inline static __attribute__((always_inline)) void stepperSetStepOutputs (axes_signals_t step_outbits)
{
    //step_outbits.mask ^= settings.steppers.step_invert.mask;
    DIGITAL_OUT(X_STEP_PORT, X_STEP_PIN, step_outbits.x);
    DIGITAL_OUT(Y_STEP_PORT, Y_STEP_PIN, step_outbits.y);
    DIGITAL_OUT(Z_STEP_PORT, Z_STEP_PIN, step_outbits.z);
  #ifdef A_AXIS
    DIGITAL_OUT(A_STEP_PORT, A_STEP_PIN, step_outbits.a);
  #endif
  #ifdef B_AXIS
    DIGITAL_OUT(B_STEP_PORT, B_STEP_PIN, step_outbits.b);
  #endif
  #ifdef C_AXIS
    DIGITAL_OUT(C_STEP_PORT, C_STEP_PIN, step_outbits.c);
  #endif
}

// Set stepper direction output pins
// NOTE: see note for stepperSetStepOutputs()
inline static __attribute__((always_inline)) void stepperSetDirOutputs (axes_signals_t dir_outbits)
{
    //dir_outbits.mask ^= settings.steppers.dir_invert.mask;
    DIGITAL_OUT(X_DIRECTION_PORT, X_DIRECTION_PIN, dir_outbits.x);
    DIGITAL_OUT(Y_DIRECTION_PORT, Y_DIRECTION_PIN, dir_outbits.y);
    DIGITAL_OUT(Z_DIRECTION_PORT, Z_DIRECTION_PIN, dir_outbits.z);
 #ifdef A_AXIS
    DIGITAL_OUT(A_DIRECTION_PORT, A_DIRECTION_PIN, dir_outbits.a);
 #endif
 #ifdef B_AXIS
    DIGITAL_OUT(B_DIRECTION_PORT, B_DIRECTION_PIN, dir_outbits.b);
 #endif
 #ifdef C_AXIS
    DIGITAL_OUT(C_DIRECTION_PORT, C_DIRECTION_PIN, dir_outbits.c);
 #endif
}

// Sets stepper direction and pulse pins and starts a step pulse.
static void stepperPulseStart(stepper_t * stepper){

    if(stepper->dir_change)
        stepperSetDirOutputs(stepper->dir_outbits);

    if(stepper->step_outbits.value) {
        stepperSetStepOutputs(stepper->step_outbits);
        PULSE_TIMER->EGR = TIM_EGR_UG;
        PULSE_TIMER->CR1 |= TIM_CR1_CEN;
    }
}

// Start a stepper pulse, delay version.
// Note: delay is only added when there is a direction change and a pulse to be output.
static void stepperPulseStartDelayed (stepper_t *stepper)
{
    if(stepper->dir_change) {

        stepperSetDirOutputs(stepper->dir_outbits);

        if(stepper->step_outbits.value) {
            next_step_outbits = stepper->step_outbits; // Store out_bits
            PULSE_TIMER->ARR = pulse_delay;
            PULSE_TIMER->EGR = TIM_EGR_UG;
            PULSE_TIMER->CR1 |= TIM_CR1_CEN;
        }

        return;
    }

    if(stepper->step_outbits.value) {
        stepperSetStepOutputs(stepper->step_outbits);
        PULSE_TIMER->EGR = TIM_EGR_UG;
        PULSE_TIMER->CR1 |= TIM_CR1_CEN;
    }
}

static void st_go_idle(stepper_t * stepper){
    
}

// Sets up stepper driver interrupt timeout, "Normal" version
static void stepperCyclesPerTick (uint32_t cycles_per_tick)
{
    STEPPER_TIMER->ARR = cycles_per_tick < (1UL << 20) ? cycles_per_tick : 0x000FFFFFUL;
}

//! \cond

ISR_CODE void ISR_FUNC(stepper_driver_interrupt_handler)(void)
{
    // Start a step pulse when there is a block to execute.
    if(st.exec_block) {

        stepperPulseStartDelayed(&st);

        st.new_block = st.dir_change = false;

        if (st.step_count == 0) // Segment is complete. Discard current segment.
            st.exec_segment = NULL;
    }

    // If there is no step segment, attempt to pop one from the stepper buffer
    if (st.exec_segment == NULL) {
        // Anything in the buffer? If so, load and initialize next step segment.
        if (segment_buffer_tail != segment_buffer_head) {

            // Initialize new step segment and load number of steps to execute
            st.exec_segment = (segment_t *)segment_buffer_tail;

            // Initialize step segment timing per step and load number of steps to execute.
            stepperCyclesPerTick(st.exec_segment->cycles_per_tick);
            st.step_count = st.exec_segment->n_step; // NOTE: Can sometimes be zero when moving slow.

            // If the new segment starts a new planner block, initialize stepper variables and counters.
            if (st.exec_block != st.exec_segment->exec_block) {

                if((st.dir_change = st.exec_block == NULL || st.dir_outbits.value != st.exec_segment->exec_block->direction_bits.value))
                    st.dir_outbits = st.exec_segment->exec_block->direction_bits;
                st.exec_block = st.exec_segment->exec_block;
                st.step_event_count = st.exec_block->step_event_count;
                st.new_block = true;

                // Initialize Bresenham line and distance counters
                st.counter_x = st.counter_y = st.counter_z
                #ifdef A_AXIS
                  = st.counter_a
                #endif
                #ifdef B_AXIS
                  = st.counter_b
                #endif
                #ifdef C_AXIS
                  = st.counter_c
                #endif
                #ifdef U_AXIS
                  = st.counter_u
                #endif
                #ifdef V_AXIS
                  = st.counter_v
                #endif
                  = st.step_event_count >> 1;

              #ifndef ADAPTIVE_MULTI_AXIS_STEP_SMOOTHING
                memcpy(st.steps, st.exec_block->steps, sizeof(st.steps));
              #endif
            }

          #ifdef ADAPTIVE_MULTI_AXIS_STEP_SMOOTHING
            // With AMASS enabled, adjust Bresenham axis increment counters according to AMASS level.
            st.amass_level = st.exec_segment->amass_level;
            st.steps[X_AXIS] = st.exec_block->steps[X_AXIS] >> st.amass_level;
            st.steps[Y_AXIS] = st.exec_block->steps[Y_AXIS] >> st.amass_level;
            st.steps[Z_AXIS] = st.exec_block->steps[Z_AXIS] >> st.amass_level;
           #ifdef A_AXIS
            st.steps[A_AXIS] = st.exec_block->steps[A_AXIS] >> st.amass_level;
           #endif
           #ifdef B_AXIS
            st.steps[B_AXIS] = st.exec_block->steps[B_AXIS] >> st.amass_level;
           #endif
           #ifdef C_AXIS
            st.steps[C_AXIS] = st.exec_block->steps[C_AXIS] >> st.amass_level;
           #endif
           #ifdef U_AXIS
            st.steps[U_AXIS] = st.exec_block->steps[U_AXIS] >> st.amass_level;
           #endif
           #ifdef V_AXIS
            st.steps[V_AXIS] = st.exec_block->steps[V_AXIS] >> st.amass_level;
           #endif
         #endif

        } else {
            // Segment buffer empty. Shutdown.
            st_go_idle(&st);

            st.exec_block = NULL;

            return; // Nothing to do but exit.
        }
    }

    register axes_signals_t step_outbits = (axes_signals_t){0};

    // Execute step displacement profile by Bresenham line algorithm

    st.counter_x += st.steps[X_AXIS];
    if (st.counter_x > st.step_event_count) {
        step_outbits.x = On;
        st.counter_x -= st.step_event_count;

        position[X_AXIS] = position[X_AXIS] + (st.dir_outbits.x ? -1 : 1);
    }

    st.counter_y += st.steps[Y_AXIS];
    if (st.counter_y > st.step_event_count) {
        step_outbits.y = On;
        st.counter_y -= st.step_event_count;

        position[Y_AXIS] = position[Y_AXIS] + (st.dir_outbits.y ? -1 : 1);
    }

    st.counter_z += st.steps[Z_AXIS];
    if (st.counter_z > st.step_event_count) {
        step_outbits.z = On;
        st.counter_z -= st.step_event_count;

        position[Z_AXIS] = position[Z_AXIS] + (st.dir_outbits.z ? -1 : 1);
    }

  #ifdef A_AXIS
      st.counter_a += st.steps[A_AXIS];
      if (st.counter_a > st.step_event_count) {
        step_outbits.a = On;
        st.counter_a -= st.step_event_count;

        position[A_AXIS] = position[A_AXIS] + (st.dir_outbits.a ? -1 : 1);
      }
  #endif

  #ifdef B_AXIS
      st.counter_b += st.steps[B_AXIS];
      if (st.counter_b > st.step_event_count) {
        step_outbits.b = On;
        st.counter_b -= st.step_event_count;
        position[B_AXIS] = position[B_AXIS] + (st.dir_outbits.b ? -1 : 1);
      }
  #endif

  #ifdef C_AXIS
      st.counter_c += st.steps[C_AXIS];
      if (st.counter_c > st.step_event_count) {
        step_outbits.c = On;
        st.counter_c -= st.step_event_count;

        position[C_AXIS] = position[C_AXIS] + (st.dir_outbits.c ? -1 : 1);
      }
  #endif

  #ifdef U_AXIS
    st.counter_u += st.steps[U_AXIS];
    if (st.counter_u > st.step_event_count) {
        step_outbits.u = On;
        st.counter_u -= st.step_event_count;

        position[U_AXIS] = position[U_AXIS] + (st.dir_outbits.u ? -1 : 1);
    }
  #endif

  #ifdef V_AXIS
    st.counter_v += st.steps[V_AXIS];
    if (st.counter_v > st.step_event_count) {
        step_outbits.v = On;
        st.counter_v -= st.step_event_count;

        position[V_AXIS] = position[V_AXIS] + (st.dir_outbits.v ? -1 : 1);
    }
  #endif

    st.step_outbits.value = step_outbits.value;

    if (st.step_count == 0 || --st.step_count == 0) {
        // Segment is complete. Advance segment tail pointer.
        segment_buffer_tail = segment_buffer_tail->next;
    }
}

// Main stepper driver
void STEPPER_TIMER_IRQHandler (void)
{
    if((STEPPER_TIMER->SR & TIM_SR_UIF) != 0) {    // check interrupt source
        STEPPER_TIMER->SR = ~TIM_SR_UIF;            // clear UIF flag
        stepper_driver_interrupt_handler();
    }
}

// This interrupt is used only when STEP_PULSE_DELAY is enabled. Here, the step pulse is
// initiated after the STEP_PULSE_DELAY time period has elapsed. The ISR TIMER2_OVF interrupt
// will then trigger after the appropriate settings.pulse_microseconds, as in normal operation.
// The new timing between direction, step pulse, and step complete events are setup in the
// st_wake_up() routine.

// This interrupt is enabled when Grbl sets the motor port bits to execute
// a step. This ISR resets the motor port after a short period (settings.pulse_microseconds)
// completing one step cycle.
void PULSE_TIMER_IRQHandler (void)
{
    PULSE_TIMER->SR &= ~TIM_SR_UIF;                 // Clear UIF flag

    if(PULSE_TIMER->ARR == pulse_delay) {          // Delayed step pulse?
        PULSE_TIMER->ARR = pulse_length;
        stepperSetStepOutputs(next_step_outbits);   // begin step pulse
        PULSE_TIMER->EGR = TIM_EGR_UG;
        PULSE_TIMER->CR1 |= TIM_CR1_CEN;
    } else
        stepperSetStepOutputs((axes_signals_t){0}); // end step pulse
}

void init_stepper (void){
    pulse_length = (uint32_t)(10.0f * (5)) - 1;
    PULSE_TIMER->ARR = pulse_length;
    PULSE_TIMER->EGR = TIM_EGR_UG;
}

//! \endcond