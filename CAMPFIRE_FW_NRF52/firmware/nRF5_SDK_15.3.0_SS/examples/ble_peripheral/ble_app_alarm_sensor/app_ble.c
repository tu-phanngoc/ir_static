#include "app_ble.h"
#include "app_mesh.h"
#include "timeslot_handler.h"
#include "nrf_advertiser.h"
#include "IR_Common.h"
#include "ir_learning_fsm.h"
#include "adc_task.h"
#include "crc32.h"
//#define APP_BLE_DEBUG			NRF_LOG_INFO
#define APP_BLE_DEBUG(...)

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define MESH_PROVISION_UUID             0x1827
#define MESH_PROXY                      0x1828

#define RAYTAC_CORP_ID                  0x068A
#define DEVICE_NAME                     "BENKON"                               /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "Raytac" 
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_SOC_OBSERVER_PRIO           1                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                1000                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */

#define APP_ADV_DURATION                0 //3000                                       /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                   1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                   1                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                   0                                          /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS               0                                          /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_DISPLAY_ONLY                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                         /**< Maximum encryption key size. */

#define PASSKEY_TXT                     "Passkey:"                                  /**< Message to be displayed together with the pass-key. */
#define PASSKEY_TXT_LENGTH              8                                           /**< Length of message to be displayed together with the pass-key. */
#define PASSKEY_LENGTH                  6                                           /**< Length of pass-key received by the stack for display. */
#define STATIC_PASSKEY					"123456" 

static ble_opt_t					    m_static_pin_option;

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */


BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                   /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */

static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
// static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
// {
//     {BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}
// };

static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
{
    // {BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE},
    // {BLE_APPEARANCE_CYCLING_POWER_SENSOR, BLE_UUID_TYPE_BLE},
//    {MESH_PROVISION_UUID, BLE_UUID_TYPE_BLE},
    {MESH_PROXY, BLE_UUID_TYPE_BLE}
};

uint8_t ble_rx_data_array[BLE_NUS_MAX_DATA_LEN];
uint8_t ble_rx_data_len = 0;
static uint8_t ble_data_array[BLE_NUS_MAX_DATA_LEN];
static uint8_t ble_data_len = 0;
uint8_t deviceName[32];
uint8_t BLE_RxBuff[256] = {0};
RINGBUF BLE_RxRingBuff;

extern ble_gap_addr_t _device_addr;
uint8_t BLE_TxBuff[256] = {0};
RINGBUF BLE_TxRingBuff;

uint8_t g_u8BleConnected = 0;

uint8_t g_u8BleDisConnectRequest = 0; // 0,1,2 - NONE, REQUESTING, DONE

volatile bool gbBleCommandRecv = false;


extern uint8_t waitingForNewCustomerKey;

ISMARTPACKAGE giSmartPacket;
bool g_TriggerSendFromBleToMesh = false;

uint8_t ble_transmit_status = 0; /* 0: standby; 1: ok; 2: fail; 3: transmitting */
#ifdef PEER_APP

/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    pm_handler_on_pm_evt(p_evt);
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id)
    {
        case PM_EVT_PEERS_DELETE_SUCCEEDED:
            advertising_start(false);
            break;

        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
            if (     p_evt->params.peer_data_update_succeeded.flash_changed
                 && (p_evt->params.peer_data_update_succeeded.data_id == PM_PEER_DATA_ID_BONDING))
            {
                NRF_LOG_INFO("New Bond, add the peer to the whitelist if possible");
                // Note: You should check on what kind of white list policy your application should use.
            }
            break;
			
        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = true};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        } break;
		
        default:
            break;
    }
}

/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}
#endif


/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
		
//		uint8_t dbgStr[64];
//		uint8_t a = 0xAB;
//		sprintf((char*)dbgStr, "%s%02X",VERSION, a);
	
		uint8_t deviceName[32];
		sd_ble_gap_addr_get(&_device_addr);
		sprintf((char*)deviceName, "%s",DEVICE_NAME);
