# Stethoscope

## Server
Server will be a thrift service which on initial load reads from zookeeper and then keeps a cache of service entries,
services will check in periodically which will update the cache, if a endpoint has not checked in 11 minutes then we
will remove that endpoint from zookeeper.

## Client
Client will run along side deployed thrift processes and will check into the stethoscope service the client will take in
a configurable timeout.

## Thrift
Thrift the thrift definitions for this file.
