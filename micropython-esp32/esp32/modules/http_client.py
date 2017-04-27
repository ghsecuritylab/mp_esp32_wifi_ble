try:
    import usocket as socket
except:
    import socket


def main(use_stream=False):
    s = socket.socket()

    #get the address by DNS serve
    #ai = socket.getaddrinfo("www.baidu.com", 80)
    ai=socket.getaddrinfo("www.weather.com.cn",80)
    print("Address infos:", ai)
    addr = ai[0][-1]

    print("Connect address:", addr)
    s.connect(addr)

    if use_stream:
        # MicroPython socket objects support stream (aka file) interface
        # directly, but the line below is needed for CPython.
        s = s.makefile("rwb", 0)
        s.write(b"GET / HTTP/1.0\r\n\r\n")
        print(s.read())
    else:
        #s.send(b"GET HTTP://www.baidu.com/ HTTP/1.0\r\n\r\n")
        s.send(b"GET http://www.weather.com.cn/data/sk/101010100.html HTTP/1.0\r\n ")
        s.send(b"Accept-Charset: utf-8\r\n\r\n")
        print(s.recv(4096))

    s.close()


main()