//	
//		uint8_t debugDevName[64];
//		sprintf((char*)debugDevName, "%s_%02X%02X%02X%02X%02X%02X",DEVICE_NAME, _device_addr.addr[0], _device_addr.addr[1], _device_addr.addr[2], _device_addr.addr[3], _device_addr.addr[4], _device_addr.addr[5]);
//		NRF_LOG_INFO("DEVICE NAME: %s\r\n", debugDevName);
		
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) deviceName,
                                          strlen((const char*)deviceName));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
										  
    // Add static pin:
#ifdef PEER_APP																					
	uint8_t passkey[] = STATIC_PASSKEY;
	m_static_pin_option.gap_opt.passkey.p_passkey = passkey;
	err_code =  sd_ble_opt_set(BLE_GAP_OPT_PASSKEY, &m_static_pin_option);
	APP_ERROR_CHECK(err_code);
#endif
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t * p_evt)
{
    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
			gu32IRCmdTimeout = IR_CMD_TIMEOUT;
			meter_timeout = METER_TIMEOUT;
			APP_BLE_DEBUG("Received data from BLE NUS. Writing data on UART, len = %d \r\n", p_evt->params.rx_data.length);
//			NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
			if(p_evt->params.rx_data.length == 24)
			{
				memcpy(&giSmartPacket.packet,p_evt->params.rx_data.p_data,sizeof(ISMARTPACKAGE) - 1);
				giSmartPacket.packet.crc = crc32_compute(&giSmartPacket.packet.addr[0], 20, NULL);
				giSmartPacket.handle = 0xFF;// "This message from app";
				g_TriggerSendFromBleToMesh = true;
			}
			if(ble_rx_data_len == 0)
			{
				memset(ble_rx_data_array,0,BLE_NUS_MAX_DATA_LEN);
				memcpy(ble_rx_data_array,p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
				ble_rx_data_len = p_evt->params.rx_data.length;
			}
			for(uint8_t i = 0;i < p_evt->params.rx_data.length;i++)
				RINGBUF_Put(&BLE_RxRingBuff,p_evt->params.rx_data.p_data[i]);
    }
}

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t           err_code;
    ble_nus_init_t     nus_init;
	ble_dis_init_t dis_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
	
	// Initialize Device Information Service.
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char *)MANUFACTURER_NAME);

	dis_init.dis_char_rd_sec = SEC_OPEN;

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
				APP_BLE_DEBUG("Fast advertising.\r\n");
//            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
//            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
				APP_BLE_DEBUG("Stop advertising.\r\n");
            //sleep_mode_enter();
            break;
        default:
            break;
    }
}

uint32_t ble_is_connected(void)
{
	if(m_conn_handle == BLE_CONN_HANDLE_INVALID)
		return 0;
	else
		return 1;
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            APP_BLE_DEBUG("Connected");
						g_u8BleConnected = 1;

//            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
//            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            APP_BLE_DEBUG("Disconnected");
						g_u8BleConnected = 0;
            // LED indication will be changed when advertising starts.
				
						g_u8BleDisConnectRequest = 0;
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            APP_BLE_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
//		err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
//		APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t evt_id, void * p_context)
{	
		if(app_get_mode() != MESH_MODE)
		{
			btle_hci_adv_sd_evt_handler(evt_id);
		}
		#ifdef USE_MESH
		else
		{
			rbc_mesh_sys_evt_handler(evt_id);
		}
		#endif
}

/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
	
	// Register a handler for SOC events.
	NRF_SDH_SOC_OBSERVER(m_soc_observer, APP_SOC_OBSERVER_PRIO, sys_evt_dispatch, NULL);
}

