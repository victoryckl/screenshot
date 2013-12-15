@echo off

::@goto test
@adb push ../libs/armeabi/tofb /data/local
@adb shell chmod 777 /data/local/*
@adb shell /data/local/tofb /mnt/sdcard/%1

:test
FOR /L %%i IN (1,1,10) DO (
	@adb shell /data/local/tofb /mnt/sdcard/bmp/%%i.bmp
)

pause