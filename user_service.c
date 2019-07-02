#include "user_service.h"
#include "sdk_macros.h"
#include "app_error.h"

#include <string.h>

#define BLE_US_NUMBER_OF_CHARACTERISTICS    1

//static ble_us_t                 m_ble_us; 

/** @brief Check if the error code is equal to NRF_SUCCESS. If it is not, return the error code.
 */
#define RETURN_IF_ERROR(PARAM)                                                                     \
    if ((PARAM) != NRF_SUCCESS)                                                                    \
    {                                                                                              \
        return (PARAM);                                                                            \
    }

typedef struct
{
    uint16_t val_handle;
    uint16_t uuid;
} val_handle_to_uuid_t;

static val_handle_to_uuid_t m_handle_to_uuid_map[BLE_US_NUMBER_OF_CHARACTERISTICS]; //!< Map from handle to UUID.
static uint8_t              m_handle_to_uuid_map_idx = 0;
 
uint16_t testCharVal=0;

static val_handle_to_uuid_t m_handle_to_uuid_map[BLE_US_NUMBER_OF_CHARACTERISTICS];//1


static uint32_t test_char_add(ble_us_t * p_us,
                              ble_us_init_t *p_us_init,
                              ble_gatts_char_handles_t * p_handles)
{
    ble_gatts_char_md_t char_md={0};//ble_gatts.h:MetaData
    ble_gatts_attr_md_t attr_md={0};
    ble_gatts_attr_md_t cccd_md={0};
    ble_gatts_attr_t    attr_char_value={0};
    
    ble_uuid_t          ble_uuid;
    ret_code_t err_code;

    char_md.char_props.read          = 1;
    char_md.char_props.write         = 1;//Property:[read,write]
    char_md.char_props.notify        = 1;
    char_md.p_char_user_desc         = NULL;//CCCD_PRESENT;//(uint8_t *)&
    //char_md.char_user_desc_size      = 1;//sizeof(CCCD_PRESENT);
    //char_md.char_user_desc_max_size  = 1;//sizeof(CCCD_PRESENT);
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = &cccd_md;
    char_md.p_sccd_md                = NULL;/**/

    //BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    //BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);

    // Attribute Metadata settings
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;//1
    attr_md.read_perm  = p_us_init->us_test_attr_md.read_perm;
    attr_md.write_perm = p_us_init->us_test_attr_md.write_perm;
    attr_md.rd_auth    = 1;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;/**/

    ble_uuid.type = p_us->uuid_type;
    ble_uuid.uuid = BLE_UUID_TEST_CHAR_UUID;

    // Attribute Value settings
    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(uint16_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(uint16_t);
    attr_char_value.p_value      = (uint8_t *)&p_us_init->test_char_val;/**/

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);    
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    err_code=sd_ble_gatts_characteristic_add(p_us->service_handle, &char_md,
                                           &attr_char_value,
                                           &p_us->test_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    else
    {
        ASSERT(m_handle_to_uuid_map_idx < BLE_US_NUMBER_OF_CHARACTERISTICS);
        m_handle_to_uuid_map[m_handle_to_uuid_map_idx].val_handle = p_handles->value_handle;
        //m_handle_to_uuid_map[m_handle_to_uuid_map_idx].uuid = p_char_init->uuid;
        m_handle_to_uuid_map_idx++;
        return NRF_SUCCESS;
    }
}

ret_code_t us_gatts_send_reply(ble_us_t *                        p_us,
                               ble_gatts_rw_authorize_reply_params_t * p_reply)
{
    VERIFY_PARAM_NOT_NULL(p_us);
    VERIFY_PARAM_NOT_NULL(p_reply);

    if (p_us->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        return sd_ble_gatts_rw_authorize_reply(p_us->conn_handle, p_reply);
    }

    return NRF_ERROR_INVALID_STATE;
}
ret_code_t send_write_reply(ble_us_t * p_us, ble_gatts_rw_authorize_reply_params_t * p_reply)
{
    VERIFY_PARAM_NOT_NULL(p_us);
    VERIFY_PARAM_NOT_NULL(p_reply);

    p_reply->type                = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
    p_reply->params.write.update = 1;
    p_reply->params.write.offset = 0;

    return us_gatts_send_reply(p_us, p_reply);
}


ret_code_t send_read_reply(ble_us_t * p_us, ble_gatts_rw_authorize_reply_params_t * p_reply)
{
    VERIFY_PARAM_NOT_NULL(p_us);
    VERIFY_PARAM_NOT_NULL(p_reply);

    p_reply->type               = BLE_GATTS_AUTHORIZE_TYPE_READ;
    p_reply->params.read.update = 1;
    p_reply->params.read.offset = 0;

    return us_gatts_send_reply(p_us, p_reply);
}
ret_code_t read_value(ble_us_t * p_us, uint8_t length, const void * p_value)
{
    VERIFY_PARAM_NOT_NULL(p_us);
    VERIFY_PARAM_NOT_NULL(p_value);

    ble_gatts_rw_authorize_reply_params_t reply = {0};
    reply.type                                  = BLE_GATTS_AUTHORIZE_TYPE_READ;//ble_types.h
    reply.params.read.len                       = length;
    reply.params.read.p_data                    = p_value;
    reply.params.read.gatt_status               = BLE_GATT_STATUS_SUCCESS;

    //return send_read_reply(p_us, &reply);//sd_ble_gatts_rw_authorize_reply
    return sd_ble_gatts_rw_authorize_reply(p_us->conn_handle, &reply);
}
ret_code_t ble_us_init(ble_us_t *p_us,ble_us_init_t *p_us_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    //p_us->evt_handler                 = p_us_init->evt_handler;
    p_us->conn_handle       = BLE_CONN_HANDLE_INVALID;
    p_us->write_evt_handler = p_us_init->write_evt_handler;
    p_us->read_evt_handler  = p_us_init->read_evt_handler;
    p_us->test_char_val     = p_us_init->test_char_val;

    //BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_USER_SERVICE);//SIG
    ble_uuid128_t base_custom_uuid = {BLE_UUID128_USER_SERVICE_BASE_UUID};
    err_code = sd_ble_uuid_vs_add(&base_custom_uuid, &p_us->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    ble_uuid.type = p_us->uuid_type;
    ble_uuid.uuid = BLE_UUID_USER_SERVICE_UUID;
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_us->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = test_char_add(p_us,p_us_init,&p_us->test_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }/**/

    return NRF_SUCCESS;
}


void on_connect(ble_us_t * p_us, ble_evt_t const * p_ble_evt)
{
    p_us->conn_handle=p_ble_evt->evt.gap_evt.conn_handle;
}

void on_disconnect(ble_us_t * p_us, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_us->conn_handle = BLE_CONN_HANDLE_INVALID;
}

void us_gatts_handle_write(ble_us_t      * p_us,
                           uint16_t        uuid,
                           uint16_t        val_handle,
                           uint8_t const * p_data,
                           uint16_t        length)
{
    //VERIFY_PARAM_NOT_NULL(p_us);//sdk_macros.h
    //VERIFY_PARAM_NOT_NULL(p_data);
    if( (p_us==NULL)||(p_data==NULL) )
        return;

    ret_code_t                            err_code;
    ble_gatts_rw_authorize_reply_params_t reply      = {0};//be_gatts.h
    bool                                  long_write = false;

    reply.params.write.gatt_status = BLE_GATT_STATUS_SUCCESS;
        
    switch (uuid)
    {
        case BLE_UUID_TEST_CHAR_UUID://0x26de
        {
            if (length > 2)
            {
                reply.params.write.gatt_status = BLE_GATT_STATUS_ATTERR_INVALID_ATT_VAL_LENGTH;
                length = 0;
            }
            else
            {
                //testCharVal=*p_data;
                memcpy(&testCharVal,p_data,length);//Little-endian
                p_us->test_char_val=testCharVal;
            }
         }            

        break;

        defaullt:
        break;
    }
    reply.params.write.len    = length;
    reply.params.write.p_data = p_data;

    err_code=send_write_reply(p_us, &reply);

}

static uint32_t get_evt_type_for_handle(uint16_t handle, uint16_t * p_uuid)
{
    VERIFY_PARAM_NOT_NULL(p_uuid);//sdk_macros.h

    for (uint8_t i = 0; i < BLE_US_NUMBER_OF_CHARACTERISTICS; ++i)
    {
        if (m_handle_to_uuid_map[i].val_handle == handle)
        {
            *p_uuid = m_handle_to_uuid_map[i].uuid;
            return NRF_SUCCESS;
        }
    }

    return NRF_ERROR_NOT_FOUND;
}/**/
void on_long_write(ble_us_t * p_us, ble_evt_t const * p_ble_evt)
{
    static uint16_t write_evt_uuid;
    static bool write_evt_uuid_set = false;
    uint32_t err_code;

    VERIFY_PARAM_NOT_NULL_VOID(p_us);
    VERIFY_PARAM_NOT_NULL_VOID(p_ble_evt);

    ble_gatts_evt_write_t const * p_evt_write =
        &p_ble_evt->evt.gatts_evt.params.authorize_request.request.write;

    ble_gatts_rw_authorize_reply_params_t reply = {0};

    if (p_evt_write->op == BLE_GATTS_OP_PREP_WRITE_REQ)
    {
        err_code = get_evt_type_for_handle(p_evt_write->handle, &write_evt_uuid);
        APP_ERROR_CHECK(err_code);//app_error.h

        write_evt_uuid_set = true;

        reply.type                     = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
        reply.params.write.gatt_status = BLE_GATT_STATUS_SUCCESS;
        reply.params.write.update      = 0;
        reply.params.write.offset      = 0;
        reply.params.write.len         = p_evt_write->len;
        reply.params.write.p_data      = NULL;

        err_code = sd_ble_gatts_rw_authorize_reply(p_us->conn_handle, &reply);
        APP_ERROR_CHECK(err_code);
    }
    /*
    else if (p_evt_write->op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW)
    {
        uint8_t           value_buffer[ESCS_ADV_SLOT_CHAR_LENGTH_MAX] = {0};
        ble_gatts_value_t value =
        {
            .len = sizeof(value_buffer),
            .offset = 0,
            .p_value = &(value_buffer[0])
        };

        ASSERT(write_evt_uuid_set);
        write_evt_uuid_set = false;

        reply.type                     = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
        reply.params.write.gatt_status = BLE_GATT_STATUS_SUCCESS;
        reply.params.write.update      = 0;
        reply.params.write.offset      = 0;
        reply.params.write.len         = p_evt_write->len;
        reply.params.write.p_data      = NULL;

        err_code = sd_ble_gatts_rw_authorize_reply(p_us->conn_handle, &reply);
        APP_ERROR_CHECK(err_code);

        // Now that the value has been accepted using 'sd_ble_gatts_rw_authorize_reply', it can be found in the database.
        err_code = sd_ble_gatts_value_get(  p_us->conn_handle,
                                            p_us->rw_adv_slot_handles.value_handle,
                                            &value);
        APP_ERROR_CHECK(err_code);

        p_us->write_evt_handler(p_us,
                                  write_evt_uuid,
                                  p_evt_write->handle,
                                  value.p_value,
                                  value.len);
    }*/
    else
    {
    }
}
ret_code_t send_notification(ble_us_t * p_us)
{
    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value={0};
      
    gatts_value.len     = sizeof(uint16_t);
    gatts_value.offset  = 0;   
    err_code=sd_ble_gatts_value_get(p_us->conn_handle,
                                    p_us->test_handle.value_handle,&gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }    

    if ((p_us->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params={0};
        //memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_us->test_handle.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_us->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

ret_code_t on_write(ble_us_t * p_us, ble_evt_t  * p_ble_evt)
{
    uint32_t err_code = NRF_SUCCESS;
    uint16_t write_evt_uuid = 0;

    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;//ble_gatts.h
    //ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.authorize_request.request.write;
    if(p_evt_write->handle==p_us->test_handle.value_handle)
    {
        p_us->write_evt_handler(p_us,
                                p_evt_write->uuid.uuid,//write_evt_uuid,
                                p_evt_write->handle,
                                p_evt_write->data,
                                p_evt_write->len);
    }
    if( (p_evt_write->handle==p_us->test_handle.cccd_handle)&&
        (p_evt_write->len==2) )
    {
        if(ble_srv_is_notification_enabled(p_evt_write->data))
        {
            NRF_LOG_INFO("Enable test char notification. \r\n"); 
            send_notification(p_us);
        }
        else
        {
            NRF_LOG_INFO("Disable test char notification. \r\n");
        }
    }
    return NRF_SUCCESS;
}

ret_code_t on_read(ble_us_t * p_us, ble_evt_t * p_ble_evt)
{
    VERIFY_PARAM_NOT_NULL(p_us);
    VERIFY_PARAM_NOT_NULL(p_ble_evt);
    ret_code_t err_code;
    //uint16_t read_evt_uuid = 0;
    ble_gatts_evt_read_t * p_evt_read = 
        &p_ble_evt->evt.gatts_evt.params.authorize_request.request.read;

    //err_code = get_evt_type_for_handle(val_handle, &read_evt_uuid);
    //RETURN_IF_ERROR(err_code);

    p_us->read_evt_handler(p_us, p_evt_read->uuid.uuid, p_evt_read->handle);

    return NRF_SUCCESS;
}
void us_gatts_handle_read(ble_us_t * p_us, uint16_t uuid, uint16_t val_handle)
{
    //VERIFY_PARAM_NOT_NULL(p_us);
    if(p_us==NULL)
        return;
    switch (uuid)
    {
        case BLE_UUID_TEST_CHAR_UUID:
            testCharVal=p_us->test_char_val;
            read_value(p_us,sizeof(testCharVal),&testCharVal);//send_read_reply
        break;

        default:
        break;
    }
}
ret_code_t on_rw_authorize_req(ble_us_t * p_us, ble_evt_t * p_ble_evt)
{
    ret_code_t err_code;
    VERIFY_PARAM_NOT_NULL(p_us);
    VERIFY_PARAM_NOT_NULL(p_ble_evt);

    ble_gatts_evt_rw_authorize_request_t const * ar =
        &p_ble_evt->evt.gatts_evt.params.authorize_request;

    if (ar->type == BLE_GATTS_AUTHORIZE_TYPE_READ)//ble_gatts.h:0x01
    {
        err_code = on_read(p_us, p_ble_evt);
        RETURN_IF_ERROR(err_code);
    }
    else if (ar->type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
    {
        /*if (ar->request.write.op == BLE_GATTS_OP_WRITE_REQ
         || ar->request.write.op == BLE_GATTS_OP_WRITE_CMD)
        {
            err_code = on_write(p_us, p_ble_evt);
            RETURN_IF_ERROR(err_code);
        }

        else*/ if (ar->request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ
             || ar->request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW)
        {
            on_long_write(p_us, p_ble_evt);
        }
        else if (ar->request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL)
        {
            ble_gatts_rw_authorize_reply_params_t auth_reply;
            memset(&auth_reply, 0, sizeof(auth_reply));

            auth_reply.type                     = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
            auth_reply.params.write.gatt_status = BLE_GATT_STATUS_SUCCESS;

            err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle, &auth_reply);
            VERIFY_SUCCESS(err_code);
        }
        else
        {
        }
    }
    else
    {
        return NRF_ERROR_INVALID_STATE;
    }

    return NRF_SUCCESS;
}

void ble_us_on_ble_evt(ble_evt_t *p_ble_evt,void *p_context)
{
    ble_us_t *p_us = (ble_us_t *)p_context;
    if( (p_ble_evt== NULL)||(p_context==NULL) )
        return ;
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED://ble_gap.h:0x10
            on_connect(p_us, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED://ble_gap.h:0x11
            on_disconnect(p_us, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE://ble_gatts.h:0x50
            on_write(p_us, p_ble_evt);
            break;/**/

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST://ble_gatts.h:0x51
            on_rw_authorize_req(p_us, p_ble_evt);
            break;

        /*case BLE_GATTS_EVT_HVC:
            on_hvc(p_us, p_ble_evt);
            break;*/

        default:
            // No implementation needed.
            break;
    }

    //ble_uscs_on_ble_evt(&m_ble_us,p_ble_evt);
}

//NRF_SDH_BLE_OBSERVER(m_us_observer,BLE_US_BLE_OBSERVER_PRIO,ble_us_on_ble_evt, &m_us);
