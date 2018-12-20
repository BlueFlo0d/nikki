# -*- coding: utf-8 -*-
from ctypes import *
from ctypes.util import *
import time
import os.path
import cgi
nikki_lib = cdll.LoadLibrary(find_library('qtdiary'))
char = c_char
sop_ftype = CFUNCTYPE(None,POINTER(char),c_size_t,POINTER(char),c_size_t,c_uint32)
op_ftype = CFUNCTYPE(None,POINTER(char),c_size_t,c_uint32);

def sop_print(title,tlen,summary,slen,ntime):
    print '''<head>
    <meta charset="utf-8">
    '''+"<title>"
    print cgi.escape(title[:tlen],True)," - QtDiary Nikki ☆ Ver 0.1"
    print "</title>"+'<link rel="stylesheet" href="http://cdn.static.runoob.com/libs/bootstrap/3.3.7/css/bootstrap.min.css">'+'</head><body><div class="container"><h1>'
    print cgi.escape(title[:tlen],True)
    print '</h1><h3 id="ndate">'
    print time.ctime(ntime)
    print "</h3><h2>"
    print cgi.escape(summary[:slen],True)
    print "<h2>"
def op_print(content,clen,ntime):
    if content[0] == 'p':
        print "<p><h3>"
        print time.ctime(ntime)
        print "</h3>"
        print cgi.escape(content[1:clen],True).replace('\n','<br/>')
        print "</p>"
    elif content[0] == 'h':
        print content[1:clen]

print "<html>"
nikki_lib.qtdiary_nikki_do_for_path(os.path.expanduser("~/.nikki"),op_ftype(op_print),sop_ftype(sop_print))
print "</div></body></html>"
