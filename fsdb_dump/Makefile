SEED = 1234

verdi_opts += -simdir simv.daidir
verdi_opts += -ssf test.fsdb

com:
	source ~/EDA/eda.bashrc ;\
	vcs -sverilog -debug_all test.v

sim:
	./simv +ntb_random_seed=$(SEED) \
	-ucli -i dump_fsdb.tcl \
	+fsdb+autoflush \
	-l sim_vcs.log 

verdi:
	verdi $(verdi_opts)

clean:
	rm -rf novas.* verdiLog simv.daidir \
	novas_dump.log com_vcs.log ucli.key \
	sim_vcs.log simv csrc
