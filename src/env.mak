ifeq ($(PUMPKINOS),)
ifneq ($(wildcard ../../../PumpkinOS/src/common.mak),)
PUMPKINOS=../../../PumpkinOS
else
$(error Set variable PUMPKINOS to the root of your PumpkinOS installation)
endif
endif
include $(PUMPKINOS)/src/common.mak
include $(PUMPKINOS)/src/commonp.mak
