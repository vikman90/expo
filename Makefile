# September 29, 2018

BIN = bin
SRC = src
INCLUDE = include

TARGET = $(BIN)/expo
LD = $(CXX)
CXXFLAGS = -pipe -std=c++11 -pthread -I$(INCLUDE)
LDFLAGS = -pipe -pthread

ifeq ($(DEBUG),)
CXXFLAGS += -O2 -DNDEBUG
else
CXXFLAGS += -g
endif

SOURCES = $(wildcard $(SRC)/*.cpp)
HEADERS = $(wildcard $(INCLUDE)/*.h)
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJECTS): $(HEADERS)

clean:
	$(RM) $(TARGET) $(OBJECTS)
