2018.6.27

프로젝트의 구성

1. common 프로젝트
이 프로제트는 hidapi, lpu237-hidapi 에서 생성된 라이브러리와 헤더 파일을 서로 공유하기 위해 사용되며, 실제 빌드되지는 않는다.
프로젝트를 할 때 eclipse Version: Oxygen.3a Release (4.7.3a) 를 사용했는데 각 프로젝트를 닫으면
 그 프로젝트에서 만든 헤더나 라이브러리를 패스를 설정해 주어도 사용 할 수 없어서, 각 프로제트의 헤더 파일 중 외부에서 사용될 것들은 
이 프로젝트에 위치하게 했으며, 각 프로젝트에서 생성된 라이브러리는 생성되면 이 프로젝트로 복사되도록 했다. 
그리고 이 프로젝트는 항상 열어 놓아서 효율성을 위해 다른 프로젝트를 닫아도 생성된 라이브러리 사용에 문제 없도록 했음.
 
2. hidapi 프로젝트
hidapi 는 hidraw 를 사용한 hid 라이브러리로 libhidapi.so.0.8.0 를 생성한다.
이 것의  soname 은 libhidapi.so.0 이다. 따라서 이 so 를 정적바인딩해서 사용 하기 위해서는 
libhidapi.so.0.8.0 에 대한 심볼릭 링크 파일 libhidapi.so.0 을 생성해서 사용해야 하며,
libhidapi.so.0.8.0 과 libhidapi.so.0 두개 의 파일이 있어야 한다.

libhidapi.so.0 생성 방법은 
ln -s libhidapi.so.0.8.0 libhidapi.so.0
 라고 콘솔에서 입력하면 된다. libhidapi.so.0은 libhidapi.so.0.8.0 에 대한 소프트 심볼링크이므로
libhidapi.so.0.8.0 의 파일 위치가 변경 되면 기존 libhidapi.so.0을 삭제하고 다시 libhidapi.so.0을 만들어야 한다.

3. lpu237-hidapi 프로젝트
lpu237-hidapi 는 libhidapi.so.0.8.0 를 정적 바인딩하여 사용하며, LPU237 카드 리더기의 API 이다.
liblpu237-hidapi.so.1.0.0 를 생성하고 soname은 liblpu237-hidapi.so.1 이다.
정적바인딩을 위해 common 프로젝트에서 lpu237-hidapi.h 라는 헤더파일을 제공하며, 
동적바인딩을 위해 common 프로젝트에서 el_lpu237-hidapi.h 라는 헤더 파일을 제공한다.

4. tp_hidapi 프로젝트
이 프로젝트는 liblpu237-hidapi.so.1.0.0 를 동적 바인딩하여 liblpu237-hidapi.so.1.0.0 기능을 테스트하는 콘솔 프로그램이다.
동적 링크 할 때 liblpu237-hidapi.so 라는 이름으로 하므로, liblpu237-hidapi.so.1.0.0 로 부터 liblpu237-hidapi.so 라는 심볼릭 링크 파일을 생성해야한다.
생성 방법은
ln -s liblpu237-hidapi.so.1.0.0 liblpu237-hidapi.so
이다.
liblpu237-hidapi.so.1.0.0 위치가 변경 되면 liblpu237-hidapi.so 은 다시 생성해야 한다.
tp_hidapi 라는 프로그램을 실행하기 위해서는 
liblpu237-hidapi.so.1.0.0
liblpu237-hidapi.so.1
libhidapi.so.0.8.0
libhidapi.so.0
tp_hidapi
이상 5개의 파일이 필요한다.


* 개발환경
Ubuntu 16.04.4 LTS
Linux version 4.13.0-45-generic (buildd@lgw01-amd64-011)
gcc (Ubuntu 5.4.0-6ubuntu1~16.04.9) 5.4.0 20160609
g++ (Ubuntu 5.4.0-6ubuntu1~16.04.9) 5.4.0 20160609
eclipse Version: Oxygen.3a Release (4.7.3a)
https://github.com/ssyang/library.git


* 종속성
libhidapi.so.0.8.0 파일
	linux-vdso.so.1 =>  (0x00007ffff1555000)
	libudev.so.1 => /lib/x86_64-linux-gnu/libudev.so.1 (0x00007f65cfa72000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f65cf2c2000)
	librt.so.1 => /lib/x86_64-linux-gnu/librt.so.1 (0x00007f65cf0ba000)
	libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f65cee9d000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f65cf890000)

liblpu237-hidapi.so.1.0.0
	linux-vdso.so.1 =>  (0x00007ffe67b40000)
	libhidapi.so.0 => not found
	libstdc++.so.6 => /usr/lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007ff30fa92000)
	libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007ff30f87c000)
	libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007ff30f65f000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007ff30f295000)
	libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007ff30ef8c000)
	/lib64/ld-linux-x86-64.so.2 (0x00007ff310031000)

tp_hidapi
	linux-vdso.so.1 =>  (0x00007ffdb37df000)
	libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007fa0cac56000)
	libstdc++.so.6 => /usr/lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007fa0ca8d4000)
	libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007fa0ca6be000)
	libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007fa0ca4a1000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fa0ca0d7000)
	/lib64/ld-linux-x86-64.so.2 (0x00007fa0cae5a000)
	libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007fa0c9dce000)

* 기타
일반 유저권한으로 실행시 권한 문제로 테스트 프로그램이 오류 날때 경우
/etc/udev/rules.d/elpusk-usb.rules 파일을 만들고
KERNEL=="hidraw*", SUBSYSTEM=="hidraw", MODE="0666", GROUP="plugdev" 를 추가 할 것.


 


