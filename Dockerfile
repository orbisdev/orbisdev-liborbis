ARG BASE_DOCKER_IMAGE

FROM $BASE_DOCKER_IMAGE

ENV PS4SDK $ORBISDEV

COPY . /src

RUN apk add build-base clang
RUN cd /src && make clean all install

# Second stage of Dockerfile
FROM alpine:latest  

ENV ORBISDEV /usr/local/orbisdev
ENV PS4SDK $ORBISDEV
ENV PATH $ORBISDEV/bin:$PATH

COPY --from=0 ${ORBISDEV} ${ORBISDEV}