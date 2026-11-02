CXX = g++
CXXFLAGS = -std=c++17 -O2 -Iinclude
LDFLAGS = -lws2_32 -lcrypt32 -lwinhttp

SRCS = src/StorageManager.cpp src/AIModel.cpp src/ConversationManager.cpp src/ChatBot.cpp src/HttpServer.cpp src/main.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = ../server.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

