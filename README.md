Majordomo Protocol Broker, Client and Worker
==============================================================

## Overview

This is an implementation of the
[Majordomo Protocol (MDP/RFC 7)](https://rfc.zeromq.org/spec:7/MDP) —
a reliable, service-oriented request-reply pattern built on
[ZeroMQ](http://zeromq.org).

The system provides a **message broker** that routes client requests to
registered workers by service name. Workers register themselves for named
services, process requests in a dedicated task thread, and return replies.
The broker enforces reliability through mutual heartbeating: dead workers
are detected and evicted within 9 seconds; clients receive an explicit
failure reply rather than hanging indefinitely.

Key characteristics:
- **Service discovery**: workers advertise named services at runtime;
  clients address services by name, not by worker address.
- **Load balancing**: idle workers for a service are selected round-robin.
- **Fault detection**: broker and workers exchange heartbeats every 3 s;
  absence for 9 s triggers disconnection and client notification.
- **Opaque payload**: message bodies are binary-safe — JSON, protobuf,
  or any other format can be used.
- **Horizontal scaling**: multiple workers may register for the same
  service; add workers without restarting broker or clients.

## Main Components

| Component | Role |
|-----------|------|
| **broker** | Central ROUTER; maintains worker registry (`WorkerPool`),
tracks in-flight tasks (`BrokerTasks`), enforces heartbeating. |
| **worker** | DEALER; registers for a named service, processes requests
in a background task thread, returns replies. Reconnects on broker failure. |
| **client** | DEALER; sends a single blocking request and waits for the
reply. Stateless and ephemeral. |
| **common** | Shared protocol constants (`MDP.h`), message factories,
`ZMQIdentity`, `MutualHeartbeatMonitor`, and I/O helpers. |
| **apps/broker** | `broker` daemon — bind address set with `-a`. |
| **apps/client** | `client` CLI — reads JSON from file/stdin, prints or
saves JSON reply. |
| **apps/echo_worker** | Example `worker` that echoes input unchanged. |

## Diagrams

### Component Diagram
![Component diagram](diagrams/component.png)

### Detailed Request / Reply Sequence
![Request sequence](diagrams/sequence_request.png)

### Heartbeat and Failure Sequence
![Heartbeat sequence](diagrams/sequence_heartbeat.png)


Dependencies
------------
1. [zmq library](http://zeromq.org)
1. [zmqpp: zmq library c++ wrapper](https://github.com/zeromq/zmqpp)
1. [json c++ library](https://github.com/nlohmann/json)

```console
# Debian based
sudo apt-get install nlohmann-json3-dev libzmq3-dev
```

Building with make
------------------

```console
git clone --recurse-submodules https://github.com/wdl83/mdp
cd mdp 
make install
```
Build artifacts will be placed in 'install_dir' dir.


Building with CMake
------------------

```console
git clone --recurse-submodules https://github.com/wdl83/mdp
cd mdp 
./buld_rel.sh build_dir install_dir
```
Build artifacts will be placed in 'install_dir' dir.


Docker build environment
------------------------

```console
git clone https://github.com/wdl83/mdp
cd mdp/docker
./make_env.sh # generate .env
docker-compose up mpd --detach
docker exec -it mdp /bin/bash -l
```

Follow build instruction listed above.

Usage
-----
Broker requires the IP address and listen port.
Running manually from console:

```console
broker -a tcp://0.0.0.0:6060
```

or better as systemd service.
Create .config/systemd/user/broker.service

```cosnole
# this config assumes you installed:
# broker in $HOME/bin
# libzmqpp.so* libraries in $HOME/lib
#
[Unit]
Description=MDP Broker

[Service]
Environment=TRACE_LEVEL=2
Environment=LD_LIBRARY_PATH=$HOME/lib/:$LD_LIBRARY_PATH
ExecStart=$HOME/bin/broker -a tcp://0.0.0.0:6060
Restart=on-failure
RestartSec=10s

[Install]
WantedBy=default.target
```

After systemd service file is created enable it:

```console
systemctl --user enable broker.service
```

and start

```console
systemctl --user start broker.service
```

Enable systemd to start $USER services at boot (no $USER login required)

```console
loginctl enable-linger $USER
```

