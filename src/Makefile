txtpaws: txtpaws.o msg.o persistente.o stringman.o
	$(CPP) txtpaws.o msg.o persistente.o stringman.o -otxtpaws

txtpaws.o: txtpaws.cpp txtpaws.h
	$(CPP) -c txtpaws.cpp -otxtpaws.o

msg.o: msg.cpp msg.h
	$(CPP) -c msg.cpp -omsg.o

persistente.o: persistente.cpp persistente.h
	$(CPP) -c persistente.cpp -opersistente.o

stringman.o: stringman.cpp stringman.h
	$(CPP) -c stringman.cpp -ostringman.o

