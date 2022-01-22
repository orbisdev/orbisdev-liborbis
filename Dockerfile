ARG BASE_DOCKER_IMAGE

FROM $BASE_DOCKER_IMAGE

COPY . /src

RUN apk add bash build-base git ncurses-dev

# oldyear
# RUN cd /src && make clean all install

# newyear: recall modular script
RUN git clone -b newyar https://github.com/orbisdev/orbisdev-liborbis
RUN orbisdev-liborbis/build.sh

# Second stage of Dockerfile
FROM alpine:latest  

ENV ORBISDEV /usr/local/orbisdev
ENV PATH $ORBISDEV/bin:$PATH

COPY --from=0 ${ORBISDEV} ${ORBISDEV}
