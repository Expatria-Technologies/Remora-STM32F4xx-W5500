#ifndef STEPPER_H
#define STEPPER_H

//#include "main_init.h"

#ifndef true
#define false 0
#define true 1
#endif

#define BITBAND_PERI(x, b) (*((__IO uint8_t *) (PERIPH_BB_BASE + (((uint32_t)(volatile const uint32_t *)&(x)) - PERIPH_BASE)*32 + (b)*4)))
#define DIGITAL_OUT(port, pin, on) { BITBAND_PERI((port)->ODR, pin) = on; }
#define DIGITAL_IN(port, pin) BITBAND_PERI(port->IDR, pin)

#define Off 0
#define On 1

#define SOME_LARGE_VALUE 1.0E+38f
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define TOLERANCE_EQUAL 0.0001f

#define RADDEG  0.01745329251994329577f // Radians per degree
#define DEGRAD 57.29577951308232087680f // Degrees per radians
#define SQRT3   1.73205080756887729353f
#define SIN120  0.86602540378443864676f
#define COS120 -0.5f
#define TAN60   1.73205080756887729353f
#define SIN30   0.5f
#define TAN30   0.57735026918962576451f
#define TAN30_2 0.28867513459481288225f

#define ABORTED (sys.abort || sys.cancel)

// Convert character to uppercase
#define CAPS(c) ((c >= 'a' && c <= 'z') ? (c & 0x5F) : c)
#define LCAPS(c) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)

typedef union {
    uint8_t mask;
    uint8_t value;
    struct {
        uint8_t x :1,
                y :1,
                z :1,
                a :1,
                b :1,
                c :1,
                u :1,
                v :1;
    };
} axes_signals_t;

/*! \brief Holds the planner block Bresenham algorithm execution data for the segments in the segment buffer.
__NOTE:__ This data is copied from the prepped planner blocks so that the planner blocks may be
 discarded when entirely consumed and completed by the segment buffer. Also, AMASS alters this
 data for its own use. */
typedef struct st_block {
    uint_fast8_t id;                  //!< Id may be used by driver to track changes
    struct st_block *next;            //!< Pointer to next element in cirular list of blocks
    uint32_t steps[N_AXIS];
    uint32_t step_event_count;
    axes_signals_t direction_bits;
    float steps_per_mm;
    float millimeters;
    float programmed_rate;
} st_block_t;

typedef struct st_segment {
    uint_fast8_t id;                    //!< Id may be used by driver to track changes
    struct st_segment *next;            //!< Pointer to next element in cirular list of segments
    st_block_t *exec_block;             //!< Pointer to the block data for the segment
    uint32_t cycles_per_tick;           //!< Step distance traveled per ISR tick, aka step rate.
    float current_rate;
    uint_fast16_t n_step;               //!< Number of step events to be executed for this segment
    bool cruising;                      //!< True when in cruising part of profile, only set for spindle synced moves
    uint_fast8_t amass_level;           //!< Indicates AMASS level for the ISR to execute this segment
} segment_t;

// Axis array index values. Must start with 0 and be continuous.
#define X_AXIS 0 // Axis indexing value.
#define Y_AXIS 1
#define Z_AXIS 2
#define X_AXIS_BIT bit(X_AXIS)
#define Y_AXIS_BIT bit(Y_AXIS)
#define Z_AXIS_BIT bit(Z_AXIS)
#if N_AXIS > 3
#define A_AXIS 3
#define A_AXIS_BIT bit(A_AXIS)
#endif
#if N_AXIS > 4
#define B_AXIS 4
#define B_AXIS_BIT bit(B_AXIS)
#endif
#if N_AXIS > 5
#define C_AXIS 5
#define C_AXIS_BIT bit(C_AXIS)
#endif
#if N_AXIS > 6
#define U_AXIS 6
#define U_AXIS_BIT bit(U_AXIS)
#endif
#if N_AXIS == 8
#define V_AXIS 7
#define V_AXIS_BIT bit(V_AXIS)
#endif

