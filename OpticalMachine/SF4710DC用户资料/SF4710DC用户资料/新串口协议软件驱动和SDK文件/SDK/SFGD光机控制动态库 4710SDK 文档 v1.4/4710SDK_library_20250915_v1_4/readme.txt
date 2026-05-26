
SF_dlp：库文件目录

test_dlpc_v01：测试程序的所有代码

test_dlpc_app: 可执行的测试程序

Windows端
	在执行  windeployqt test_dlpc_v01.exe  打包程序时，需要将  SF_dlp\lib_win 目录下的 dlphandle.dll 拷贝过来


Linux端
	将 SF_dlp\lib_linux\lib_linux.tar.xz 解压到当前目录，如果目录有改动，需在项目中按对应的位置添加库文件
	libdlphandled.so.1.0.0 为Debug模式下库文件
	libdlphandle.so.1.0.0  为Release模式下库文件
	
	运行程序时，出现 “error while loading shared libraries: libdlphandle.so.1: cannot open shared object file: No such file or directory” 问题时，可通过  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:[存放libdlphandled.so.1.0.0的目录] ,[]不需要
