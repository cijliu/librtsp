all:
	@cd src && $(MAKE)  all
	@cd example && $(MAKE)  all

clean:
	@cd example && $(MAKE) clean
	@cd src && $(MAKE) clean
