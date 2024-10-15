osek_GEN_FILES := $(wildcard gen/inc/task_Cfg.h.php) \
                  $(wildcard gen/src/task_Cfg.c.php) \
                  $(wildcard gen/inc/resource_Cfg.h.php) \
                  $(wildcard gen/src/resource_Cfg.c.php) \
                  $(wildcard gen/inc/event_Cfg.h.php) \
                  $(wildcard gen/src/event_Cfg.c.php) \
                  $(wildcard gen/inc/alarm_Cfg.h.php) \
                  $(wildcard gen/src/alarm_Cfg.c.php) \
                  $(wildcard gen/inc/os_Cfg.h.php) \
                  $(wildcard gen/inc/isr_Cfg.h.php) \
                  $(wildcard gen/src/isr_Cfg.c.php) \
				  $(wildcard gen/src/nmdll_Cfg.c.php) \
				  $(wildcard gen/inc/nmdll_Cfg.h.php) \
				  $(wildcard gen/src/com_Cfg.c.php) \
				  $(wildcard gen/inc/com_Cfg.h.php) \
				  $(wildcard gen/inc/ipdu_Cfg.h.php) \
				  $(wildcard gen/inc/networkmsg_Cfg.h.php) \
				  $(wildcard gen/src/com_alarm_Cfg.c.php) \
				  $(wildcard gen/inc/com_alarm_Cfg.h.php) \
				  $(wildcard gen/src/notification_Cfg.c.php) \
				  $(wildcard gen/inc/notification_Cfg.h.php) \
				  $(wildcard gen/src/nmcallback_Cfg.c.php) \
				  $(wildcard gen/inc/nmcallback_Cfg.h.php) \
				  $(wildcard gen/src/nm_Cfg.c.php) \
				  $(wildcard gen/inc/nm_Cfg.h.php) \
				  $(wildcard gen/src/nm_alarm_Cfg.c.php) \
				  $(wildcard gen/inc/nm_alarm_Cfg.h.php) \
				  $(wildcard gen/inc/bus_Cfg.h.php) \
				  


TESTDIR ?= ../oil_test/oil_test1

all:
	@echo "Clean OSEK /Os_Configure folder..."
	rm -rf Os_Configure/
	@echo "Delete boot.asm"
	rm -f device/boot.asm
	@echo "Clean complete."
	@echo ""
	@echo "Generate OSEK C files form .oil configuration file..."
	@echo ""
	@make generate
	@echo ""
	@echo "Generate OSEK C files complete."

generate:
	php generator/generator.php --cmdline -l -v \
	-DARCH=cortexM4 -DCPUTYPE=lpc43xx -DCPU=lpc4337 \
	-c  application/osek.oil -f gen/osekcom.oil.php -o application
	php generator/generator.php --cmdline -l -v \
	-DARCH=cortexM4 -DCPUTYPE=lpc43xx -DCPU=lpc4337 \
	-c  application/osekcom.oil -f $(osek_GEN_FILES) -o Os_Configure
	php generator/generator.php --cmdline -l -v \
	-DARCH=cortexM4 -DCPUTYPE=lpc43xx -DCPU=lpc4337 \
	-c  application/osek.oil -f gen/boot.asm.php -o device

t:
	@make generate_test_oil

generate_test_oil:
	cd $(TESTDIR) && make

clean:
	rm -rf Os_Configure/ device/boot.asm*

.DEFAULT: all

