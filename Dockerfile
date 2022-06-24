FROM debian:bullseye

ARG USR
ARG UID
ARG GID

ENV USR=${USR}
ENV UID=${UID}
ENV GID=${GID}

#RUN echo "USER ${USR}, uid: ${UID}, gid: ${GID}"

RUN apt-get update
RUN apt-get install -y dumb-init
RUN apt-get install -y g++
RUN apt-get install -y git
RUN apt-get install -y libzmq3-dev
RUN apt-get install -y make
RUN apt-get clean

RUN rm -rf /var/lib/apt/lists/*

RUN addgroup --gid ${GID} ${USR}

RUN adduser \
        --disabled-password \
        --uid ${UID} \
        --gid ${GID} \
        --gecos '' \
        --shell /bin/bash \
        --home /home/${USR} \
        ${USR}

USER ${USR}
WORKDIR /home/${USR}
RUN mkdir -p dst
ENTRYPOINT ["dumb-init", "--"]
CMD [ \
    "/bin/bash", \
    "-c", \
    "git clone --recurse-submodules https://github.com/wdl83/mdp; cd mdp; RELEASE=1 OBJ_DIR=../dst make"]
