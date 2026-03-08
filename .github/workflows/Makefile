cat > Makefile << 'EOF'
# iOS 18 Rootless Makefile
ARCHS = arm64 arm64e
TARGET = iphone:clang:latest:18.0
DEBUG = 0
FINALPACKAGE = 1
THEOS_PACKAGE_SCHEME = rootless

# 检测 Theos 路径
ifeq ($(THEOS),)
THEOS = $(HOME)/theos
endif

include $(THEOS)/makefiles/common.mk

TWEAK_NAME = AppSync18
AppSync18_FILES = Tweak.x
AppSync18_CFLAGS = -fobjc-arc -Wno-deprecated-declarations
AppSync18_LDFLAGS = -lsubstrate
AppSync18_PRIVATE_FRAMEWORKS = MobileCoreServices

include $(THEOS_MAKE_PATH)/tweak.mk

# 安装后提示
after-install::
	@echo "安装完成，请重启 installd: killall -9 installd"
EOF
