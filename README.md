# Majordomo Protocol 

Implementation of Majordomo Protocol Broker, Client and Worker.
See: https://rfc.zeromq.org/spec:7/MDP/ for detail.s


Dependencies
------------
1. [zmq library](http://zeromq.org)
1. [zmqpp: zmq C++ wrapper](https://github.com/zeromq/zmqpp)

Building
--------

```console
git clone --recurse-submodules https://github.com/wdl83/mdp
cd mdp 
RELEASE=1 make
```

Building with Docker
--------------------

```console
git clone https://github.com/wdl83/mdp
cd bootloader
./make_env.sh # generate .env
sudo docker-compose up
grep DST= .env # directory where artifacts are located
````

Usage
-----

```console
broker.elf -a tcp://0.0.0.0:6060
```

or better as systemd service:

```cosnole
# create .config/systemd/user/broker.service
#
# this config assumes you installed:
# broker.elf in $HOME/bin
# libzmqpp.so* libraries in $HOME/lib
#
[Unit]
Description=MDP Broker

[Service]
Environment=TRACE_LEVEL=2
Environment=LD_LIBRARY_PATH=$HOME/lib/:$LD_LIBRARY_PATH
ExecStart=$HOME/bin/broker.elf -a tcp://0.0.0.0:6060
Restart=on-failure
RestartSec=10s

[Install]
WantedBy=default.target
```

After systemd service file is created enable it:

```console
systemctl --user enable broker.service
```

Enable systemd to start $USER services at boot (no $USER login required)

```console
loginctl enable-linger $USER
```

## Simple Task Processing Flow

![diagram](diagrams/simple_request.png)

## Simple Worker Lifetime

![diagram](diagrams/worker_lifetime.png)
