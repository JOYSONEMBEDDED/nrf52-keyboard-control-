
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <nrf_delay.h>
#include "nrf.h"
#include "nrf_drv_usbd.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_drv_power.h"
#include "app_timer.h"
#include <string.h>
#include "app_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd_hid_kbd.h"
#include "app_error.h"
#include "bsp.h"


#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
volatile bool send_keys_flag = false;

volatile bool send_now = false;
static bool waiting_for_release = false;
#define BUTTON_DETECTION_TIME    10
char var[] = "how are you";
#define UART_PRINTING_ENABLED   // comment to disable the UART loggin
uint8_t hid_key = 0;

/**
 * All the keys are available in the app_usbd_hid_kbd.h file
 * Located under components\libraries\usbd\class\hid\kbd\
 */

#define BUTTON1_CONFIG  APP_USBD_HID_KBD_A
#define BUTTON2_CONFIG  APP_USBD_HID_KBD_B
#define BUTTON3_CONFIG  APP_USBD_HID_KBD_C
#define BUTTON4_CONFIG  APP_USBD_HID_KBD_D

/**
 * @brief Enable USB power detection
 */
#ifndef USBD_POWER_DETECTION
#define USBD_POWER_DETECTION true
#endif

#define LED_CAPSLOCK       (BSP_BOARD_LED_0) /**< CAPSLOCK */
#define LED_NUMLOCK        (BSP_BOARD_LED_1) /**< NUMLOCK */
#define LED_HID_REP        (BSP_BOARD_LED_2) /**< Changes its state if any HID report was received or transmitted */
#define LED_USB_START      (BSP_BOARD_LED_3) /**< The USBD library has been started and the bus is not in SUSPEND state */

/**
 * @brief USB composite interfaces
 */
#define APP_USBD_INTERFACE_KBD   1

/**
 * @brief User event handler, HID keyboard
 */
static void hid_kbd_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_hid_user_event_t event);


/**
 * @brief Global HID keyboard instance
 */
APP_USBD_HID_KBD_GLOBAL_DEF(m_app_hid_kbd,
                            APP_USBD_INTERFACE_KBD,
                            NRF_DRV_USBD_EPIN2,
                            hid_kbd_user_ev_handler,
                            APP_USBD_HID_SUBCLASS_BOOT
);

/**
 * @brief keyboard status
 */

static void kbd_status(void)
{
    if(app_usbd_hid_kbd_led_state_get(&m_app_hid_kbd, APP_USBD_HID_KBD_LED_NUM_LOCK))
    {
        bsp_board_led_on(LED_NUMLOCK);
    }
    else
    {
        bsp_board_led_off(LED_NUMLOCK);
    }

    if(app_usbd_hid_kbd_led_state_get(&m_app_hid_kbd, APP_USBD_HID_KBD_LED_CAPS_LOCK))
    {
        bsp_board_led_on(LED_CAPSLOCK);
    }
    else
    {
        bsp_board_led_off(LED_CAPSLOCK);
    }
}

/**
 * @brief Class specific event handler.
 *
 * @param p_inst    Class instance.
 * @param event     Class specific event.
 * */
static void hid_kbd_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_hid_user_event_t event)
{
    UNUSED_PARAMETER(p_inst);
    switch (event) {
        case APP_USBD_HID_USER_EVT_OUT_REPORT_READY:
            /* Only one output report IS defined for HID keyboard class. Update LEDs state. */
            bsp_board_led_invert(LED_HID_REP);
            kbd_status();
            break;
        case APP_USBD_HID_USER_EVT_IN_REPORT_DONE:
            bsp_board_led_invert(LED_HID_REP);
            send_now=false;
            break;
        case APP_USBD_HID_USER_EVT_SET_BOOT_PROTO:
            UNUSED_RETURN_VALUE(hid_kbd_clear_buffer(p_inst));
            break;
        case APP_USBD_HID_USER_EVT_SET_REPORT_PROTO:
            UNUSED_RETURN_VALUE(hid_kbd_clear_buffer(p_inst));
            break;
        default:
            break;
    }
}


/**
 * @brief USBD library specific event handler.
 *
 * @param event     USBD library event.
 * */
