all:
	ndk-build
	adb push libs/armeabi-v7a/mfdisk /data/local/tmp
	adb shell /data/local/tmp/mfdisk

clean:
	ndk-build clean
	rm -rf libs obj
