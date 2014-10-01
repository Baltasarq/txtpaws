txtpaws: txtpaws.o msg.o persistente.o stringman.o
	$(CPP) -m32 txtpaws.o msg.o persistente.o stringman.o -otxtpaws

txtpaws.o: txtpaws.cpp txtpaws.h
	$(CPP) -m32 -c txtpaws.cpp -otxtpaws.o

msg.o: msg.cpp msg.h
	$(CPP) -m32 -c msg.cpp -omsg.o

persistente.o: persistente.cpp persistente.h
	$(CPP) -m32 -c persistente.cpp -opersistente.o

stringman.o: stringman.cpp stringman.h
	$(CPP) -m32 -c stringman.cpp -ostringman.o

clean:
	$(RM) *.o
	$(RM) txtpaws
