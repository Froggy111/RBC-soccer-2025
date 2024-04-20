RPi Compute Module 4:
https://forums.raspberrypi.com/viewtopic.php?t=333292 - 12g mass
https://www.raspberrypi.com/products/compute-module-4/?variant=raspberry-pi-cm4001000

Broadcom BCM2711 quad-core Cortex-A72 (ARM v8) 64-bit SoC @ 1.5GHz
H.265 (HEVC) (up to 4Kp60 decode), H.264 (up to 1080p60 decode, 1080p30 encode) 
OpenGL ES 3.1, Vulkan 1.0
Options for 1GB, 2GB, 4GB or 8GB LPDDR4-3200 SDRAM (depending on variant)
Options for 0GB ("Lite"), 8GB, 16GB or 32GB eMMC Flash memory (depending on variant)
Option for fully certified radio module:
2.4 GHz, 5.0 GHz IEEE 802.11 b/g/n/ac wireless;
Bluetooth 5.0, BLE;
On-board electronic switch to select either external or PCB trace antenna

key TPU is not compatiable with RPi though... :..(
have to use USB accelerator



Dual M.2 TPU ($39.99 USD(?) MSRP) (2.5g) (NO COOLING):
https://coral.ai/products/m2-accelerator-dual-edgetpu/

PCIe driver - need to figure out PCIe slots though (yay motherboard design)
https://coral.ai/docs/m2/get-started/#4-run-a-model-on-the-edge-tpu

Barebones accelerator (DO NOT USE)
https://www.mouser.sg/new/google-coral/coral-accelerator-mod-Edge-TPU/

Fully built (90g) (USB-C) (mem speed/latency possibly an issue bcos transformers, only 10GBps which is shit) (with cooling already i think??)
https://coral.ai/products/accelerator/

Devboard (1 or 4GB DDR4 on board) (88.10*59.90mm^2) (cant find weight bcos i suck at googling) (191.170g) (4TOPs int8) (WAY TOO HEAVY)
https://coral.ai/products/dev-board/
https://coral.ai/docs/dev-board/datasheet/#overview
https://coral.ai/static/files/Coral-Dev-Board-datasheet.pdf
https://www.mouser.sg/ProductDetail/Coral/G950-01455-01?qs=u16ybLDytRZzoDDKD3Sj%2FQ%3D%3D

Devboard mini (2GB DDR3 on board) (48.00*64.00mm^2) (25.5g) (4TOPs int8)
https://coral.ai/products/dev-board-mini/
https://coral.ai/static/files/Coral-Dev-Board-Mini-datasheet.pdf