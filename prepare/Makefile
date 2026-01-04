CC = gcc # Compilador
CFLAGS = -Wall -Wextra -g  # Flags de compilação # Avisos e debug

# Nome do executável final
TARGET = prepare

# Objetos necessários
OBJS = prepare.o functions.o

all: $(TARGET) 

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) 
#Ligação do executável final

prepare.o: prepare.c functions.h
	$(CC) $(CFLAGS) -c prepare.c

functions.o: functions.c functions.h
	$(CC) $(CFLAGS) -c functions.c

clean:#Remove arquivos objeto e executável
	rm -f *.o $(TARGET)
	@echo "Limpeza (build) concluída!"

clean-data: #Remove arquivos de dados gerados
	rm -f ../tasks_data/*.bin 
	@echo "Limpeza (dados) concluída!"

clean-all: clean clean-data
	@echo "Limpeza total concluída!"