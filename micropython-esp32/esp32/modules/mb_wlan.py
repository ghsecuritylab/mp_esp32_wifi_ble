from makeblock import wlan

class makeblock_wlan(object):
    # mode:0 for null, 1 for sta, 2 for ap,3 for apsta 
    def __init__(self,mode=0):
        self.w_mode=mode % 3
        self.w_en=False
        self.w=wlan();
        
    def wifi_enable(self):
        self.w.enable()
        self.w_en=True
        
    def wifi_mode(self,mode=None):
        if self.w_en==True:
            if mode==None:
                self.w.get_mode()
            elif mode<=3:
                self.w_mode=mode
                self.w.set_mode(self.w_mode)
            else:
                raise ValueError("para should not more than 3")
        else:
            raise AssertionError("wifi is not enabled")
        
    def wifi_ap(self,ssid=None,password=None):
        if (ssid is None) and (password is None):
            self.w.get_config(0)
        elif password==None:
            print("please set the password\n")
        else:
            self.w.set_ap(ssid,password)
            
    def wifi_ap_start(self):
        if self.w_mode==2 or self.w_mode==3:
            self.w.start(2)
        
    def wifi_sta(self,ssid=None,password=None):
        if (ssid is None) and (password is None):
            self.w.get_config(1)
        elif password==None:
            print("please set the password")
        else:
            self.w.set_sta(ssid,password)

    def wifi_sta_auto_connect(self,en):
        self.w.set_auto_connect(True)

    def wifi_connect(self):
        if self.w_mode==1 or self.w_mode==3:
            self.w.start(1)
        else:
            #print("please set the wifi mode to sta or apsta ")
            raise AssertionError("only sta or apsta mode have this func")
        
    def wifi_scan(self):
        if self.w_mode==1 or self.w_mode==3:
            self.w.scan()
        else:
            #print("please set the wifi mode to sta or apsta ")
            raise AssertionError("only sta or apsta mode have this func")
        
    def wifi_get_mac(self,ifx): 
        print(self.w.get_mac(ifx))
        