#if N_AXIS == 3
#define AXES_BITMASK (X_AXIS_BIT|Y_AXIS_BIT|Z_AXIS_BIT)
#elif N_AXIS == 4
#define AXES_BITMASK (X_AXIS_BIT|Y_AXIS_BIT|Z_AXIS_BIT|A_AXIS_BIT)
#elif N_AXIS == 5
#define AXES_BITMASK (X_AXIS_BIT|Y_AXIS_BIT|Z_AXIS_BIT|A_AXIS_BIT|B_AXIS_BIT)
#elif N_AXIS == 6
#define AXES_BITMASK (X_AXIS_BIT|Y_AXIS_BIT|Z_AXIS_BIT|A_AXIS_BIT|B_AXIS_BIT|C_AXIS_BIT)
#elif N_AXIS == 7
#define AXES_BITMASK (X_AXIS_BIT|Y_AXIS_BIT|Z_AXIS_BIT|A_AXIS_BIT|B_AXIS_BIT|C_AXIS_BIT|U_AXIS_BIT)
#else
#define AXES_BITMASK (X_AXIS_BIT|Y_AXIS_BIT|Z_AXIS_BIT|A_AXIS_BIT|B_AXIS_BIT|C_AXIS_BIT|U_AXIS_BIT|V_AXIS_BIT)
#endif

#ifdef V_AXIS
#define N_ABC_AXIS 5
#elif defined(U_AXIS)
#define N_ABC_AXIS 4
#elif defined(C_AXIS)
#define N_ABC_AXIS 3
#elif defined(B_AXIS)
#define N_ABC_AXIS 2
#elif defined(A_AXIS)
#define N_ABC_AXIS 1
#else
#define N_ABC_AXIS 0
#endif

extern char const *const axis_letter[];

//! Stepper ISR data struct. Contains the running data for the main stepper ISR.
typedef struct stepper {
    uint32_t counter_x,             //!< Counter variable for the Bresenham line tracer, X-axis
             counter_y,             //!< Counter variable for the Bresenham line tracer, Y-axis
             counter_z              //!< Counter variable for the Bresenham line tracer, Z-axis
    #ifdef A_AXIS
           , counter_a              //!< Counter variable for the Bresenham line tracer, A-axis
    #endif
    #ifdef B_AXIS
           , counter_b              //!< Counter variable for the Bresenham line tracer, B-axis
    #endif
    #ifdef C_AXIS
           , counter_c              //!< Counter variable for the Bresenham line tracer, C-axis
    #endif
    #ifdef U_AXIS
           , counter_u              //!< Counter variable for the Bresenham line tracer, U-axis
    #endif
    #ifdef V_AXIS
           , counter_v              //!< Counter variable for the Bresenham line tracer, V-axis
    #endif
;
    bool new_block;                 //!< Set to true when a new block is started, might be referenced by driver code for advanced functionality.
    bool dir_change;                //!< Set to true on direction changes, might be referenced by driver for advanced functionality.
    axes_signals_t step_outbits;    //!< The stepping signals to be output.
    axes_signals_t dir_outbits;     //!< The direction signals to be output. The direction signals may be output only when \ref stepper.dir_change is true to reduce overhead.
    uint32_t steps[N_AXIS];         //!< Number of step pulse event events per axis step pulse generated.
    uint_fast8_t amass_level;       //!< AMASS level for this segment.
//    uint_fast16_t spindle_pwm;
    uint_fast16_t step_count;       //!< Steps remaining in line segment motion.
    uint32_t step_event_count;      //!< Number of step pulse events to be output by this segment.
    st_block_t *exec_block;         //!< Pointer to the block data for the segment being executed.
    segment_t *exec_segment;        //!< Pointer to the segment being executed.
} stepper_t;

#endif