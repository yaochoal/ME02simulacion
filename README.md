
# ME02 : Simulador NS3.

## Propósito

Generar una Red Ad hoc de mínimo 20 nodos, con diferentes tipos de tráfico y servicios, se debe utilizar la herramienta OpenAI Gym 
definiendo dos métricas de aprendizaje para la simulación creada.

## Integrantes

|       Integrante      |                 Correo                       |
|-----------------------|-----------------------------------------------|
| Dave Sebastian Valencia Salazar      |    <dsvalencias@unal.edu.co>    |
| Yesid Alberto Ochoa Luque      |    <yaochoal@unal.edu.co>     |

## Entregables

### 1. Código fuente del simulador construido:


``` fichero: ./red-ad-hoc.py```
```
import ns.applications
import ns.core
import ns.csma
import ns.internet
import ns.mobility
import ns.network
import ns.olsr
import ns.wifi
import ns.visualizer


def main(argv): 
    # 
    #  Primero, inicializamos algunas variables locales que controlan algunos
    #  parametros de la simulacion.
    #

    cmd = ns.core.CommandLine()
    cmd.backboneNodes = 25
    cmd.stopTime = 20

    ns.core.Config.SetDefault("ns3::OnOffApplication::PacketSize", ns.core.StringValue("1472"))
    ns.core.Config.SetDefault("ns3::OnOffApplication::DataRate", ns.core.StringValue("100kb/s"))

    cmd.AddValue("backboneNodes", "number of backbone nodes")
    cmd.AddValue("stopTime", "simulation stop time(seconds)")

    cmd.Parse(argv)

    backboneNodes = int(cmd.backboneNodes)
    stopTime = int(cmd.stopTime)

    if (stopTime < 10):
        print ("Use a simulation stop time >= 10 seconds")
        exit(1)
    # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # / 
    #                                                                        # 
    #  EL backbone                                                           # 
    #                                                                        # 
    # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # / 

    # 
    #  Creamos un contenedor para administrar los nodos de la red adhoc (backbone).
    # 
    backbone = ns.network.NodeContainer()
    backbone.Create(backboneNodes)
    # 
    #  Creamos los dispositivos de red wifi y los instalamos
    #  en nuestro contenedor
    # 
    wifi = ns.wifi.WifiHelper()
    mac = ns.wifi.WifiMacHelper()
    mac.SetType("ns3::AdhocWifiMac")
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                  "DataMode", ns.core.StringValue("OfdmRate54Mbps"))
    wifiPhy = ns.wifi.YansWifiPhyHelper.Default()
    wifiChannel = ns.wifi.YansWifiChannelHelper.Default()
    wifiPhy.SetChannel(wifiChannel.Create())
    backboneDevices = wifi.Install(wifiPhy, mac, backbone)
    # 
    #  Agregamos la pila de protocolos IPv4 a los nodos en nuestro contenedor
    # 
    internet = ns.internet.InternetStackHelper()
    olsr = ns.olsr.OlsrHelper()
    internet.SetRoutingHelper(olsr);
    internet.Install(backbone);
    # internet.Reset()
    # 
    #  Asignamos direcciones IPv4 a los controladores de dispositivo (en realidad a las interfaces 
    #  IPv4 asociadas) que acabamos de crear.
    # 
    ipAddrs = ns.internet.Ipv4AddressHelper()
    ipAddrs.SetBase(ns.network.Ipv4Address("192.168.0.0"), ns.network.Ipv4Mask("255.255.255.0"))
    ipAddrs.Assign(backboneDevices)
    # 
    #  Los nodos de red ad-hoc necesitan un modelo de movilidad, por lo que agregamos uno para
    #  cada uno de los nodos que acabamos de terminar de construir. 
    # 
    mobility = ns.mobility.MobilityHelper()
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", ns.core.DoubleValue(20.0),
                                  "MinY", ns.core.DoubleValue(20.0),
                                  "DeltaX", ns.core.DoubleValue(20.0),
                                  "DeltaY", ns.core.DoubleValue(20.0),
                                  "GridWidth", ns.core.UintegerValue(5),
                                  "LayoutType", ns.core.StringValue("RowFirst"))
    mobility.SetMobilityModel("ns3::RandomDirection2dMobilityModel",
                               "Bounds", ns.mobility.RectangleValue(ns.mobility.Rectangle(-500, 500, -500, 500)),
                               "Speed", ns.core.StringValue ("ns3::ConstantRandomVariable[Constant=2]"),
                               "Pause", ns.core.StringValue ("ns3::ConstantRandomVariable[Constant=0.2]"))
    mobility.Install(backbone)

    # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #  
    #                                                                        # 
    #  Corremos la simulacion                                                # 
    #                                                                        # 
    # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #  
    print ("Corremos la simulacion.")
    ns.core.Simulator.Stop(ns.core.Seconds(stopTime))
    ns.core.Simulator.Run()
    ns.core.Simulator.Destroy()


if __name__ == '__main__':
    import sys
    main(sys.argv)
```
### 1.1 Descripción del codigo: 
``` fichero: ./simple_test.py```
```
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import gym
import argparse
import ns3gym

env = gym.make('ns3-v0')
env.reset()

ob_space = env.observation_space
ac_space = env.action_space
print("Observation space: ", ob_space,  ob_space.dtype)
print("Action space: ", ac_space, ac_space.dtype)

stepIdx = 0

try:
    obs = env.reset()
    print("Step: ", stepIdx)
    print("---obs: ", obs)

    while True:
        stepIdx += 1

        action = env.action_space.sample()
        print("---action: ", action)
        obs, reward, done, info = env.step(action)

        print("Step: ", stepIdx)
        print("---obs, reward, done, info: ", obs, reward, done, info)

        if done:
            break

except KeyboardInterrupt:
    print("Ctrl-C -> Exit")
finally:
    env.close()
    print("Done")
```

### 1.2 Descripción del codigo: 

### 2. Manual técnico:

### Requerimientos: 

NS3 y OpenAI Gym

### Herramientas utilizadas en el desarrollo: 

#### NS3:

#### OpenAI Gym:

## 3. Análisis de resultados.
