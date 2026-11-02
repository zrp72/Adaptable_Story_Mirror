
FROM debian

RUN apt-get update
RUN apt-get -y install aptitude
RUN aptitude install -y make
RUN aptitude install -y g++
RUN aptitude install -y libsdl3-dev
RUN aptitude install -y libsdl3-image-dev libsdl3-ttf-dev


ARG dest=/src/Adaptable_Story_Mirror


RUN mkdir -p ${dest}
RUN mkdir -p ${dest}/Source
RUN mkdir -p ${dest}/Include
COPY * ${dest}
COPY Source/*.c Source/*.cxx ${dest}/Source
COPY Include/* ${dest}/Include
WORKDIR ${dest}

RUN make CXXFLAGS=-std=c++20 CPPFLAGS=-IInclude 2>&1

CMD ["/src/Adaptable_Story_Mirror/ASM"]
