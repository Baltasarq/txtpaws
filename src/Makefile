CFLAGS = -O2

.PHONY = clean

txtpaws: txtpaws.o msg.o persistente.o stringman.o
	$(CXX) $(CFLAGS) txtpaws.o msg.o persistente.o stringman.o -otxtpaws

txtpaws.o: txtpaws.cpp txtpaws.h
	$(CXX) $(CFLAGS) -c txtpaws.cpp -otxtpaws.o

msg.o: msg.cpp msg.h
	$(CXX) $(CFLAGS) -c msg.cpp -omsg.o

persistente.o: persistente.cpp persistente.h
	$(CXX) $(CFLAGS) -c persistente.cpp -opersistente.o

stringman.o: stringman.cpp stringman.h
	$(CXX) $(CFLAGS) -c stringman.cpp -ostringman.o

clean:
	$(RM) *.o
	$(RM) txtpaws
