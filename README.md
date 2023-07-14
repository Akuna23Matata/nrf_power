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
Download this Github repo to your NRF5 SDK folder as (NRF5 SDK folder)/examples/ble_peripheral. By putting the code in correct folder, the Segger Embedded Studio can find all nessesary dependeces automatically. Believe me you don't want to manually setup these.
### Second Step
Nowthat we have everything we need to start programming, we will start to build our first BLE beacon.
1. Init Service