static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    switch (event)
    {
        case APP_USBD_EVT_DRV_SOF:
            break;
        case APP_USBD_EVT_DRV_SUSPEND:
            app_usbd_suspend_req(); // Allow the library to put the peripheral into sleep mode
            bsp_board_leds_off();
            break;
        case APP_USBD_EVT_DRV_RESUME:
            bsp_board_led_on(LED_USB_START);
            kbd_status(); /* Restore LED state - during SUSPEND all LEDS are turned off */
            break;
        case APP_USBD_EVT_STARTED:
            bsp_board_led_on(LED_USB_START);
            break;
        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            bsp_board_leds_off();
            break;
        case APP_USBD_EVT_POWER_DETECTED:
            #ifdef UART_PRINTING_ENABLED
            NRF_LOG_INFO("USB power detected");
            #endif

            if (!nrf_drv_usbd_is_enabled())
            {
                app_usbd_enable();
            }
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            #ifdef UART_PRINTING_ENABLED
            NRF_LOG_INFO("USB power removed");
            #endif 
            app_usbd_stop();
            break;
        case APP_USBD_EVT_POWER_READY:
            #ifdef UART_PRINTING_ENABLED
            NRF_LOG_INFO("USB ready");
            #endif
            app_usbd_start();
            break;
        default:
            break;
    }
}

void send_string_as_hid_keys( char *str, uint8_t size)
{
    uint8_t hid_key;

  for(int I=0;str[I]!=EOF;I++){
        char ch = *str;
        bool valid = true;

        switch (ch)
        {
            case 'a': hid_key = 4; break;
            case 'b': hid_key = 5; break;
            case 'c': hid_key = 6; break;
            case 'd': hid_key = 7; break;
            case 'e': hid_key = 8; break;
            case 'f': hid_key = 9; break;
            case 'g': hid_key = 10; break;
            case 'h': hid_key = 11; break;
            case 'i': hid_key = 12; break;
            case 'j': hid_key = 13; break;
            case 'k': hid_key = 14; break;
            case 'l': hid_key = 15; break;
            case 'm': hid_key = 16; break;
            case 'n': hid_key = 17; break;
            case 'o': hid_key = 18; break;
            case 'p': hid_key = 19; break;
            case 'q': hid_key = 20; break;
            case 'r': hid_key = 21; break;
            case 's': hid_key = 22; break;
            case 't': hid_key = 23; break;
            case 'u': hid_key = 24; break;
            case 'v': hid_key = 25; break;
            case 'w': hid_key = 26; break;
            case 'x': hid_key = 27; break;
            case 'y': hid_key = 28; break;
            case 'z': hid_key = 29; break;
            case '1': hid_key = 30; break;
            case '2': hid_key = 31; break;
            case '3': hid_key = 32; break;
            case '4': hid_key = 33; break;
            case '5': hid_key = 34; break;
            case '6': hid_key = 35; break;
            case '7': hid_key = 36; break;
            case '8': hid_key = 37; break;
            case '9': hid_key = 38; break;
            case '0': hid_key = 39; break;
            case '\n':
            case '\r': hid_key = 40; break; // ENTER key
          case '\0' : valid=false ;break;
          case ' ': hid_key=44;break;
            default:
                valid = false;
                break;
        }

        if (valid)
        {
  ret_code_t err;

 UNUSED_RETURN_VALUE(app_usbd_hid_kbd_key_control(&m_app_hid_kbd, hid_key, true));

nrf_delay_ms(6);

UNUSED_RETURN_VALUE( app_usbd_hid_kbd_key_control(&m_app_hid_kbd, hid_key, false));
nrf_delay_ms(6);

 UNUSED_RETURN_VALUE(hid_kbd_clear_buffer(app_usbd_hid_kbd_class_inst_get(&m_app_hid_kbd)));
nrf_delay_ms(6);
UNUSED_RETURN_VALUE( app_usbd_hid_kbd_key_control(&m_app_hid_kbd, 0, false));
nrf_delay_ms(6);
        }

        str++; // Now increment
    }


 
}





static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
  ret_code_t err_code;

  switch(pin_no)
  {
   case BUTTON_1:
     if(button_action == APP_BUTTON_PUSH) {
send_now = true;
      #ifdef UART_PRINTING_ENABLED
        NRF_LOG_INFO("button1 pressed.");
        #endif
     }
  
     break;

   case BUTTON_2:
     if(button_action == APP_BUTTON_PUSH) {
        UNUSED_RETURN_VALUE(app_usbd_hid_kbd_key_control(&m_app_hid_kbd, BUTTON2_CONFIG, true));
        #ifdef UART_PRINTING_ENABLED
        NRF_LOG_INFO("button2 pressed.");
        #endif
     }
     else if(button_action == APP_BUTTON_RELEASE) {
        UNUSED_RETURN_VALUE(app_usbd_hid_kbd_key_control(&m_app_hid_kbd, BUTTON2_CONFIG, false));
        #ifdef UART_PRINTING_ENABLED
        NRF_LOG_INFO("button2 released.");
        #endif
     }
     break;

   case BUTTON_3:
     if(button_action == APP_BUTTON_PUSH) {
        UNUSED_RETURN_VALUE(app_usbd_hid_kbd_key_control(&m_app_hid_kbd, BUTTON3_CONFIG, true));
        #ifdef UART_PRINTING_ENABLED
        NRF_LOG_INFO("button3 pressed.");
        #endif
     }
     else if(button_action == APP_BUTTON_RELEASE) {
        UNUSED_RETURN_VALUE(app_usbd_hid_kbd_key_control(&m_app_hid_kbd, BUTTON3_CONFIG, false));
        #ifdef UART_PRINTING_ENABLED
        NRF_LOG_INFO("button3 released.");
        #endif
     }
     break;

   case BUTTON_4:
     if(button_action == APP_BUTTON_PUSH) {
        UNUSED_RETURN_VALUE(app_usbd_hid_kbd_key_control(&m_app_hid_kbd, BUTTON4_CONFIG, true));
        #ifdef UART_PRINTING_ENABLED
        NRF_LOG_INFO("button4 pressed.");
        #endif
     }
     else if(button_action == APP_BUTTON_RELEASE) {
        UNUSED_RETURN_VALUE(app_usbd_hid_kbd_key_control(&m_app_hid_kbd, BUTTON4_CONFIG, false));
        #ifdef UART_PRINTING_ENABLED
        NRF_LOG_INFO("button4 released.");
        #endif
     }
     break;
     default:
        return; // no implementation needed
  }

}

