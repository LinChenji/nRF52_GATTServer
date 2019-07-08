#ifndef USER_SERVICE_H_
#define USER_SERVICE_H_

#include "ble.h"
#include "ble_srv_common.h"
#include "sdk_errors.h"
#include "ble_gatts.h"

#include <nrf_log.h>
#include <nrf_log_ctrl.h>
#include <nrf_log_default_backends.h>

//873226dc-f5d8-4586-94f9-6e794c2f6820(Little-endian)
#define BLE_UUID128_USER_SERVICE_BASE_UUID \
{0x20,0x68,0x2f,0x4c,0x79,0x6e,0xf9,0x94,0x86,0x45,0xd8,0xf5,0xdc,0x26,0x32,0x87}

#define BLE_UUID_USER_SERVICE_UUID 0x26dd   //Octet[12:13]
#define BLE_UUID_TEST_CHAR_UUID    0x26de

//#define CCCD_PRESENT    1
#define BLE_US_DEF(_Obj) static ble_us_t _Obj;\
NRF_SDH_BLE_OBSERVER(_Obj ## _obs,                                                                 \
                     BLE_US_BLE_OBSERVER_PRIO,                                                     \
                     ble_us_on_ble_evt, &_Obj)/**/

//BLE_US_DEF(m_us);//user service

typedef enum
{
    BLE_BAS_EVT_NOTIFICATION_ENABLED, /**< Test char notification enabled event. */
    BLE_BAS_EVT_NOTIFICATION_DISABLED /**< Test char notification disabled event. */
} ble_us_evt_type_t;
typedef struct{
    ble_us_evt_type_t evt_type;
    uint16_t          conn_handle;
}ble_us_evt_t;

typedef struct ble_us_s ble_us_t;
typedef void (*ble_us_evt_handler_t)(ble_us_t *p_us,ble_us_evt_t *p_evt);
typedef void (*ble_us_write_evt_handler_t)(ble_us_t        * p_us,
                                           uint16_t        uuid,
                                           uint16_t        value_handle,
                                           uint8_t const   * p_data,
                                           uint16_t        length);

typedef void (*ble_us_read_evt_handler_t)(ble_us_t         * p_us,
                                          uint16_t         uuid,
                                          uint16_t         value_handle);

typedef struct{
    ble_us_evt_handler_t       evt_handler;
    ble_us_read_evt_handler_t  read_evt_handler;
    ble_us_write_evt_handler_t write_evt_handler;
    ble_srv_cccd_security_mode_t us_test_attr_md;//ble_srv_common.h
    uint16_t test_char_val;
    bool is_us_notify_support;
}ble_us_init_t;

struct ble_us_s{
    ble_us_evt_handler_t       evt_handler;
    ble_us_read_evt_handler_t  read_evt_handler;
    ble_us_write_evt_handler_t write_evt_handler;
    uint16_t service_handle;
    uint16_t conn_handle;
    uint8_t  uuid_type;
    uint16_t test_char_val;
    ble_gatts_char_handles_t test_handle;//ble_gatts.h
};

ret_code_t ble_us_init(ble_us_t *p_us,ble_us_init_t *p_us_init);//sdk_errors.h
void ble_us_on_ble_evt(ble_evt_t *p_ble_evt,void *p_context);// const

void us_gatts_handle_write(ble_us_t      * p_us,
                           uint16_t        uuid,
                           uint16_t        val_handle,
                           uint8_t const * p_data,
                           uint16_t        length);
void us_gatts_handle_read(ble_us_t * p_us, uint16_t uuid, uint16_t val_handle);

#endif