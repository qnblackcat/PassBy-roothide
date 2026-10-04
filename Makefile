# Target iOS 15+ Devices; use the iOS 16.5 SDK
TARGET = iphone:clang:16.5:15.0
export ARCHS = arm64 arm64e

# THEOS_DEVICE_IP = 192.168.1.15
INSTALL_TARGET_PROCESSES = SpringBoard

FINALPACKAGE = 1
# DEBUG = 1
export ADDITIONAL_CFLAGS = -O3

include $(THEOS)/makefiles/common.mk


TWEAK_NAME = PassBy
PassBy_FILES = Tweak.xm

PassBy_FRAMEWORKS = UIKit
PassBy_PRIVATE_FRAMEWORKS = SpringBoard SpringBoardFoundation BluetoothManager BatteryCenter MobileWiFi

include $(THEOS_MAKE_PATH)/tweak.mk


after-install::
	install.exec "killall -9 SpringBoard"


SUBPROJECTS += passbyprefs
include $(THEOS_MAKE_PATH)/aggregate.mk
