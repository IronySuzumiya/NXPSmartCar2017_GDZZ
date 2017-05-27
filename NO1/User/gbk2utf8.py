import codecs
def ReadFile(filePath,encoding="gbk"):
    with codecs.open(filePath,"r",encoding) as f:
        return f.read()
 
def WriteFile(filePath,u,encoding="utf-8"):
    with codecs.open(filePath,"w",encoding) as f:
        f.write(u)
 
def GBK2UTF8(src,dst):
    content = ReadFile(src,encoding="gbk")
    WriteFile(dst,content,encoding="utf-8")

import os
import os.path

def ReadDirectoryFile(rootdir):
    for parent,dirnames,filenames in os.walk(rootdir):
            #case 1:
            for dirname in dirnames:
                    print("parent folder is:" + parent)
                    print("dirname is:" + dirname)
            #case 2
            for filename in filenames:    
                    print("parent folder is:" + parent)
                    print("filename with full path:"+ os.path.join(parent,filename))
                    if filename.endswith(".c") or filename.endswith(".h"):
                            GBK2UTF8(os.path.join(parent,filename),os.path.join(parent,filename))
if __name__=="__main__":
    ReadDirectoryFile(".")