/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        APP_BLE_DEBUG("Data len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    APP_BLE_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}


/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;
	uint8_t u8_manufacture_data[3];
    u8_manufacture_data[0] = 0x02;
    u8_manufacture_data[1] = 0x01;
    u8_manufacture_data[2] = 0x06;
	uint8_t mac[6] = {0};
	ble_gap_addr_t	addr;
	sd_ble_gap_addr_get(&addr);
	memcpy(mac,addr.addr,6);
	// add mac to res
	ble_advdata_manuf_data_t  manuf_data_response;
			
    memset(&init, 0, sizeof(init));

//    init.config.ble_adv_extended_enabled = true;

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
		

	manuf_data_response.data.p_data = u8_manufacture_data;
	manuf_data_response.data.size   = 3;
	manuf_data_response.company_identifier = RAYTAC_CORP_ID;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;
	init.srdata.p_tx_power_level        = NULL;
	init.srdata.p_manuf_specific_data   = &manuf_data_response;		

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
		APP_BLE_DEBUG("MAC: %02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


/**@brief Function for starting advertising.
 */
void advertising_start(bool erase_bonds)
{
    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
	
		APP_BLE_DEBUG("Advertising ...");
}

uint32_t ble_check_status(void)
{
	if(ble_data_len)
		return	NRF_ERROR_BUSY;
	return NRF_SUCCESS;
}

uint32_t ble_send_data(uint8_t *data,uint8_t len)
{
	if(ble_data_len || len > BLE_NUS_MAX_DATA_LEN)
	{
		return	NRF_ERROR_BUSY;
	}
	memcpy(ble_data_array,data,len);
	ble_data_len = len;
	return NRF_SUCCESS;
}

uint32_t ble_get_data(uint8_t *data)
{
	uint8_t len = 0;
	if(ble_rx_data_len)
	{
		len = ble_rx_data_len;
		ble_rx_data_len = 0;
		memcpy(data,ble_rx_data_array,len);
	}
	return len;
}

uint32_t ble_task(void)
{
	uint8_t i;
	uint32_t       err_code;
	BLE_SendFromNUSToMesh();
	if(g_u8BleConnected == 0) {
		ble_data_len = 0;
		return 20;
	}
	
	if(ble_data_len)
	{
		APP_BLE_DEBUG("BLE_Senddata");
//		NRF_LOG_HEXDUMP_DEBUG(ble_data_array, ble_data_len);
		uint16_t length = ble_data_len;
		err_code = ble_nus_data_send(&m_nus, ble_data_array, &length,m_conn_handle);
		if ( (err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_BUSY) )
		{
				APP_BLE_DEBUG("err = %u....", err_code);
//				APP_ERROR_CHECK(err_code);
		}
		if(err_code != NRF_ERROR_BUSY)
		{
			ble_data_len = 0;
			set_ble_transmit_status(1);
		}
		if(bSendingToMesh == true) {
			bSendingToMesh = false;
		}
	}
	
	return err_code;
}

void BLE_SendFromNUSToMesh()
{
	if(g_TriggerSendFromBleToMesh == true)
	{
		g_TriggerSendFromBleToMesh = false;
//		NRF_LOG_INFO("Send to Mesh Task !!!!!!!!!!!!!!!!!!");
		App_iSmartPacketAddToList(&giSmartPacket);
		//App_iSmartAppPacketAddToList(&giSmartPacket);
		App_iSmartMeshPacketAddToList((uint8_t *)&giSmartPacket,sizeof(ISMARTPACKAGE));
	}
}

uint8_t BLE_PutChar (uint8_t ch) 
{
	RINGBUF_Put(&BLE_TxRingBuff,ch);
	return ch;
}

void BLE_PutString (char *s) 
{
   while(*s)
	{
		BLE_PutChar(*s++);
	}
}

void ble_app_init(void)
{
	ble_stack_init();
	gap_params_init();
	gatt_init();
	services_init();
	advertising_init();
	conn_params_init();
	#ifdef PEER_APP
	peer_manager_init();
	#endif

//	RINGBUF_Init(&BLE_RxRingBuff,BLE_RxBuff,sizeof(BLE_RxBuff));
//	RINGBUF_Init(&BLE_TxRingBuff,BLE_TxBuff,sizeof(BLE_TxBuff));
}


uint32_t force_device_disconnect(void)
{
	APP_BLE_DEBUG("force_device_disconnect");
 	uint32_t err = 0;
// 	if(g_u8BleDisConnectRequest == 0)
// 		{
// 			g_u8BleDisConnectRequest = 1;
// 		}
//		
// 	if(g_u8BleDisConnectRequest == 1)
// 	{
// 		g_u8BleDisConnectRequest = 2;
// 		NRF_LOG_INFO("force_device_disconnect");
// 		err = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
// 	}
	err = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
 	return err;
}

void set_ble_transmit_status(uint8_t status)
{
	ble_transmit_status = status;
}
uint8_t get_ble_transmit_status(void)
{
	return ble_transmit_status;
}
