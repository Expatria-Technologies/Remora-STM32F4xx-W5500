#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define PRU_BASEFREQ    	75000           // PRU Base thread ISR update frequency (hz)
#define PRU_SERVOFREQ       1000            // PRU Servo thread ISR update freqency (hz)  Ideally this is 2x higher than the servo thread frequency of LCNC.

#define BASE_SLICE          0               // IRQ Slice used by the Base thread
#define SERVO_SLICE         1               // IRQ Slice used by the Servo thread

#define STEPBIT     		22            	// bit location in DDS accum
#define STEP_MASK   		(1L<<STEPBIT)

#define JSON_BUFF_SIZE	    10000			// Jason dynamic buffer size

#define JOINTS			    6				// Number of joints - set this the same as LinuxCNC HAL compenent. Max 8 joints
#define VARIABLES           2             	// Number of command values - set this the same as the LinuxCNC HAL compenent

#define PRU_DATA		    0x64617461 	    // "data" SPI payload
#define PRU_READ            0x72656164      // "read" SPI payload
#define PRU_WRITE           0x77726974      // "writ" SPI payload
#define PRU_ESTOP           0x65737470      // "estp" SPI payload
#define PRU_ACKNOWLEDGE		0x61636b6e	    // "ackn" payload
#define PRU_ERR		        0x6572726f	    // "erro" payload

#define DATA_ERR_MAX         40

#define X_STEP_PORT             GPIOA
#define X_STEP_PIN              3
#define Y_STEP_PORT             GPIOC
#define Y_STEP_PIN              1
#define Z_STEP_PORT             GPIOB
#define Z_STEP_PIN              8
#define A_STEP_PORT            GPIOD
#define A_STEP_PIN             2
#define B_STEP_PORT            GPIOB
#define B_STEP_PIN             14

#define X_DIRECTION_PORT        GPIOC
#define X_DIRECTION_PIN         2
#define Y_DIRECTION_PORT        GPIOC
#define Y_DIRECTION_PIN         0
#define Z_DIRECTION_PORT        GPIOC
#define Z_DIRECTION_PIN         15
#define A_DIRECTION_PORT       GPIOC
#define A_DIRECTION_PIN        12
#define B_DIRECTION_PORT       GPIOB
#define B_DIRECTION_PIN        15

// Data buffer configuration
#define BUFFER_SIZE 		68            	// Size of recieve buffer - same as HAL component, 64

#define PLL_SYS_KHZ (125 * 1000)    // 133MHz
#define SOCKET_MACRAW 0
#define PORT_LWIPERF 5001

// Location for storage of JSON config file in Flash
#define JSON_STORAGE_ADDRESS 0x8060000
#define JSON_UPLOAD_ADDRESS 0x8050000
#define USER_FLASH_LAST_PAGE_ADDRESS  	0x8060000
#define USER_FLASH_END_ADDRESS        	0x807FFFF
#define JSON_SECTOR FLASH_SECTOR_7

#define DEFAULT_CONFIG {0x7b,0x0a,0x09,0x22,0x42,0x6f,0x61,0x72,0x64,0x22,0x3a,0x20,0x22,0x46,0x6c,0x65,0x78,0x69,0x48,0x41,0x4c,0x22,0x2c,0x0a,0x09,0x22,0x4d,0x6f,0x64,0x75,0x6c,0x65,0x73,0x22,0x3a,0x5b,0x0a,0x09,0x7b,0x0a,0x09,0x22,0x54,0x68,0x72,0x65,0x61,0x64,0x22,0x3a,0x20,0x22,0x53,0x65,0x72,0x76,0x6f,0x22,0x2c,0x0a,0x09,0x22,0x54,0x79,0x70,0x65,0x22,0x3a,0x20,0x22,0x42,0x6c,0x69,0x6e,0x6b,0x22,0x2c,0x0a,0x09,0x09,0x22,0x43,0x6f,0x6d,0x6d,0x65,0x6e,0x74,0x22,0x3a,0x09,0x09,0x09,0x22,0x42,0x6c,0x69,0x6e,0x6b,0x79,0x22,0x2c,0x0a,0x09,0x09,0x22,0x50,0x69,0x6e,0x22,0x3a,0x09,0x09,0x09,0x09,0x22,0x50,0x43,0x31,0x33,0x22,0x2c,0x0a,0x09,0x09,0x22,0x46,0x72,0x65,0x71,0x75,0x65,0x6e,0x63,0x79,0x22,0x3a,0x20,0x09,0x09,0x34,0x0a,0x09,0x7d,0x0a,0x09,0x5d,0x0a,0x7d}

/* Default config contents:

{
	"Board": "FlexiHAL",
	"Modules":[
	{
	"Thread": "Servo",
	"Type": "Blink",
		"Comment":			"Blinky",
		"Pin":				"PC13",
		"Frequency": 		4
	}
	]
}

*/

#endif