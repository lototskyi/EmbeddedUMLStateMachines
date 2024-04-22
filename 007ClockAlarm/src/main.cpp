#include <Arduino.h>
#include "qpn.h"
#include "ClockAlarm_SM.h"
#include "main.h"
#include "lcd.h"
// #include "avr8-stub.h"

static void display_init(void);
static void Timer1_setup(void);
static uint8_t process_button_pad_value(uint8_t btn_pad_value);

Q_DEFINE_THIS_FILE;

void setup() {
	// debug_init();
	Serial.begin(9600);
	//Serial.println("<<<ClockAlarm Application>>>");
	display_init();
  	Clock_Alarm_ctor();
  	QHSM_INIT(super_ClockAlarm);
  	Timer1_setup();

	// pinMode(PIN_BUTTON1, INPUT);
    // pinMode(PIN_BUTTON2, INPUT);
	pinMode(4, OUTPUT);
	digitalWrite(4, HIGH);
}

void loop() {
	
	static uint32_t tick_time = millis();
	static uint32_t alarm_check_time = millis();

	uint8_t volatile b1, b2, btn_pad_value;
	b1 = digitalRead(PIN_BUTTON1);
    b2 = digitalRead(PIN_BUTTON2);
	
	btn_pad_value = (b1 << 1U) | b2;

	btn_pad_value = process_button_pad_value(btn_pad_value);

	while(millis() - tick_time >= 50) {
		//send TICK event
		tick_time = millis();
		Q_SIG(super_ClockAlarm) = TICK_SIG;
		QHSM_DISPATCH(super_ClockAlarm);
	}

	while(millis() - alarm_check_time >= 500) {
		//send ALARM event
		alarm_check_time = millis();
		Q_SIG(super_ClockAlarm) = ALARM_SIG;
		QHSM_DISPATCH(super_ClockAlarm);
	}

	if (btn_pad_value) {
	
		if (btn_pad_value == BTN_PAD_VALUE_SET) {
			Q_SIG(super_ClockAlarm) = SET_SIG; 
		} else if (btn_pad_value == BTN_PAD_VALUE_OK) {
			Q_SIG(super_ClockAlarm) = OK_SIG;
		} else if (btn_pad_value == BTN_PAD_VALUE_ABRT) {
			Q_SIG(super_ClockAlarm) = ABRT_SIG;
		} else { 
			Q_SIG(super_ClockAlarm) = IGNORE_SIG;
		}

		QHSM_DISPATCH(super_ClockAlarm);
    }
}

static void Timer1_setup(void) 
{
	TCCR1A = 0;				//CTC mode
	TCCR1B = B00001100;		//prescaler = 256, CTC mode
	TIMSK1 |= B00000010;	//Interrupt enable for OCR1A compare match
	OCR1A = 6250 - 1;		//OC match value for 100ms time base generation
}

static void display_init(void)
{
	lcd_begin(16,2);
	lcd_clear();
	lcd_move_cursor_L_to_R();
	lcd_set_cursor(0,0);
	lcd_no_auto_scroll();
	lcd_cursor_off();
}

static uint8_t process_button_pad_value(uint8_t btn_pad_value)
{
    static button_state_t btn_sm_state = NOT_PRESSED;
    static uint32_t curr_time = millis();

    switch (btn_sm_state) {
        case NOT_PRESSED:
            if (btn_pad_value) {
                btn_sm_state = BOUNCE;
                curr_time = millis();
            }
            break;
        case BOUNCE:
            if (millis() - curr_time >=50) {
                if (btn_pad_value) {
                    btn_sm_state = PRESSED;
                    return btn_pad_value;
                } else {
                    btn_sm_state = NOT_PRESSED;
                }
            }
            break;
        case PRESSED:
            if (!btn_pad_value) {
                btn_sm_state = BOUNCE;
                curr_time = millis();
            }
            break;
    }

    return 0;
}

Q_NORETURN Q_onAssert	(	char_t const Q_ROM *const 	module,int_t const 	location ){
	Serial.println("Assertion failure!!");
	Serial.println((String)module);
	Serial.println(location);
	while(1);
}	
