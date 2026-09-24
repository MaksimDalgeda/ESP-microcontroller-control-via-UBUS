# ESP Microcontroller Control via UBUS

## Overview

This project implements a daemon that allows controlling ESP microcontrollers connected to a router through the ubus interface.

The daemon communicates with ESP devices over a serial connection and exposes control methods through ubus.

## Supported UBUS Methods

### devices

Returns a list of connected devices with:

- port
- vendor id
- product id

### on

Turns on a specified pin.

Parameters:

- port
- pin

### off

Turns off a specified pin.

Parameters:

- port
- pin

### get

Reads data from a connected sensor.

Parameters:

- port
- pin
- model
- sensor

Currently DHT sensors are supported.

## Features

- Supports multiple connected ESP devices.
- Automatic device discovery.
- JSON responses using the blobmsg library.
- Error reporting through ubus responses.
- Designed to run as a background service.
