# NRF52 Series Bluetooth Low Energy(BLE) Guide
This document serve as a beginner's guide to program your Nordic NRF52 microcontroller as a functional BLE beacon
## Require Software and Hardware
1. **NRF52DK**
<br /><img src="https://www.nordicsemi.com/-/media/Images/Products/DevKits/nRF52-Series/nRF52-DK/nRF52-DK-render_prod-page.png?h=551&iar=0&mw=350&w=350&hash=DB87685680509C17DF15906A7DC9A289" alt="drawing" width="100"/>
<br />Notice that there may be cheaper NRF52 chips like NRF52 dongle by MDK with direct USB connector, however these dongles are programmed with UF2 bootloader that is not campatible with official Nordic programming tools. So the official NRF52DK is always recommanded for a beginner.<br />
2. **PC**(Windows OS is recommanded)
<br />Apple has a secruity rule that the internal hardware will disguise the real MAC address and provide you a fake ID, which can be different from computer to computer. This will cause some compatible problem especially in early developing phase.<br />
3. **Smart Phone**(Optional)
<br /> A smart device is super easy as a BLE scanner that will ease your time debugging!<br />
4. **Segger Embedded Studio V5.4.2a**
<br /> Although Nordic always recommand their own embedded studio "NRF Connect" in documents, Segger has way more resources about NRF52 programming than NRF Connect has. Also Segger can save you a lot of time by automatically install all related drivers. <br />Also please notice that newer Segger Embedded Studio may not fully compatible with NRF52 chip. So it is important to use v5.x.x. It has been tested on my side that 5.4.2a is the most compatible version.<br />
5. **NRF5 SDK**
## BLE Basic Knowledge
Bluetooth is a compilcated communication protocol that define a veriaty of services and rules. You can read more about these services and rule in [The Bluetooth Core Specification](https://www.bluetooth.com/specifications/specs/?keyword=core+specification). But Nordic has considerablly provided a pre-compiled protocol stack called S132 for NRF52 chips that support all the common Bluetooth functions that you may use. With this pre-compiled binary, you can easily perform Bluetooth functions by simply performing some function calls.
## BLE Beacon Guide
This Guide will guide you to build a basic BLE beacon using NRF52 Chip that can advertise a data.
### First Step
Download this Github repo to your NRF5 SDK folder as ```(NRF5 SDK folder)/examples/ble_peripheral```. By putting the code in correct folder, the Segger Embedded Studio can find all nessesary dependeces automatically. Believe me you don't want to manually setup these.
### Second Step
Nowthat we have everything we need to start programming, we will start to build our first BLE beacon.
1. Service Init
   <br/> As previously mentioned, S132 is a pre-compiled binary with a lot of helpful functions. This functions will enable these functions(Also known as SoftDevice) and init BLE event.
```
void ble_stack_init(void)
```
2. Advertisement Init
   <br/> Now we have already initiated the Bluetooth Functionality in our NRF52 chip, we will now start to construct a message block and configure the Bluetooth function to BLE advertisment.
```
static void advertising_init(char* data)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;

    ble_advdata_manuf_data_t manuf_specific_data;

    manuf_specific_data.data.p_data = data;
    manuf_specific_data.data.size   = 3;

    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type             = BLE_ADVDATA_NO_NAME;
    advdata.flags                 = flags;
    advdata.p_manuf_specific_data = &manuf_specific_data;

    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
    m_adv_params.p_peer_addr     = NULL;
    m_adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval        = NON_CONNECTABLE_ADV_INTERVAL;
    m_adv_params.duration        = 0;

    err_code = ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &m_adv_params);
    APP_ERROR_CHECK(err_code);
}
```
Although it may seem a long function, the only thing we need to pay attention is
```
ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len);
sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &m_adv_params);
```
Where ```ble_advdata_encode``` is encode the content from advdata to m_adv_data, and ```sd_ble_gap_adv_set_configure``` set the transmitting data to stack.<br/>
Also it is worth noticing that all user defined data is stored in a variable with type ```ble_advdata_manuf_data_t.data```. For our implementation, we did not set the UUID for identifying the device to greatly reduce the message length, which will reduce the energy cost of the system. If you want to use the UUID, you can always set it in this function. But in our implementation, we will count on the MAC address of the BLE device.
3. Advertisment start
'''
static void advertising_start(void)
{
    ret_code_t err_code;

    err_code = sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    APP_ERROR_CHECK(err_code);
}
'''
Now that we have already load the message in the BLE stack, and successfully configured the BLE service type, we can easily start the service by calling ```sd_ble_gap_adv_start``` to **START** the service. You can always use ```sd_ble_gap_adv_stop``` to **STOP** the service.
## Notes:
After following the previous steps, you should successfully build your own BLE beacon with costumized messages. Now you can use your smart phone to recieve and read the message. There are a lot of helpful BLE scan tools on either IOS or Andriod platform. Personally I prefer to use ```NRF Connect for Mobile```. This is a powerful BLE scanner with a lot of reference materials. If you want to build your own BLE scanner with some logics, I would recommand use ```Python``` with ```Bleak``` Library. <br/>Please note that Apple will always provide you a fake UUID and MAC address. So please use Windows or Linux OS PC. As I mentioned Apple is doing the conversion internally in hardware, so installing Windows/Linux OS on a Macbook does not solve the problem.
<br/>Don't forget the endless loop in the main function. The MCU will go to sleep when it reach the end of main function, and the BLE event will be shut down.
<br/>The BLE event is performed in a different core, so you can always put the device to ```Sleep Mode``` after configuration, this will save a lot of energy.
## Reference:
NRF52 softdevice APIs:<br/>
https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.s132.api.v6.0.0/index.html<br/>
Bluetooth Core Specifications:<br/>
https://www.bluetooth.com/specifications/specs/?keyword=core+specification<br/>
Segger Embedded Studio(Please Note version number when you download):<br/>
https://www.segger.com/products/development-tools/embedded-studio/<br/>
NRF5 SDK(Please Note version number when you download):<br/>
https://www.nordicsemi.com/Products/Development-software/nrf5-sdk
