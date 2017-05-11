class makeblock_callback(object):
    def __init__(self,handle=None,data=None):
        self.handle=handle
        self.data=data

    def cb_register(self,func,data=None):
        self.handle=func
        self.data=data

    def call(self):
        self.handle(self.data)

