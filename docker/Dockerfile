FROM debian:trixie

ARG USR
ARG UID
ARG GID

ENV USR=${USR}
ENV UID=${UID}
ENV GID=${GID}
ENV HOME_DIR=/home/${USR}/

RUN apt-get update
RUN apt-get -y upgrade --no-install-recommends
RUN apt-get -y install --no-install-recommends \
    dumb-init \
    g++ \
    git \
    htop \
    libzmq3-dev \
    make \
    nlohmann-json3-dev \
    tree

RUN apt-get clean && \
  rm /var/lib/apt/lists/* -rf 

RUN groupadd --gid ${GID} ${USR}
RUN useradd \
  --uid ${UID} \
  --gid ${GID} \
  --shell /bin/bash \
  --home-dir /home/${USR} \
  --create-home \
  --no-user-group \
  ${USR}

RUN echo "${USR} ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers 
USER ${USR}
RUN mkdir -p ${HOME_DIR}/mdp
WORKDIR /home/${USR}
ENTRYPOINT ["dumb-init", "--"]
CMD ["/bin/bash"]
