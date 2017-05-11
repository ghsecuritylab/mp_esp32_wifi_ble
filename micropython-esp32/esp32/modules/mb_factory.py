import time
import makeblockclass

w=makeblock_wlan()
w.wifi_enable()
w.wifi_mode(2)
w.wifi_ap("maker123-eap","12345678")
w.wifi_ap_start()
time.sleep(2)