/**@brief Function for buttons configuration.
 *
 */
static const app_button_cfg_t app_buttons[BUTTONS_NUMBER] = 
{
// BUTTONS_NUMBER,
// BUTTON_PIN,
// BUTTONS_ACTIVE_STATE,
// BUTTON_PULL,
// are all declared in your components/boards/pca10056.h file

    {BUTTON_1, BUTTONS_ACTIVE_STATE, BUTTON_PULL, button_event_handler},
    {BUTTON_2, BUTTONS_ACTIVE_STATE, BUTTON_PULL, button_event_handler},
    {BUTTON_3, BUTTONS_ACTIVE_STATE, BUTTON_PULL, button_event_handler},
    {BUTTON_4, BUTTONS_ACTIVE_STATE, BUTTON_PULL, button_event_handler}
}; 

/**
 *@brief Function for initialising the timers. 
 * It is necessary for the buttons to work when using the app_button library. 
 */
static void timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the buttons with the app_button library.
 *
 */
 void buttons_init(){

    ret_code_t  err_code;                        
// BUTTONS_NUMBER  is declared in your components/boards/Ppca10056.h file
    err_code = app_button_init((app_button_cfg_t *)app_buttons,
                                                BUTTONS_NUMBER,
                                         BUTTON_DETECTION_TIME);
    APP_ERROR_CHECK(err_code);

    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);

}

/**@brief Function for initializing the leds with the bsp library.
 *
 */
static void leds_init()
{
    ret_code_t err_code;

    err_code = bsp_init(BSP_INIT_LEDS , NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**@brief Function for initializing the usb.
 */
void usb_init(void)
{
    ret_code_t ret;
    static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_ev_handler,
    };

    ret = nrf_drv_clock_init();
    APP_ERROR_CHECK(ret);

    nrf_drv_clock_lfclk_request(NULL);
    while(!nrf_drv_clock_lfclk_is_running());

    ret = app_usbd_init(&usbd_config);
    APP_ERROR_CHECK(ret);
}

/**@brief Function for initializing the usb classes.
 */
void usb_classes_init(void)
{
    ret_code_t ret;

    app_usbd_class_inst_t const * class_inst_kbd;

    class_inst_kbd = app_usbd_hid_kbd_class_inst_get(&m_app_hid_kbd);

    ret = app_usbd_class_append(class_inst_kbd);
    APP_ERROR_CHECK(ret);

}

/**@brief Function for the checking the usb power.
 */
void usb_power_check(void)
{
    ret_code_t ret;

    if (USBD_POWER_DETECTION)
    {
        ret = app_usbd_power_events_enable();
        APP_ERROR_CHECK(ret);
    }
    else
    {
        #ifdef UART_PRINTING_ENABLED
        NRF_LOG_INFO("No USB power detection enabled\r\nStarting USB now");
        #endif
        app_usbd_enable();
        app_usbd_start();
    }

}
    







int main(void)
{

   #ifdef UART_PRINTING_ENABLED
    log_init();
   #endif

    timers_init();

    leds_init();
    buttons_init();

    usb_init();
    usb_classes_init();

    usb_power_check();
     
    while (true)
    {
 
  while (app_usbd_event_queue_process())
        {
       }
  if (send_now)
        {
char my_str[] = "joyson\r";
uint8_t size = sizeof(my_str);
send_string_as_hid_keys(my_str,size);
                              send_now = false;


           //   nrf_delay_ms(1000);
        }
                //    send_string_as_hid_keys("joyson vashikaran ");

        UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
        /* Sleep CPU only if there was no interrupt since last loop processing */
        __WFE();

    }
}
