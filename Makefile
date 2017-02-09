CXX := gcc 
LD_SCRIPS := ntss.ld 
CFLAGS := -g3 -Iinclude/ -MMD
LDFLAGS := -T $(LD_SCRIPS)

obj := main.o initcalls.o streaming_source.o v4l_device.o
target := ntss

$(target): $(obj)
	@echo "[LD] $@ from $?"
	@$(CXX) $(CFLAGS) $(LDFLAGS) -o $@.elf $(obj)

clean:
	@echo "[RM]"
	@rm -f *.o *.elf *.d *.bin

%.o: %.c
	@echo "[CC] $<"
	@$(CXX) $(CFLAGS) -c -o $@ $<

%.d : ;

.PRECIOUS: %.d

.PHONY: clean

-include $(obj:.o=.d)
