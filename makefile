CC		= g++
LDFLAGS 	= -lglut -lGL -lGLU -lm -std=c++11 
PROJECT		= assignment6
FILES		= assignment6.cpp

${PROJECT}:${FILES}
	${CC} ${CFLAGS} -o ${PROJECT} ${FILES} ${LDFLAGS}