# University of Florida EEL6528
# Tan F. Wong
# Aug 31, 2023

CXX = g++
CPPFLAGS = -O3 -DUSE_VOLK -DQPSK #The D is not for differential just a flag
LDFLAGS = -lfftw3f -lfftw3f_threads -lvolk -luhd -lboost_program_options
OBJDIR := ../build

common_hdrs = lab1.h RX.h
OBJS = $(OBJDIR)/lab1.o $(OBJDIR)/Coherent_demodulator.o $(OBJDIR)/Carrier_sync.o $(OBJDIR)/Demodulator.o $(OBJDIR)/Correlator.o $(OBJDIR)/file_gen.o $(OBJDIR)/TX_modulator.o $(OBJDIR)/TX_producer.o $(OBJDIR)/impulse.o $(OBJDIR)/TX.o $(OBJDIR)/multi_filter.o $(OBJDIR)/RX.o $(OBJDIR)/POWER_THREAD.o $(OBJDIR)/AVG_POWER_THREAD.o $(OBJDIR)/filters.o $(OBJDIR)/Shared_Printer.o

all: lab1

lab1: $(common_hdrs) $(OBJDIR) $(OBJS)
	$(CXX) -o lab1 $(OBJS) $(LDFLAGS)

$(OBJDIR):
	mkdir -p $@

$(OBJDIR)/%.o: %.cpp $(common_hdrs)
	$(CXX) $(CPPFLAGS) -c $< -o $@

clean: 
	rm -rf lab1 $(OBJDIR)
